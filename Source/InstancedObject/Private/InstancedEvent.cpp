// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.


#include "InstancedEvent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(InstancedEvent)

UObject* FInstancedEventStruct::Get() const
{
	return Object;
}

void FInstancedEventStruct::ExecuteEvent(const FInstancedEventContext& Context) const
{
	if (Object)
	{
		Object->Execute(Context);
	}
}



UWorld* UInstancedEvent::GetWorld() const
{
	return World ? World : nullptr;
}

void UInstancedEvent::Execute(const FInstancedEventContext& Context)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UInstancedEvent::Execute);
	
	World = GEngine->GetWorldFromContextObject(Context.WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	ExecuteEvent(Context);
	World = nullptr;
}

/*--------------------------------------------
	UInstancedEventBlueprintLibrary
 *--------------------------------------------*/


void UInstancedEventBlueprintLibrary::ExecuteInstancedEvent(UObject* WorldContextObject, const FInstancedEventStruct& Event, const FInstancedEventContext& Context)
{
	if (Context.WorldContextObject == nullptr)
	{
		FInstancedEventContext NewContext = Context;
		NewContext.WorldContextObject = WorldContextObject;
		Event.ExecuteEvent(NewContext);
	}
	else
	{
		Event.ExecuteEvent(Context);
	}
}



/*--------------------------------------------
	 Default boolean operators
 *--------------------------------------------*/


void UInstancedEvent_Multi::ExecuteEvent_Implementation(const FInstancedEventContext& Context)
{
	for (const FInstancedEventStruct Event : Events)
	{
		Event.ExecuteEvent(Context);
	}
}

FString UInstancedEvent_Multi::GetInstancedObjectTitle_Implementation(bool bFullTitle) const
{	
	const FString DefaultValue = TEXT("None");
	
	if (!bFullTitle)
	{
		return FString::Printf(TEXT("<RichTextBlock.Bold>Events</>: %d "), Events.Num());
	}
			
	TArray<FString> Lines;
	for (int32 Index = 0; Index < Events.Num(); Index++)
	{
		FString Line = Events[Index].IsValid() ? *GetTitleSafe(Events[Index].Get(), bFullTitle) : *DefaultValue;
		Lines.Add(Line);
	}			
	return FString::Printf(TEXT("<RichTextBlock.Bold>Events</>{%s}"), *FString::Join(Lines, TEXT("\n")));
}





