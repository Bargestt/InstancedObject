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



