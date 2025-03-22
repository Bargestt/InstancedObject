// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.


#include "InstancedEvent/InstancedEvent.h"

#include "InstancedObjectModule.h"
#include "Misc/DataValidation.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InstancedEvent)

FInstancedEventTags FInstancedEventTags::Instance;

void FInstancedEventTags::Initialize(const FInstancedEventTags& Tags)
{
	UE_LOG(LogInstancedObject, Log, TEXT("FInstancedEventTags were initialized"));
	checkNoReentry();
	Instance = Tags;
}

UObject* FInstancedEventStruct::Get() const
{
	return Object;
}

void FInstancedEventStruct::ExecuteEvent(const FInstancedEventContext& Context) const
{
	if (Object)
	{
		Object->Execute(Context);
	}
}

TArray<UObject*> FInstancedEventArrayStruct::GetObjects() const
{
	return TArray<UObject*>(Objects);
}

int32 FInstancedEventArrayStruct::GetNumObjects() const
{
	return Objects.Num();
}

UObject* FInstancedEventArrayStruct::GetObjectAt(int32 Index) const
{
	return Objects[Index];
}

void FInstancedEventArrayStruct::RemoveObjectAt(int32 Index)
{
	Objects.RemoveAt(Index);
}

int32 FInstancedEventArrayStruct::InsertObjectAt(UObject* Object, int32 Index)
{
	return Objects.Insert(Cast<UInstancedEvent>(Object), Index);
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
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		BP_CancelEvent();
	}
}

void UInstancedEvent::ExecuteEvent(const FInstancedEventContext& Context)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
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



