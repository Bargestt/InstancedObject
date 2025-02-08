// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedEvent/InstancedEvent_Conditional.h"

void UInstancedEvent_Conditional::ExecuteEvent(const FInstancedEventContext& Context)
{
	FInstancedConditionContext ConditionContext;
	ConditionContext.WorldContextObject = Context.WorldContextObject;
	ConditionContext.Payload = Context.Payload;
	if (Condition.CheckCondition(ConditionContext, true))
	{
		Event.ExecuteEvent(Context);
	}
	BroadcastResult({});
}

FString UInstancedEvent_Conditional::GetInstancedObjectTitle_Implementation(bool bFullTitle) const
{
	if (!bFullTitle)
	{
		return FString::Printf(TEXT("<RichTextBlock.Bold>Condtion</>: %s -> %s"), *GetTitleSafe(Condition.Get(), bFullTitle), *GetTitleSafe(Event.Get(), bFullTitle));
	}

	return FString::Printf(TEXT("<RichTextBlock.Bold>Condtion</>{%s}<RichTextBlock.Bold>Event</>{%s}"), *GetTitleSafe(Condition.Get(), bFullTitle), *GetTitleSafe(Event.Get(), bFullTitle));
}
