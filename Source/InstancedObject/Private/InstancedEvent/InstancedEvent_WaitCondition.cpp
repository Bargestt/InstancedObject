// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedEvent/InstancedEvent_WaitCondition.h"

#include "InstancedObjectModule.h"

void UInstancedEvent_WaitCondition::ExecuteEvent(const FInstancedEventContext& Context)
{
	if (UWorld* World = TimerWorld.Get())
	{		
		World->GetTimerManager().ClearTimer(TimerHandle);		
	}
	TimerWorld.Reset();
	
	if (UWorld* World = GetWorld())
	{
		CachedEventContext = Context;
		CachedConditionContext.WorldContextObject = Context.WorldContextObject;
		CachedConditionContext.Payload = Context.Payload;
		
		TimerWorld = World;
		Check();
		
		if (CheckPeriod >= 0)
		{
			if(TimerWorld.IsValid())
			{
				World->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::Check, CheckPeriod, true);
			}
		}
	}
	else
	{
		UE_LOG(LogInstancedObject, Log, TEXT("%s: No world from %s"), *GetName(), *GetNameSafe(Context.WorldContextObject));
	}
}

FString UInstancedEvent_WaitCondition::GetInstancedObjectTitle_Implementation(bool bFullTitle) const
{
	if (!bFullTitle)
	{
		return FString::Printf(TEXT("<RichTextBlock.Bold>Wait Condtion</>: %s -> %s"), *GetTitleSafe(Condition.Get(), bFullTitle), *GetTitleSafe(Event.Get(), bFullTitle));
	}

	return FString::Printf(TEXT("<RichTextBlock.Bold>Condtion</> Check period: %.2fs{%s}<RichTextBlock.Bold>Event</>{%s}"), CheckPeriod, *GetTitleSafe(Condition.Get(), bFullTitle), *GetTitleSafe(Event.Get(), bFullTitle));
}

void UInstancedEvent_WaitCondition::Check()
{	
	if (Condition.CheckCondition(CachedConditionContext, true))
	{
		if (UWorld* World = TimerWorld.Get())
		{		
			World->GetTimerManager().ClearTimer(TimerHandle);			
		}
		TimerWorld.Reset();

		Event.ExecuteEvent(CachedEventContext);
		BroadcastResult({});
	}
}
