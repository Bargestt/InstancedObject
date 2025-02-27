// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedEvent/InstancedEvent_Repeat.h"

#include "InstancedObjectModule.h"

void UInstancedEvent_Repeat::ExecuteEvent(const FInstancedEventContext& Context)
{	
	Cancel();

	if (UWorld* World = GetWorld())
	{
		TimerWorld = World;
		{
			const int32 Min = FMath::Min(NumExecutions.X, NumExecutions.Y);
			const int32 Max = FMath::Max(NumExecutions.X, NumExecutions.Y);		
			if(Max < 0)
			{
				ExecutionsLeft = -1;
			}
			else if (Min >= 0) 
			{
				ExecutionsLeft = FMath::RandRange(Min, Max);
			}
			else 
			{
				ExecutionsLeft = Max;
			}
		}
		CachedContext = Context;
		CreateTimer();
	}
	else
	{
		UE_LOG(LogInstancedObject, Log, TEXT("%s: No world from %s"), *GetName(), *GetNameSafe(Context.WorldContextObject));
	}
}

void UInstancedEvent_Repeat::Cancel()
{
	if (UWorld* World = TimerWorld.Get())
	{		
		World->GetTimerManager().ClearTimer(TimerHandle);		
	}
	TimerWorld.Reset();
	Super::Cancel();
}

FString UInstancedEvent_Repeat::GetInstancedObjectTitle_Implementation(bool bFullTitle) const
{
	FString NumRange;
	{		
		const int32 Min = FMath::Min(NumExecutions.X, NumExecutions.Y);
		const int32 Max = FMath::Max(NumExecutions.X, NumExecutions.Y);		
		if(Max < 0)
		{
			NumRange = TEXT("Inf");
		}
		else if (Min >= 0)
		{
			NumRange = FString::Printf(TEXT("[%d, %d]"), Min, Max);
		}
		else
		{
			NumRange = FString::Printf(TEXT("%d"), Max);
		}
	}
	
	FString TimeRange;
	{
		FNumberFormattingOptions Options;
		Options.MaximumFractionalDigits = 2;
		
		const double Min = FMath::Min(DelayRange.X, DelayRange.Y);
		const double Max = FMath::Max(DelayRange.X, DelayRange.Y);		
		if(Max < 0)
		{
			TimeRange = TEXT("Inf");
		}
		else if (Min >= 0)
		{
			TimeRange = FString::Printf(TEXT("[%s, %s]s"), *FText::AsNumber(Min, &Options).ToString(), *FText::AsNumber(Max, &Options).ToString());
		}
		else
		{
			TimeRange = FString::Printf(TEXT("%ss"), *FText::AsNumber(Max, &Options).ToString());
		}
	}
	if (!bFullTitle)
	{
		return FString::Printf(TEXT("<RichTextBlock.Bold>Repeat</>: %s times %s -> %s"), *NumRange, *TimeRange, *GetTitleSafe(Event.Get(), bFullTitle));
	}
	return FString::Printf(TEXT("<RichTextBlock.Bold>Repeat</>:{Num: %s\nPeriod: %s %s}\n<RichTextBlock.Bold>Event</>{%s}"), *NumRange, *TimeRange, bRecalculateDelay ? TEXT("(recalc)") : TEXT(""), *GetTitleSafe(Event.Get(), bFullTitle));
}

void UInstancedEvent_Repeat::CreateTimer()
{
	if (UWorld* World = TimerWorld.Get())
	{
		double Delay;
		{
			const double Min = FMath::Min(DelayRange.X, DelayRange.Y);
			const double Max = FMath::Max(DelayRange.X, DelayRange.Y);		
			if(Max < 0)
			{
				Delay = -1;
			}
			else if (Min >= 0) // Both enabled
			{
				Delay = FMath::RandRange(Min, Max);
			}
			else
			{
				Delay = Max;
			}
		}
		
		if (Delay >= 0)
		{
			World->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::Execute, Delay, !bRecalculateDelay);
		}
	}
	else
	{
		UE_LOG(LogInstancedObject, Log, TEXT("%s: Lost world during execution"), *GetName());
	}
}

void UInstancedEvent_Repeat::Execute()
{
	bool bHasFinished = false;
	if (ExecutionsLeft > 0)
	{
		ExecutionsLeft--;
		bHasFinished = ExecutionsLeft == 0;
	}
	
	if (bRecalculateDelay && !bHasFinished)
	{
		CreateTimer();
	}
	
	Event.ExecuteEvent(CachedContext);
	if (bHasFinished)
	{
		if (UWorld* World = TimerWorld.Get())
		{		
			World->GetTimerManager().ClearTimer(TimerHandle);		
		}
		TimerWorld.Reset();
		BroadcastResult(FInstancedEventTags::Get().Tag_EventEnd);
	}
}
