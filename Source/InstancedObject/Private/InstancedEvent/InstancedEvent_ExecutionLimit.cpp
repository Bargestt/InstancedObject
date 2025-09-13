// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedEvent/InstancedEvent_ExecutionLimit.h"

void UInstancedEvent_ExecutionLimit::ExecuteEvent(const FInstancedEventContext& Context)
{
	double CurrentTime = 0;
	
	if (bExecutionTimeout)
	{
		if (Timeout < 0)
		{
			const double Min = FMath::Min(ExecutionTimeout.X, ExecutionTimeout.Y);
			const double Max = FMath::Max(ExecutionTimeout.X, ExecutionTimeout.Y);		
			if(Max < 0)
			{
				Timeout = TNumericLimits<double>::Max();
			}
			else if (Min >= 0) // Both enabled
			{
				Timeout = FMath::RandRange(Min, Max);
			}
			else
			{
				Timeout = Max;
			}	
		}

		CurrentTime = FPlatformTime::Seconds();
		UE_LOG(LogTemp, Log, TEXT("Test %.2f - %.2f < %.2f"), CurrentTime, LastExecution, Timeout);
		if (CurrentTime - LastExecution < Timeout)
		{
			return;
		}	
	}

	if (bLimitNumber)
	{
		if (ExecutionsLeft < 0)
		{
			const int32 Min = FMath::Min(LimitNumber.X, LimitNumber.Y);
			const int32 Max = FMath::Max(LimitNumber.X, LimitNumber.Y);		
			if(Max < 0)
			{
				ExecutionsLeft = TNumericLimits<int32>::Max();
			}
			else if (Min >= 0) // Both enabled
			{
				ExecutionsLeft = FMath::RandRange(Min, Max);
			}
			else
			{
				ExecutionsLeft = Max;
			}
		}		

		if (ExecutionsLeft == 0)
		{
			return;
		}
	}

	
	if (bLimitNumber)
	{
		ExecutionsLeft--;
	}
	if (bExecutionTimeout)
	{
		LastExecution = CurrentTime;
		Timeout = -1;
	}	
	Event.ExecuteEvent(Context);
}

void UInstancedEvent_ExecutionLimit::Cancel()
{
	ExecutionsLeft = -1;
	LastExecution = -1;
	Timeout = -1;
}

void UInstancedEvent_ExecutionLimit::GetSubEvents_Implementation(TArray<UInstancedEvent*>& OutEvents) const
{
	OutEvents.Add(Event.Object);
}

FString UInstancedEvent_ExecutionLimit::GetInstancedObjectTitle_Implementation(bool bFullTitle) const
{
	FString ExecutionTimeoutStr;
	if(bExecutionTimeout)
	{
		FNumberFormattingOptions Options;
		Options.MaximumFractionalDigits = 2;
		
		const double Min = FMath::Min(ExecutionTimeout.X, ExecutionTimeout.Y);
		const double Max = FMath::Max(ExecutionTimeout.X, ExecutionTimeout.Y);		
		if(Max < 0)
		{
			ExecutionTimeoutStr = TEXT("Inf");
		}
		else if (Min >= 0)
		{
			ExecutionTimeoutStr = FString::Printf(TEXT("[%s, %s]s"), *FText::AsNumber(Min, &Options).ToString(), *FText::AsNumber(Max, &Options).ToString());
		}
		else
		{
			ExecutionTimeoutStr = FString::Printf(TEXT("%ss"), *FText::AsNumber(Max, &Options).ToString());
		}
	}

	FString LimitNumberStr;
	if (bLimitNumber)
	{
		FNumberFormattingOptions Options;
		Options.MaximumFractionalDigits = 0;
		
		const double Min = FMath::Min(LimitNumber.X, LimitNumber.Y);
		const double Max = FMath::Max(LimitNumber.X, LimitNumber.Y);		
		if(Max < 0)
		{
			LimitNumberStr = TEXT("Inf");
		}
		else if (Min >= 0)
		{
			LimitNumberStr = FString::Printf(TEXT("[%s, %s]"), *FText::AsNumber(Min, &Options).ToString(), *FText::AsNumber(Max, &Options).ToString());
		}
		else
		{
			LimitNumberStr = FString::Printf(TEXT("%s"), *FText::AsNumber(Max, &Options).ToString());
		}		
	}

	if (!bFullTitle)
	{
		FString Param = ExecutionTimeoutStr;
		Param += (Param.IsEmpty() ? TEXT("") : TEXT(", ")) + LimitNumberStr;
		if (Param.IsEmpty())
		{
			Param = TEXT("Unlimited");
		}
		
		return FString::Printf(TEXT("<RichTextBlock.Bold>ExecutionLimit</>: %s -> %s"), *Param, *GetTitleSafe(Event.Get(), bFullTitle));
	}
	return FString::Printf(TEXT("<RichTextBlock.Bold>ExecutionLimit</>:\nTimeout: %s\nNum: %s\n<RichTextBlock.Bold>Event</>{%s}"), *ExecutionTimeoutStr, *LimitNumberStr, *GetTitleSafe(Event.Get(), bFullTitle));
}

#if WITH_EDITOR
void UInstancedEvent_ExecutionLimit::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	Cancel();
	bReset = false;
}
#endif 
