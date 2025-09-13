// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedEvent/InstancedEvent_Switch.h"

void UInstancedEvent_Switch::ExecuteEvent(const FInstancedEventContext& Context)
{
	FInstancedConditionContext ConditionContext;
	ConditionContext.WorldContextObject = Context.WorldContextObject;
	ConditionContext.Payload = Context.Payload;
	
	for (auto& [Condition, Event] : Events)
	{
		if (Condition.CheckCondition(ConditionContext, true))
		{
			Event.ExecuteEvent(Context);
			if (bStopOnFirstSuccess)
			{
				break;
			}
		}
	}
	BroadcastResult(FInstancedEventTags::Get().Tag_EventEnd);
}

void UInstancedEvent_Switch::GetSubEvents_Implementation(TArray<UInstancedEvent*>& OutEvents) const
{
	for (auto& [Condition, Event] : Events)
	{
		OutEvents.Add(Event.Object);
	}
}

FString UInstancedEvent_Switch::GetInstancedObjectTitle_Implementation(bool bFullTitle) const
{
	if (!bFullTitle)
	{
		return FString::Printf(TEXT("<RichTextBlock.Bold>Switch %s</>: %d"), bStopOnFirstSuccess ? TEXT("Single") : TEXT("Multi"), Events.Num());
	}

	TArray<FString> Lines;
	for (int32 Index = 0; Index < Events.Num(); Index++)
	{
		const FInstancedEvent_SwitchPair& Pair = Events[Index];
		FString Condition = Pair.Condition.IsValid()	? TEXT("{") + GetTitleSafe(Pair.Condition.Get(), bFullTitle)	+ TEXT("}") : TEXT(": true");
		FString Event =		Pair.Event.IsValid()		? TEXT("{") + GetTitleSafe(Pair.Event.Get(), bFullTitle)		+ TEXT("}")	: TEXT(": None");		
		Lines.Add(FString::Printf(TEXT("<RichTextBlock.Bold>Case %d</>{Condtion%s\nEvent%s}"), Index, *Condition, *Event));
	}			
	return FString::Printf(TEXT("<RichTextBlock.Bold>Switch %s</>{%s}"), bStopOnFirstSuccess ? TEXT("Single") : TEXT("Multi"), *FString::Join(Lines, TEXT("\n")));
}
