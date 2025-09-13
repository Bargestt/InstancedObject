// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.


#include "InstancedEvent/InstancedEvent.h"

#include "InstancedObjectModule.h"
#include "Blueprint/BlueprintExceptionInfo.h"
#include "Misc/DataValidation.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InstancedEvent)

FInstancedEventTags FInstancedEventTags::Instance;

void FInstancedEventTags::Initialize(const FInstancedEventTags& Tags)
{
	UE_LOG(LogInstancedObject, Log, TEXT("FInstancedEventTags were initialized"));
	checkNoReentry();
	Instance = Tags;
}

UObject* FInstancedEventStruct::GetObject() const
{
	return Object;
}

void FInstancedEventStruct::SetObject(UObject* NewObject)
{
	Object = Cast<UInstancedEvent>(NewObject);
}

void FInstancedEventStruct::ExecuteEvent(const FInstancedEventContext& Context) const
{
	if (Object)
	{
		Object->Execute(Context);
	}
}



UInstancedEvent::UInstancedEvent()
{
	
}

UWorld* UInstancedEvent::GetWorld() const
{
	return CachedWorld.Get();
}

#if WITH_EDITOR
EDataValidationResult UInstancedEvent::IsDataValid(FDataValidationContext& Context) const
{	
	return UObject::IsDataValid(Context);
}
#endif //


void UInstancedEvent::Execute(const FInstancedEventContext& Context)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UInstancedEvent::Execute);

	CachedWorld = GEngine->GetWorldFromContextObject(Context.WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	ExecuteEvent(Context);
}

void UInstancedEvent::Cancel()
{
	if (!HasAnyFlags(RF_BeginDestroyed) && !IsUnreachable() && GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		BP_CancelEvent();
	}
}

void UInstancedEvent::ExecuteEvent(const FInstancedEventContext& Context)
{
	if (!HasAnyFlags(RF_BeginDestroyed) && !IsUnreachable() && GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		BP_ExecuteEvent(Context);
	}
}

void UInstancedEvent::BroadcastResult(const FGameplayTag& Type, const FInstancedStruct& Data)
{
	FInstancedEventResult Result
	{
		.Event = this,
		.Type = Type.IsValid() ? Type : FInstancedEventTags::Get().Tag_ReplaceInvalid,
		.Data = Data
	};
	OnResultNative.Broadcast(Result);
	OnResult.Broadcast(Result);
}

TArray<UInstancedEvent*> UInstancedEvent_Operator::GetSubEvents(bool bRecursive) const
{
	TArray<UInstancedEvent*> Result;
	GetSubEvents_Implementation(Result);
	
	if (bRecursive)
	{
		TArray<UInstancedEvent*> EventsToCheck = Result;
		TSet<UInstancedEvent*> CheckedEvents;
		while (!EventsToCheck.IsEmpty())
		{
			UInstancedEvent* Event = EventsToCheck[0];
			EventsToCheck.RemoveAt(0);
			
			if (Event)
			{
				bool bAlreadyChecked = false;
				CheckedEvents.Add(Event, &bAlreadyChecked);
				if (!bAlreadyChecked)
				{
					CheckedEvents.Add(Event);
				
					if (const UInstancedEvent_Operator* Operator = Cast<UInstancedEvent_Operator>(Event))
					{
						TArray<UInstancedEvent*> Sub; 
						Operator->GetSubEvents_Implementation(Sub);
						EventsToCheck.Append(Sub);
					}	
				}							
			}
		}
		
		Result = CheckedEvents.Array();
	}
	else
	{
		// Easier to remove all here instead of hoping users will ensure no nullptr
		Result.Remove(nullptr);
	}

	return Result;
}

/*--------------------------------------------
	UInstancedEventBlueprintLibrary
 *--------------------------------------------*/


void UInstancedEventBlueprintLibrary::ExecuteInstancedEvent(UObject* WorldContextObject, const FInstancedEventStruct& Event, const FInstancedEventContext& Context, const FInstancedEventResultDelegate& Result)
{
	if (Result.IsBound() && Event.Object)
	{
		Event.Object->OnResult.AddUnique(Result);
	}
	
	if (Context.WorldContextObject == nullptr)
	{
		FInstancedEventContext NewContext = Context;
		NewContext.WorldContextObject = WorldContextObject;
		Event.ExecuteEvent(NewContext);
	}
	else
	{
		Event.ExecuteEvent(Context);
	}
}

void UInstancedEventBlueprintLibrary::CancelInstancedEvent(const FInstancedEventStruct& Event)
{
	if (Event.Object)
	{
		Event.Object->Cancel();
	}
}


DEFINE_FUNCTION(UInstancedEventBlueprintLibrary::execGetResultData)
{
	P_GET_STRUCT_REF(FInstancedEventResult, Result);
	P_GET_STRUCT_REF(FGameplayTag, Type);
	P_GET_UBOOL(bExact)
	
	// Read wildcard Value input.
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	
	const FStructProperty* ValueProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	void* ValuePtr = Stack.MostRecentPropertyAddress;

	P_FINISH;

	bool bSuccess = false;

	if (!ValueProp || !ValuePtr)
	{
		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AbortExecution,
			NSLOCTEXT("InstancedObject", "InstancedEvent_GetInvalidValueWarning", "Failed to resolve the Value for GetResultData")
		);

		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
	}
	else
	{
		P_NATIVE_BEGIN;
		if (Result.Type == Type || (!bExact && Result.Type.MatchesTag(Type)))
		{
			if (Result.Data.IsValid() && Result.Data.GetScriptStruct()->IsChildOf(ValueProp->Struct))
			{
				ValueProp->Struct->CopyScriptStruct(ValuePtr, Result.Data.GetMemory());
				bSuccess = true;
			}
		}		
		P_NATIVE_END;
	}

	*static_cast<bool*>(RESULT_PARAM) = bSuccess;
}



