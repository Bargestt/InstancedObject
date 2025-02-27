// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedEvent/InstancedEvent_Wait.h"

void UInstancedEvent_Wait::ExecuteEvent(const FInstancedEventContext& Context)
{
	Cancel();
	
	if (Event.Object)
	{
		Event.Object->OnResultNative.AddUObject(this, &ThisClass::OnResult);
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
}
