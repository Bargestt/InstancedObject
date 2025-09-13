// Fill out your copyright notice in the Description page of Project Settings.


#include "Async/AsyncAction_ExecuteInstancedEventAndWait.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_ExecuteInstancedEventAndWait)

UAsyncAction_ExecuteInstancedEventAndWait* UAsyncAction_ExecuteInstancedEventAndWait::ExecuteInstancedEventAndWait(UObject* WorldContextObject, const FInstancedEventStruct& Event, const FInstancedEventContext& Context, bool bOnce, bool bDuplicateEvent)
{
	if (Event.Object)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			if (UAsyncAction_ExecuteInstancedEventAndWait* Action = NewObject<UAsyncAction_ExecuteInstancedEventAndWait>(WorldContextObject))
			{
				Action->InstancedEvent = Event.Object;
				Action->InstancedEventContext = Context;
				Action->bOnce = bOnce;
				Action->bDuplicate = bDuplicateEvent;
				return Action;
			}
		}
	}
	return nullptr;
}

void UAsyncAction_ExecuteInstancedEventAndWait::Activate()
{
	if (InstancedEvent)
	{
		if (bDuplicate)
		{
			InstancedEvent = DuplicateObject<UInstancedEvent>(InstancedEvent, this);
		}
		
		InstancedEvent->OnResultNative.AddUObject(this, &UAsyncAction_ExecuteInstancedEventAndWait::OnResult);
		if (InstancedEventContext.WorldContextObject == nullptr)
		{
			InstancedEventContext.WorldContextObject = GetOuter();
		}
		InstancedEvent->Execute(InstancedEventContext);
	}
	else
	{
		Cancel();
	}
}

void UAsyncAction_ExecuteInstancedEventAndWait::SetReadyToDestroy()
{	
	if (IsValid(InstancedEvent))
	{
		InstancedEvent->OnResultNative.RemoveAll(this);	
		InstancedEvent->Cancel();			
	}
	InstancedEvent = nullptr;
	
	Super::SetReadyToDestroy();
}

void UAsyncAction_ExecuteInstancedEventAndWait::Cancel()
{
	Super::Cancel();
}

UWorld* UAsyncAction_ExecuteInstancedEventAndWait::GetWorld() const
{
	UObject* Outer = GetOuter();
	return Outer ? Outer->GetWorld() : nullptr;
}

void UAsyncAction_ExecuteInstancedEventAndWait::OnResult(const FInstancedEventResult& Result)
{
	OnEvent.Broadcast(Result);
	if (bOnce && Result.IsEndEvent())
	{
		Cancel();
	}
}
