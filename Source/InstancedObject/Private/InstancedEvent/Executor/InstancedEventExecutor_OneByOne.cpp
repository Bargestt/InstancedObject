// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.


#include "InstancedEvent/Executor/InstancedEventExecutor_OneByOne.h"
#include "InstancedEvent/InstancedEvent.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(InstancedEventExecutor_OneByOne)


void FInstancedEventExecutor_OneByOne::Reset()
{
	EventsToExecute.Empty();
	
	if (UInstancedEvent* CurrentEvent = WeakCurrentEvent.Get())
	{
		CurrentEvent->OnResultNative.RemoveAll(this);
		CurrentEvent->Cancel();
	}
	WeakCurrentEvent.Reset();
}

void FInstancedEventExecutor_OneByOne::Execute(const FInstancedEventContext& Context)
{	
	EventsToExecute.Reset();
	EventsToExecute.Append(Owner->GetSubEvents(false));
	
	CachedContext = Context;
	NextEvent();
}

void FInstancedEventExecutor_OneByOne::NextEvent()
{
	if(!EventsToExecute.IsEmpty())
	{
		WeakCurrentEvent = EventsToExecute[0];
		EventsToExecute.RemoveAt(0);

		if (UInstancedEvent* CurrentEvent = WeakCurrentEvent.Get())
		{
			CurrentEvent->OnResultNative.AddRaw(this, &FInstancedEventExecutor_OneByOne::OnResult);
			CurrentEvent->Execute(CachedContext);
		}
		else
		{
			NextEvent();
		}			
	}
	else
	{
		HandleResult(FInstancedEventResult::MakeSuccess(Owner));
	}
}

void FInstancedEventExecutor_OneByOne::OnResult(const FInstancedEventResult& Result)
{
	if (Result.IsEndEvent())
	{
		if (UInstancedEvent* CurrentEvent = WeakCurrentEvent.Get())
		{
			CurrentEvent->OnResultNative.RemoveAll(this);
		}
		WeakCurrentEvent.Reset();

		if (bStopOnFail && Result.IsFailEvent())
		{			
			HandleResult(FInstancedEventResult::MakeFail(Owner));
		}
		else
		{
			NextEvent();
		}
	}
}
