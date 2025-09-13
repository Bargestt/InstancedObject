// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedEvent/InstancedEvent_Conditional.h"

void UInstancedEvent_Conditional::ExecuteEvent(const FInstancedEventContext& Context)
{
	FInstancedConditionContext ConditionContext;
	ConditionContext.WorldContextObject = Context.WorldContextObject;
	ConditionContext.Payload = Context.Payload;
	if (Condition.CheckCondition(ConditionContext, true))
	{
		OnTrue.ExecuteEvent(Context);
	}
	else
	{
		OnFalse.ExecuteEvent(Context);
	}
	BroadcastResult(FInstancedEventTags::Get().Tag_EventEnd);
}

FString UInstancedEvent_Conditional::GetInstancedObjectTitle_Implementation(bool bFullTitle) const
{
	if (!bFullTitle)
	{
		
		return FString::Printf(TEXT("<RichTextBlock.Bold>Condtion</>: %s -> %s / %s"), *GetTitleSafe(Condition.Get(), bFullTitle), *GetTitleSafe(OnTrue.Get(), bFullTitle), *GetTitleSafe(OnFalse.Get(), bFullTitle));
	}

	return FString::Printf(TEXT("<RichTextBlock.Bold>Condtion</>{%s}<RichTextBlock.Bold>OnTrue</>{%s}<RichTextBlock.Bold>OnFalse</>{%s}"),
		*GetTitleSafe(Condition.Get(), bFullTitle),
		*GetTitleSafe(OnTrue.Get(), bFullTitle),
		*GetTitleSafe(OnFalse.Get(), bFullTitle));
}

void UInstancedEvent_Conditional::GetSubEvents_Implementation(TArray<UInstancedEvent*>& OutEvents) const
{
	OutEvents.Add(OnTrue.Object);
	OutEvents.Add(OnFalse.Object);
}
