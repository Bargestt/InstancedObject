// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedEvent/InstancedEvent_Branch.h"

void UInstancedEvent_Branch::ExecuteEvent(const FInstancedEventContext& Context)
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

void UInstancedEvent_Branch::Cancel()
{
	if (Event.Object)
	{
		Event.Object->OnResultNative.RemoveAll(this);
		Event.Object->Cancel();
	}
	
	Super::Cancel();
}

void UInstancedEvent_Branch::GetSubEvents_Implementation(TArray<UInstancedEvent*>& OutEvents) const
{
	OutEvents.Add(Event.Object);
	OutEvents.Add(OnSuccess.Object);
	OutEvents.Add(OnFail.Object);
}

FString UInstancedEvent_Branch::GetInstancedObjectTitle_Implementation(bool bFullTitle) const
{
	if (!bFullTitle)
	{
		return FString::Printf(TEXT("<RichTextBlock.Bold>Branch</>: %s"), *GetTitleSafe(Event.Object, bFullTitle));
	}

	TArray<FString> Lines;
	{
		const FString Str = OnSuccess.IsValid() ? TEXT("{") + GetTitleSafe(OnSuccess.Get(), bFullTitle) + TEXT("}") : TEXT("None");		
		Lines.Add(FString::Printf(TEXT("<RichTextBlock.Bold>On Success</>: %s"), *Str));
	}
	{
		const FString Str = OnFail.IsValid() ? TEXT("{") + GetTitleSafe(OnFail.Get(), bFullTitle) + TEXT("}") : TEXT("None");		
		Lines.Add(FString::Printf(TEXT("<RichTextBlock.Bold>On Fail</>: %s"), *Str));
	}			
	return FString::Printf(TEXT("<RichTextBlock.Bold>Branch on Result</>{%s}"), *FString::Join(Lines, TEXT("\n")));
}

void UInstancedEvent_Branch::OnResult(const FInstancedEventResult& Result)
{
	if (Result.IsEndEvent())
	{
		Event.Object->OnResultNative.RemoveAll(this);
	
		if (Result.IsSuccessEvent())
		{
			OnSuccess.ExecuteEvent(CachedContext);
		}
		else 
		{
			OnFail.ExecuteEvent(CachedContext);
		}

		BroadcastResult(FInstancedEventTags::Get().Tag_EventSuccess);
	}
}
