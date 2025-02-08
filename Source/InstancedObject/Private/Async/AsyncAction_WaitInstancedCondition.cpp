// Fill out your copyright notice in the Description page of Project Settings.


#include "Async/AsyncAction_WaitInstancedCondition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_WaitInstancedCondition)

UAsyncAction_WaitInstancedCondition* UAsyncAction_WaitInstancedCondition::WaitInstancedCondition(UObject* WorldContextObject, const FInstancedConditionStruct& Condition, const FInstancedConditionContext& Context, bool bFinishOnSuccess, bool bFinishOnFail, float CheckPeriod, bool bNotifyEveryCheck)
{
	if (Condition.Object)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			UAsyncAction_WaitInstancedCondition* Action = NewObject<UAsyncAction_WaitInstancedCondition>(WorldContextObject);
			if (Action)
			{
				Action->Condition = Condition.Object;
				Action->ConditionContext = Context;
				Action->bFinishOnSuccess = bFinishOnSuccess;
				Action->bFinishOnFail = bFinishOnFail;
				Action->CheckPeriod = CheckPeriod;
			
				return Action;
			}
		}
	}
	return nullptr;
}

void UAsyncAction_WaitInstancedCondition::Activate()
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->GetTimerManager().SetTimer(Timer, this, &ThisClass::Check, CheckPeriod, true);
		Check();
	}
	else
	{
		Cancel();
	}
}

void UAsyncAction_WaitInstancedCondition::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();
	if (UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->GetTimerManager().ClearTimer(Timer);
	}
	Timer.Invalidate();
}

UWorld* UAsyncAction_WaitInstancedCondition::GetWorld() const
{
	UObject* Outer = GetOuter();
	return Outer ? Outer->GetWorld() : nullptr;
}

void UAsyncAction_WaitInstancedCondition::Check()
{
	if (!Condition)
	{
		Cancel();
		return;
	}	
	
	bool bNewResult = Condition->Check(ConditionContext, GetWorld());
	if (bNotifyEveryCheck || (!bHasInitialized || bNewResult != bLastResult))
	{
		bHasInitialized = true;
		bLastResult = bNewResult;
		OnResultChanged.Broadcast(bLastResult);
	}

	if (bFinishOnSuccess && bLastResult)
	{
		Cancel();
	}

	if (bFinishOnFail && !bLastResult)
	{
		Cancel();
	}
}
