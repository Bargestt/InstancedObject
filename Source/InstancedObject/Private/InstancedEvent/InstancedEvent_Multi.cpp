// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.


#include "InstancedEvent/InstancedEvent_Multi.h"
#include "InstancedEvent/Executor/InstancedEventExecutor.h"
#include "InstancedEvent/Executor/InstancedEventExecutor_Sequence.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(InstancedEvent_Multi)


UInstancedEvent_Multi::UInstancedEvent_Multi()
{	
	Execution.InitializeAs<FInstancedEventExecutor_Sequence>();
}

void UInstancedEvent_Multi::ExecuteEvent(const FInstancedEventContext& Context)
{
	if (!Execution.IsValid() || Execution.GetScriptStruct() == FInstancedEventExecutor::StaticStruct())
	{
		Execution.InitializeAs<FInstancedEventExecutor_Sequence>();
	}
	
	FInstancedEventExecutor& Struct = Execution.GetMutable();
	Struct.Reset();
	Struct.Init(this);
	if (!Struct.OnResult.IsBound())
	{
		Struct.OnResult.BindUObject(this, &UInstancedEvent_Multi::OnExecutorResult);
	}	
	Struct.Execute(Context);
}

void UInstancedEvent_Multi::Cancel()
{
	if (FInstancedEventExecutor* Struct = Execution.GetMutablePtr())
	{
		Struct->Reset();
	}	
	Super::Cancel();
}

void UInstancedEvent_Multi::GetSubEvents_Implementation(TArray<UInstancedEvent*>& OutEvents) const
{
	for (const FInstancedEventStruct& Event : Events)
	{
		OutEvents.Add(Event.Object);
	}
}

FString UInstancedEvent_Multi::GetInstancedObjectTitle_Implementation(bool bFullTitle) const
{	
	const FString DefaultValue = TEXT("None");

	const UScriptStruct* ExecutorStruct = Execution.IsValid() ? Execution.GetScriptStruct() : FInstancedEventExecutor_Sequence::StaticStruct();	
	FString ExecutionStr = Execution.Get().GetDisplayString(ExecutorStruct);	
	if (ExecutionStr.IsEmpty())
	{
#if WITH_EDITOR
		ExecutionStr = ExecutorStruct->GetDisplayNameText().ToString();			
#else
		ExecutionStr = ExecutorStruct->GetName();
#endif // WITH_EDITOR
	}	
	
	if (!bFullTitle)
	{
		return FString::Printf(TEXT("<RichTextBlock.Bold>Events</>: %d, %s "), Events.Num(), *ExecutionStr);
	}
			
	TArray<FString> Lines;
	for (int32 Index = 0; Index < Events.Num(); Index++)
	{
		FString Line = Events[Index].IsValid() ? *GetTitleSafe(Events[Index].Get(), bFullTitle) : *DefaultValue;
		Lines.Add(Line);
	}			
	return FString::Printf(TEXT("<RichTextBlock.Bold>Events</> %s {%s}"), *ExecutionStr, *FString::Join(Lines, TEXT("\n")));
}

void UInstancedEvent_Multi::BeginDestroy()
{
	Super::BeginDestroy();
	if (Execution.IsValid())
	{
		Execution.GetMutable().Reset();
	}
}

void UInstancedEvent_Multi::OnExecutorResult(const FInstancedEventResult& Result)
{
	if (Result.IsEndEvent())
	{
		BroadcastResult(Result.Type);
	}
}