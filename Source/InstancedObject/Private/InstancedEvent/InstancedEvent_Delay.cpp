// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedEvent/InstancedEvent_Delay.h"
#include "InstancedObjectModule.h"

void UInstancedEvent_Delay::ExecuteEvent(const FInstancedEventContext& Context)
{
	Cancel();
	
	if (UWorld* World = GetWorld())
	{
		double Delay;
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
		
		if (Delay >= 0)
		{
			TimerWorld = World;
			CachedContext = Context;
			World->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::ExecuteContent, Delay, false);
		}
	}
	else
	{
		UE_LOG(LogInstancedObject, Log, TEXT("%s: No world from %s"), *GetName(), *GetNameSafe(Context.WorldContextObject));
	}
}

void UInstancedEvent_Delay::Cancel()
{
	if (UWorld* World = TimerWorld.Get())
	{		
		World->GetTimerManager().ClearTimer(TimerHandle);		
	}
	TimerWorld.Reset();
	Super::Cancel();
}

void UInstancedEvent_Delay::GetSubEvents_Implementation(TArray<UInstancedEvent*>& OutEvents) const
{
	OutEvents.Add(Event.Object);
}

FString UInstancedEvent_Delay::GetInstancedObjectTitle_Implementation(bool bFullTitle) const
{
	FString Range;
	{
		FNumberFormattingOptions Options;
		Options.MaximumFractionalDigits = 2;
		
		const double Min = FMath::Min(DelayRange.X, DelayRange.Y);
		const double Max = FMath::Max(DelayRange.X, DelayRange.Y);		
		if(Max < 0)
		{
			Range = TEXT("Inf");
		}
		else if (Min >= 0)
		{
			Range = FString::Printf(TEXT("[%s, %s]s"), *FText::AsNumber(Min, &Options).ToString(), *FText::AsNumber(Max, &Options).ToString());
		}
		else
		{
			Range = FString::Printf(TEXT("%ss"), *FText::AsNumber(Max, &Options).ToString());
		}
	}

	if (!bFullTitle)
	{
		return FString::Printf(TEXT("<RichTextBlock.Bold>Delay</>: %s -> %s"), *Range, *GetTitleSafe(Event.Get(), bFullTitle));
	}
	return FString::Printf(TEXT("<RichTextBlock.Bold>Delay</>: %s\n<RichTextBlock.Bold>Event</>{%s}"), *Range, *GetTitleSafe(Event.Get(), bFullTitle));
}

void UInstancedEvent_Delay::ExecuteContent()
{
	TimerWorld.Reset();
	TimerHandle.Invalidate();
	
	Event.ExecuteEvent(CachedContext);
	BroadcastResult(FInstancedEventTags::Get().Tag_EventEnd);
}
