// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InstancedEventExecutor.h"
#include "InstancedEventExecutor_OneByOne.generated.h"



/** Execute events in sequence, but wait for event to finish before starting next */
USTRUCT(NotBlueprintType, meta=(DisplayName="One by One"))
struct INSTANCEDOBJECT_API FInstancedEventExecutor_OneByOne : public FInstancedEventExecutor
{
	GENERATED_BODY()

	/** Stop execution and broadcast Fail if executed event fails */
	UPROPERTY(EditAnywhere, Category=Execution)
	bool bStopOnFail = false;
	
public:
	virtual void Reset() override;
	virtual void Execute(const FInstancedEventContext& Context) override;
	void NextEvent();
	void OnResult(const FInstancedEventResult& Result);

protected:
	FInstancedEventContext CachedContext;
	TWeakObjectPtr<UInstancedEvent> WeakCurrentEvent;
	TArray<TWeakObjectPtr<UInstancedEvent>> EventsToExecute;	
};