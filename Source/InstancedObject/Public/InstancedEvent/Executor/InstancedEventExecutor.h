// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InstancedEvent/InstancedEvent.h"
#include "InstancedEventExecutor.generated.h"



USTRUCT(NotBlueprintType)
struct INSTANCEDOBJECT_API FInstancedEventExecutor
{
	GENERATED_BODY()

public:
	DECLARE_DELEGATE_OneParam(FOnResultEvent, const FInstancedEventResult&);
	FOnResultEvent OnResult;
	
public:
	FInstancedEventExecutor() { }
	virtual ~FInstancedEventExecutor() { }

	virtual void Init(UInstancedEvent_Operator* InOwner);
	virtual void Reset() { }
	virtual void Execute(const FInstancedEventContext& Context) { }
	virtual FString GetDisplayString(const UScriptStruct* ThisStruct) const;
protected:
	virtual void HandleResult(const FInstancedEventResult& Result);

protected:
	// Should be ref, but initialization will be inconvenient
	UInstancedEvent_Operator* Owner = nullptr;
};
