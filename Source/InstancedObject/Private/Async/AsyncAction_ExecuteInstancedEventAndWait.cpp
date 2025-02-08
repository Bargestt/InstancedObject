// Fill out your copyright notice in the Description page of Project Settings.


#include "Async/AsyncAction_ExecuteInstancedEventAndWait.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_ExecuteInstancedEventAndWait)

UAsyncAction_ExecuteInstancedEventAndWait* UAsyncAction_ExecuteInstancedEventAndWait::ExecuteInstancedEventAndWait(UObject* WorldContextObject, const FInstancedEventStruct& Event, const FInstancedEventContext& Context, bool bOnce)
{
	if (Event.Object)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			if (UAsyncAction_ExecuteInstancedEventAndWait* Action = NewObject<UAsyncAction_ExecuteInstancedEventAndWait>(WorldContextObject))
			{
				Action->InstancedEvent = Event.Object;
				Action->InstancedEventContext = Context;			
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
		InstancedEvent->OnResultNative.AddUObject(this, &UAsyncAction_ExecuteInstancedEventAndWait::OnResult);
		if (InstancedEventContext.WorldContextObject == nullptr)
		{
			InstancedEventContext.WorldContextObject = this;
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
	Super::SetReadyToDestroy();
	if (InstancedEvent)
	{
		InstancedEvent->OnResultNative.RemoveAll(this);
		InstancedEvent = nullptr;
	}
}

UWorld* UAsyncAction_ExecuteInstancedEventAndWait::GetWorld() const
{
	UObject* Outer = GetOuter();
	return Outer ? Outer->GetWorld() : nullptr;
}

void UAsyncAction_ExecuteInstancedEventAndWait::OnResult(const FInstancedEventResult& Result)
{
	OnEnd.Broadcast(Result);
	if (bOnce)
	{
		Cancel();
	}
}
