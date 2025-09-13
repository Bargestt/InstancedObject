// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.


#include "InstancedEvent/Executor/InstancedEventExecutor.h"
#include "InstancedEvent/InstancedEvent.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(InstancedEventExecutor)


void FInstancedEventExecutor::Init(UInstancedEvent_Operator* InOwner)
{
	Owner = InOwner;
}

FString FInstancedEventExecutor::GetDisplayString(const UScriptStruct* ThisStruct) const
{
#if WITH_EDITOR
	return ThisStruct->GetDisplayNameText().ToString();			
#else
	return ThisStruct->GetName();
#endif //
}

void FInstancedEventExecutor::HandleResult(const FInstancedEventResult& Result)
{
	if (Result.IsEndEvent())
	{
		Reset();
	}
	
	OnResult.Execute(Result);
}
