// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedEvent/InstancedEvent_External.h"

#include "InstancedEvent/InstancedEventAsset.h"

void UInstancedEvent_External::ExecuteEvent(const FInstancedEventContext& Context)
{
	Cancel();
	
	if (UInstancedEvent* Template = Event->GetEvent())
	{
		LastExecution = bDuplicateOnExecution ? DuplicateObject<UInstancedEvent>(Template, this) : Template;		
	}
	
	if (LastExecution)
	{
		LastExecution->OnResultNative.AddUObject(this, &UInstancedEvent_External::OnResult);
		LastExecution->Execute(Context);
	}
	else
	{
		BroadcastResult(FInstancedEventTags::Get().Tag_EventEnd);
	}	
}

void UInstancedEvent_External::Cancel()
{
	if (LastExecution)
	{
		LastExecution->OnResultNative.RemoveAll(this);
	}	
	LastExecution = nullptr;
	Super::Cancel();
}

FString UInstancedEvent_External::GetInstancedObjectTitle_Implementation(bool bFullTitle) const
{
	if (!bFullTitle)
	{
		return FString::Printf(TEXT("<RichTextBlock.Bold>External</>: %s"), *GetNameSafe(Event));
	}
	return FString::Printf(TEXT("<RichTextBlock.Bold>External</>:{%s}"), Event ? *GetTitleSafe(Event->GetEvent(), bFullTitle) : TEXT("None"));
}

void UInstancedEvent_External::OnResult(const FInstancedEventResult& Result)
{	
	BroadcastResult(Result.Type, Result.Data);
}
