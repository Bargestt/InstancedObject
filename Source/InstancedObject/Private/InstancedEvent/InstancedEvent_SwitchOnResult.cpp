// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedEvent/InstancedEvent_SwitchOnResult.h"

void UInstancedEvent_SwitchOnResult::ExecuteEvent(const FInstancedEventContext& Context)
{
	if (Event.Object)
	{
		if (!Event.Object->OnResultNative.IsBoundToObject(this))
		{
			Event.Object->OnResultNative.AddUObject(this, &ThisClass::OnResult);
		}
		CachedContext = Context;
		Event.Object->Execute(Context);
	}
	else
	{
		BroadcastResult(FInstancedEventTags::Get().Tag_EventEnd);
	}
}

void UInstancedEvent_SwitchOnResult::Cancel()
{
	if (Event.Object)
	{
		Event.Object->OnResultNative.RemoveAll(this);
		Event.Object->Cancel();
	}
	
	Super::Cancel();
}

void UInstancedEvent_SwitchOnResult::GetSubEvents_Implementation(TArray<UInstancedEvent*>& OutEvents) const
{
	OutEvents.Add(Event.Object);
	for (auto& [Tag, Data] : Entries)
	{
		OutEvents.Add(Data.Event.Object);
	}
}

FString UInstancedEvent_SwitchOnResult::GetInstancedObjectTitle_Implementation(bool bFullTitle) const
{
	if (!bFullTitle)
	{
		return FString::Printf(TEXT("<RichTextBlock.Bold>Switch on Result</>: %s"), *GetTitleSafe(Event.Object, bFullTitle));
	}

	TArray<FString> Lines;
	for (auto& [Tag, Data] : Entries)
	{		
		FString Str;
		if (Data.bExactTag)
		{
			Str = TEXT("ExactTag\n") + Str;		
		}
		if (Data.bEndExecution)
		{
			Str = TEXT("End\n") + Str;			
		}
		Str += GetTitleSafe(Data.Event.Get(), bFullTitle);		
		
		Lines.Add(FString::Printf(TEXT("<RichTextBlock.Bold>%s</>:{%s}"), *Tag.ToString(), *Str));
	}			
	return FString::Printf(TEXT("<RichTextBlock.Bold>Switch on Result</>\nEvent:{%s}\nCases:{%s}"), *GetTitleSafe(Event.Object, bFullTitle), *FString::Join(Lines, TEXT("\n")));
}

void UInstancedEvent_SwitchOnResult::OnResult(const FInstancedEventResult& Result)
{
	bool bEnded = false;

	for (auto& [Tag, Data] : Entries)
	{
		if (Result.Type == Tag || (!Data.bExactTag && Result.Type.MatchesTag(Tag)))
		{
			if (!bEnded && Data.bEndExecution)
			{
				bEnded = true;
				Event.Object->OnResultNative.RemoveAll(this);
				Event.Object->Cancel();
			}
			
			Data.Event.ExecuteEvent(CachedContext);
		}
	}
	
	if (bEnded)
	{
		BroadcastResult(FInstancedEventTags::Get().Tag_EventSuccess);
	}
}
