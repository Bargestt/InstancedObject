// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.


#include "InstancedEvent/Executor/InstancedEventExecutor_Sequence.h"
#include "InstancedEvent/InstancedEvent.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(InstancedEventExecutor_Sequence)


void FInstancedEventExecutor_SequenceBase::Reset()
{
	for (auto& WeakEvent : WaitEvents)
	{
		if (UInstancedEvent* Event = WeakEvent.Get())
		{			
			Event->OnResultNative.RemoveAll(this);
			Event->Cancel();
		}			
	}
	WaitEvents.Empty();
}

void FInstancedEventExecutor_SequenceBase::Execute(const FInstancedEventContext& Context)
{
	TArray<UInstancedEvent*> Events;
	PreExecute(Context, Events);
	
	for (auto& WeakEvent : WaitEvents)
	{
		if (UInstancedEvent* Event = WeakEvent.Get())
		{
			Event->OnResultNative.AddRaw(this, &FInstancedEventExecutor_SequenceBase::OnResult);
		}
	}

	const bool bEndExecution = Events.IsEmpty() && WaitEvents.IsEmpty();
	
	if (!Events.IsEmpty())
	{
		for (UInstancedEvent* Event : Events)
		{
			Event->Execute(Context);
		}
	}
	
	if(bEndExecution)
	{
		HandleResult(FInstancedEventResult::MakeSuccess(Owner));
	}
}

void FInstancedEventExecutor_SequenceBase::OnResult(const FInstancedEventResult& Result)
{
	Result.Event->OnResultNative.RemoveAll(this);
	WaitEvents.RemoveAll([&Result](const TWeakObjectPtr<UInstancedEvent>& Entry) { return !Entry.IsValid() || Entry == Result.Event; });
	
	if (WaitEvents.IsEmpty())
	{
		HandleResult(FInstancedEventResult::MakeSuccess(Owner));
	}
}



void FInstancedEventExecutor_Sequence::PreExecute(const FInstancedEventContext& Context, TArray<UInstancedEvent*>& EventsToExecute)
{	
	for (UInstancedEvent* Event : Owner->GetSubEvents(false))
	{
		EventsToExecute.Add(Event);		
	}	
}


void FInstancedEventExecutor_SequenceWait::PreExecute(const FInstancedEventContext& Context, TArray<UInstancedEvent*>& EventsToExecute)
{
	Super::PreExecute(Context, EventsToExecute);
	WaitEvents.Reset();
	WaitEvents.Append(EventsToExecute);
}

void FInstancedEventExecutor_SequenceWait::OnResult(const FInstancedEventResult& Result)
{
	if (bStopOnFail && Result.IsFailEvent())
	{
		for (auto& WeakEvent : WaitEvents)
		{
			if (UInstancedEvent* Event = WeakEvent.Get())
			{
				Event->Cancel();
			}
		}
		HandleResult(FInstancedEventResult::MakeFail(Owner));
	}
	else
	{
		Super::OnResult(Result);
	}	
}



void FInstancedEventExecutor_Random::PreExecute(const FInstancedEventContext& Context, TArray<UInstancedEvent*>& EventsToExecute)
{
	TArray<UInstancedEvent*> Events = Owner->GetSubEvents(false);

	int32 Num;
	{
		const int32 Min = FMath::Min(RandomRange.X, RandomRange.Y);
		const int32 Max = FMath::Max(RandomRange.X, RandomRange.Y);		
		if(Max < 0) // Both disabled
		{
			Num = Events.Num();
		}
		else if (Min >= 0) // Both enabled
		{
			Num = FMath::RandRange(Min, Max);
		}
		else // Min is disabled
		{
			Num = Max;
		}
	}

	while (!Events.IsEmpty() && EventsToExecute.Num() < Num)
	{
		const int32 Index = FMath::RandRange(0, Events.Num() - 1);		
		EventsToExecute.Add(Events[Index]);
		Events.RemoveAtSwap(Index);
	}
}

FString FInstancedEventExecutor_Random::GetDisplayString(const UScriptStruct* ThisStruct) const
{
	FString Range;
	const int32 Min = FMath::Min(RandomRange.X, RandomRange.Y);
	const int32 Max = FMath::Max(RandomRange.X, RandomRange.Y);		
	if(Max < 0)
	{
		Range = TEXT("All");		
	}
	else if (Min >= 0)
	{
		Range = FString::Printf(TEXT("[%d, %d]"), Min, Max);
	}
	else
	{
		Range = FString::Printf(TEXT("%d"), Max);
	}
	
	return FString::Printf(TEXT("%s %s"), *FInstancedEventExecutor::GetDisplayString(ThisStruct), *Range);
}


void FInstancedEventExecutor_RandomWait::PreExecute(const FInstancedEventContext& Context, TArray<UInstancedEvent*>& EventsToExecute)
{
	Super::PreExecute(Context, EventsToExecute);
	WaitEvents.Reset();
	WaitEvents.Append(EventsToExecute);
}

void FInstancedEventExecutor_RandomWait::OnResult(const FInstancedEventResult& Result)
{
	if (bStopOnFail && Result.IsFailEvent())
	{
		HandleResult(FInstancedEventResult::MakeFail(Owner));
	}
	else
	{
		Super::OnResult(Result);
	}	
}
