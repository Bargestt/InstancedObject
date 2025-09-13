// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedEvent.h"
#include "InstancedEvent_SwitchOnResult.generated.h"

USTRUCT(BlueprintType)
struct FInstancedEvent_SwitchOnResult_Entry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event")
	bool bExactTag = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event")
	bool bEndExecution = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(InheritFilters))
	FInstancedEventStruct Event;
};

/**
 * Execute event and switch on received results
 */
UCLASS(NotBlueprintable, meta = (DisplayName = ".Switch on Result"))
class INSTANCEDOBJECT_API UInstancedEvent_SwitchOnResult : public UInstancedEvent_Operator
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(InheritFilters))
	FInstancedEventStruct Event;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(ForceInlineRow))
	TMap<FGameplayTag, FInstancedEvent_SwitchOnResult_Entry> Entries;
	
protected:
	virtual void ExecuteEvent(const FInstancedEventContext& Context) override;
	virtual void Cancel() override;
	virtual void GetSubEvents_Implementation(TArray<UInstancedEvent*>& OutEvents) const override;
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const override;

	virtual void OnResult(const FInstancedEventResult& Result);
protected:
	FInstancedEventContext CachedContext;
};
