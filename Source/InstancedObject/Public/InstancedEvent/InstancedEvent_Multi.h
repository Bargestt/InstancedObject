// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InstancedEvent.h"
#include "InstancedEvent_Multi.generated.h"



/** Execute multiple events */
UCLASS(NotBlueprintable, meta = (DisplayName = ".Multi"))
class INSTANCEDOBJECT_API UInstancedEvent_Multi : public UInstancedEvent_Operator
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", NoClear, meta=(ExcludebaseStruct))
	TInstancedStruct<struct FInstancedEventExecutor> Execution;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event")
	TArray<FInstancedEventStruct> Events;

	UInstancedEvent_Multi();
	virtual void ExecuteEvent(const FInstancedEventContext& Context) override;
	virtual void Cancel() override;
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const override;

	virtual void BeginDestroy() override;

protected:
	friend struct FInstancedEventExecutor;
	virtual void End();
};



USTRUCT(NotBlueprintType)
struct INSTANCEDOBJECT_API FInstancedEventExecutor
{
	GENERATED_BODY()
	
public:
	FInstancedEventExecutor() { }
	virtual ~FInstancedEventExecutor() { }

	virtual void Init(UInstancedEvent_Multi* InOwner) { Owner = InOwner; }
	virtual void Reset() { }
	virtual void Execute(const FInstancedEventContext& Context) { }
	virtual FString GetDisplayString(const UScriptStruct* ThisStruct) const {
#if WITH_EDITOR
		return ThisStruct->GetDisplayNameText().ToString();			
#else
		return ThisStruct->GetName();
#endif //
	}
protected:
	void End() const
	{		
		Owner->End();
	}
protected:
	UInstancedEvent_Multi* Owner = nullptr;
};


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
	TArray<TObjectPtr<UInstancedEvent>> WaitEvents;
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
protected:
	virtual void PreExecute(const FInstancedEventContext& Context, TArray<UInstancedEvent*>& EventsToExecute) override;
};




/** Execute events in random order and instantly finish */
USTRUCT(NotBlueprintType, meta=(DisplayName="Random"))
struct INSTANCEDOBJECT_API FInstancedEventExecutor_Random : public FInstancedEventExecutor_SequenceBase
{
	GENERATED_BODY()

	/** -1 disables randomization */
	UPROPERTY(EditAnywhere, Category=Execution)
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
protected:
	virtual void PreExecute(const FInstancedEventContext& Context, TArray<UInstancedEvent*>& EventsToExecute) override;
};



/** Execute events in sequence, but wait for event to finish before starting next */
USTRUCT(NotBlueprintType, meta=(DisplayName="One by One"))
struct INSTANCEDOBJECT_API FInstancedEventExecutor_OneByOne : public FInstancedEventExecutor
{
	GENERATED_BODY()
public:
	virtual void Reset() override;
	virtual void Execute(const FInstancedEventContext& Context) override;
	void NextEvent();
	void OnResult(const FInstancedEventResult& Result) { NextEvent(); }
protected:
	FInstancedEventContext CachedContext;
	TObjectPtr<UInstancedEvent> CurrentEvent;
	TArray<TObjectPtr<UInstancedEvent>> EventsToExecute;	
};