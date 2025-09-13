// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedEvent/InstancedEvent_Wait.h"

void UInstancedEvent_Wait::ExecuteEvent(const FInstancedEventContext& Context)
{
	if (Event.Object)
	{
		if (!Event.Object->OnResultNative.IsBoundToObject(this))
		{
			Event.Object->OnResultNative.AddUObject(this, &ThisClass::OnResult);
		}		
		Event.Object->Execute(Context);
	}
	else
	{
		BroadcastResult(FInstancedEventTags::Get().Tag_EventEnd);
	}	
}

void UInstancedEvent_Wait::Cancel()
{
	if (Event.Object)
	{
		Event.Object->OnResultNative.RemoveAll(this);
		Event.Object->Cancel();
	}
	ReceivedEvents.Empty();
	
	Super::Cancel();
}

void UInstancedEvent_Wait::GetSubEvents_Implementation(TArray<UInstancedEvent*>& OutEvents) const
{
	OutEvents.Add(Event.Object);
}

FString UInstancedEvent_Wait::GetInstancedObjectTitle_Implementation(bool bFullTitle) const
{
	if (!bFullTitle)
	{
		return FString::Printf(TEXT("<RichTextBlock.Bold>Wait</>: %s"), *GetNameSafe(Event.Object));
	}
	FString RemapInfo = FString::JoinBy(EventRemap, TEXT(",\n"), [](const TPair<FGameplayTag,FGameplayTag>& Pair){ return Pair.Key.ToString() + TEXT(" -> " + Pair.Value.ToString()); });
	if (!RemapInfo.IsEmpty())
	{
		RemapInfo = TEXT("\n Remap: {") + RemapInfo + TEXT("}");
	}
	
	return FString::Printf(TEXT("<RichTextBlock.Bold>Wait</>:%s Event{%s}"), *RemapInfo, *GetTitleSafe(Event.Object, bFullTitle));
}

void UInstancedEvent_Wait::OnResult(const FInstancedEventResult& Result)
{
	LastResult_Event = Result.Event;
	LastResult_Type = Result.Type;
	LastResult_Data = Result.Data;
	
	ReceivedEvents.Remove(Result.Type);
	ReceivedEvents.Add(Result.Type);
	
	if (EventRemap.IsEmpty())
	{
		BroadcastResult(FInstancedEventTags::Get().Tag_EventEnd, Result.Data);
	}
	else
	{
		if (const FGameplayTag* RemapTag = EventRemap.Find(Result.Type))
		{
			BroadcastResult(*RemapTag, Result.Data);
		}
	}

	if (bEnable_CancelEvent && (CancelEvent.IsEmpty() || Result.Type.MatchesAny(CancelEvent)))
	{
		Cancel();
	}
}

bool UInstancedEvent_Wait::HasResult() const
{
	return ReceivedEvents.Num() > 0;
}

bool UInstancedEvent_Wait::HasMultipleResults() const
{
	return ReceivedEvents.Num() > 1;
}

bool UInstancedEvent_Wait::HasSucceeded() const
{
	return ReceivedEvents.Contains(FInstancedEventTags::Get().Tag_EventSuccess);
}

bool UInstancedEvent_Wait::HasFailed() const
{
	return ReceivedEvents.Contains(FInstancedEventTags::Get().Tag_EventFail);
}


FInstancedEventResult UInstancedEvent_Wait::GetLastResult() const
{
	return FInstancedEventResult{
		.Event = LastResult_Event.Get(),
		.Type = LastResult_Type,
		.Data = LastResult_Data
	};
}

bool UInstancedEvent_Wait::HasLastResultSucceeded() const
{
	return HasResult() && LastResult_Type == FInstancedEventTags::Get().Tag_EventSuccess;
}

bool UInstancedEvent_Wait::HasLastResultFailed() const
{
	return HasResult() && LastResult_Type == FInstancedEventTags::Get().Tag_EventFail;
}
