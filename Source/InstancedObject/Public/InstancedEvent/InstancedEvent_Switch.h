// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedEvent.h"
#include "InstancedCondition/InstancedCondition.h"
#include "InstancedEvent_Switch.generated.h"

USTRUCT(BlueprintType)
struct FInstancedEvent_SwitchPair
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event")
	FInstancedConditionStruct Condition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(InheritFilters))
	FInstancedEventStruct Event;
};

/**
 * Execute one or multiple events that has condition successes
 */
UCLASS(NotBlueprintable, meta = (DisplayName = ".Switch"))
class INSTANCEDOBJECT_API UInstancedEvent_Switch : public UInstancedEvent_Operator
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event")
	bool bStopOnFirstSuccess = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event")
	TArray<FInstancedEvent_SwitchPair> Events;
	
protected:
	virtual void ExecuteEvent(const FInstancedEventContext& Context) override;
	virtual void GetSubEvents_Implementation(TArray<UInstancedEvent*>& OutEvents) const override;
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const override;
};
