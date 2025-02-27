// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.


#include "InstancedEvent/InstancedEvent_Multi.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(InstancedEvent_Multi)


UInstancedEvent_Multi::UInstancedEvent_Multi()
{	
	Execution.InitializeAs<FInstancedEventExecutor_Sequence>();
}

void UInstancedEvent_Multi::ExecuteEvent(const FInstancedEventContext& Context)
{
	if (!Execution.IsValid() || Execution.GetScriptStruct() == FInstancedEventExecutor::StaticStruct())
	{
		Execution.InitializeAs<FInstancedEventExecutor_Sequence>();
	}
	
	FInstancedEventExecutor& Struct = Execution.GetMutable();
	Struct.Reset();
	Struct.Init(this);
	Struct.Execute(Context);
}

void UInstancedEvent_Multi::Cancel()
{
	if (FInstancedEventExecutor* Struct = Execution.GetMutablePtr())
	{
		Struct->Reset();
	}	
	Super::Cancel();
}

FString UInstancedEvent_Multi::GetInstancedObjectTitle_Implementation(bool bFullTitle) const
{	
	const FString DefaultValue = TEXT("None");

	FString ExecutionStr;
	if (Execution.IsValid())
	{
		ExecutionStr = Execution.Get().GetDisplayString(Execution.GetScriptStruct());
		if (ExecutionStr.IsEmpty())
		{
#if WITH_EDITOR
			ExecutionStr = Execution.GetScriptStruct()->GetDisplayNameText().ToString();			
#else
			ExecutionStr = Execution.GetScriptStruct()->GetName();
#endif //
		}
	}
	else
	{
#if WITH_EDITOR
		ExecutionStr = FInstancedEventExecutor_Sequence::StaticStruct()->GetDisplayNameText().ToString();			
#else
		ExecutionStr = FInstancedEventExecutor_Sequence::StaticStruct()->GetName();
#endif //
	}	
	
	if (!bFullTitle)
	{
		return FString::Printf(TEXT("<RichTextBlock.Bold>Events</>: %d, %s "), Events.Num(), *ExecutionStr);
	}
			
	TArray<FString> Lines;
	for (int32 Index = 0; Index < Events.Num(); Index++)
	{
		FString Line = Events[Index].IsValid() ? *GetTitleSafe(Events[Index].Get(), bFullTitle) : *DefaultValue;
		Lines.Add(Line);
	}			
	return FString::Printf(TEXT("<RichTextBlock.Bold>Events</> %s {%s}"), *ExecutionStr, *FString::Join(Lines, TEXT("\n")));
}

void UInstancedEvent_Multi::BeginDestroy()
{
	Super::BeginDestroy();
	if (Execution.IsValid())
	{
		Execution.GetMutable().Reset();
	}
}

void UInstancedEvent_Multi::End()
{
	if (Execution.IsValid())
	{
		Execution.GetMutable().Reset();
	}
	BroadcastResult(FInstancedEventTags::Get().Tag_EventEnd);
}



void FInstancedEventExecutor_SequenceBase::Reset()
{
	for (UInstancedEvent* Event : WaitEvents)
	{
		if (Event)
		{
			Event->OnResultNative.RemoveAll(this);
		}			
	}
	WaitEvents.Empty();
}

void FInstancedEventExecutor_SequenceBase::Execute(const FInstancedEventContext& Context)
{
	TArray<UInstancedEvent*> Events;
	PreExecute(Context, Events);
	
	for (UInstancedEvent* Event : WaitEvents)
	{
		Event->OnResultNative.AddRaw(this, &FInstancedEventExecutor_SequenceBase::OnResult);	
	}

	const bool bEndExecution = Events.IsEmpty() || WaitEvents.IsEmpty();
	
	if (!Events.IsEmpty())
	{
		for (UInstancedEvent* Event : Events)
		{
			Event->Execute(Context);
		}
	}
	
	if(bEndExecution)
	{
		End();
	}
}

void FInstancedEventExecutor_SequenceBase::OnResult(const FInstancedEventResult& Result)
{
	WaitEvents.Remove(Result.Event);
	if (WaitEvents.IsEmpty())
	{
		End();	
	}
}



void FInstancedEventExecutor_Sequence::PreExecute(const FInstancedEventContext& Context, TArray<UInstancedEvent*>& EventsToExecute)
{
	for (const FInstancedEventStruct& EventStruct : Owner->Events)
	{
		if (UInstancedEvent* Event = EventStruct.Object)
		{
			EventsToExecute.Add(Event);
		}
	}	
}
void FInstancedEventExecutor_SequenceWait::PreExecute(const FInstancedEventContext& Context, TArray<UInstancedEvent*>& EventsToExecute)
{
	FInstancedEventExecutor_Sequence::PreExecute(Context, EventsToExecute);
	WaitEvents = EventsToExecute;
}


void FInstancedEventExecutor_Random::PreExecute(const FInstancedEventContext& Context, TArray<UInstancedEvent*>& EventsToExecute)
{
	int32 Num;
	{
		const int32 Min = FMath::Min(RandomRange.X, RandomRange.Y);
		const int32 Max = FMath::Max(RandomRange.X, RandomRange.Y);		
		if(Max < 0) // Both disabled
		{
			Num = Owner->Events.Num();
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

	TArray<FInstancedEventStruct> Temp = Owner->Events;
	while (!Temp.IsEmpty() && EventsToExecute.Num() < Num)
	{
		const int32 Index = FMath::RandRange(0, Temp.Num() - 1);		
		if (UInstancedEvent* Event = Temp[Index].Object)
		{
			EventsToExecute.Add(Event);
		}
		Temp.RemoveAtSwap(Index);
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
	FInstancedEventExecutor_Random::PreExecute(Context, EventsToExecute);
	WaitEvents = EventsToExecute;
}




void FInstancedEventExecutor_OneByOne::Reset()
{
	EventsToExecute.Empty();
}

void FInstancedEventExecutor_OneByOne::Execute(const FInstancedEventContext& Context)
{
	for (const FInstancedEventStruct& EventStruct : Owner->Events)
	{
		if (UInstancedEvent* Event = EventStruct.Object)
		{
			EventsToExecute.Add(Event);
		}
	}
	CachedContext = Context;
	NextEvent();
}

void FInstancedEventExecutor_OneByOne::NextEvent()
{
	if(!EventsToExecute.IsEmpty())
	{
		CurrentEvent = EventsToExecute[0];
		EventsToExecute.RemoveAt(0);

		if (CurrentEvent)
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
		End();
	}
}
