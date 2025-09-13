// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InstancedEventExecutor.h"
#include "InstancedEventExecutor_Sequence.generated.h"


USTRUCT(NotBlueprintType, meta=(Hidden))
struct INSTANCEDOBJECT_API FInstancedEventExecutor_SequenceBase : public FInstancedEventExecutor
{
	GENERATED_BODY()
public:
	virtual void Reset() override;
	virtual void Execute(const FInstancedEventContext& Context) override;
	virtual void OnResult(const FInstancedEventResult& Result);	
protected:
	virtual void PreExecute(const FInstancedEventContext& Context, TArray<UInstancedEvent*>& EventsToExecute) { }
protected:
	TArray<TWeakObjectPtr<UInstancedEvent>> WaitEvents;
};


/** Execute events in sequence and instantly finish */
USTRUCT(NotBlueprintType, meta=(DisplayName="Sequence"))
struct INSTANCEDOBJECT_API FInstancedEventExecutor_Sequence : public FInstancedEventExecutor_SequenceBase
{
	GENERATED_BODY()
	
protected:
	virtual void PreExecute(const FInstancedEventContext& Context, TArray<UInstancedEvent*>& EventsToExecute) override;
};

/** Execute events in sequence and wait for all of them to finish */
USTRUCT(NotBlueprintType, meta=(DisplayName="Sequence Wait"))
struct INSTANCEDOBJECT_API FInstancedEventExecutor_SequenceWait : public FInstancedEventExecutor_Sequence
{
	GENERATED_BODY()

	/**
	 * Stop execution and broadcast Fail if executed event fails.
	 * This will cancel pending events
	 */
	UPROPERTY(EditAnywhere, Category=Execution)
	bool bStopOnFail = false;
	
protected:
	virtual void PreExecute(const FInstancedEventContext& Context, TArray<UInstancedEvent*>& EventsToExecute) override;
	virtual void OnResult(const FInstancedEventResult& Result) override;
};




/** Execute events in random order and instantly finish */
USTRUCT(NotBlueprintType, meta=(DisplayName="Random"))
struct INSTANCEDOBJECT_API FInstancedEventExecutor_Random : public FInstancedEventExecutor_SequenceBase
{
	GENERATED_BODY()

	/** -1 disables randomization */
	UPROPERTY(EditAnywhere, Category=Execution, meta=(DisplayPriority=-10))
	FIntPoint RandomRange = FIntPoint(-1, -1);
	
protected:
	virtual void PreExecute(const FInstancedEventContext& Context, TArray<UInstancedEvent*>& EventsToExecute) override;
	virtual FString GetDisplayString(const UScriptStruct* ThisStruct) const override;
};

/** Execute events in random order and wait for all of them to finish */
USTRUCT(NotBlueprintType, meta=(DisplayName="Random Wait"))
struct INSTANCEDOBJECT_API FInstancedEventExecutor_RandomWait : public FInstancedEventExecutor_Random
{
	GENERATED_BODY()

	/**
	 * Stop execution and broadcast Fail if executed event fails.
	 * This will cancel pending events
	 */
	UPROPERTY(EditAnywhere, Category=Execution)
	bool bStopOnFail = false;
	
protected:
	virtual void PreExecute(const FInstancedEventContext& Context, TArray<UInstancedEvent*>& EventsToExecute) override;
	virtual void OnResult(const FInstancedEventResult& Result) override;
};
