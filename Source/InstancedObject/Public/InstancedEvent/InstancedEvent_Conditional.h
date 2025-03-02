// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedEvent.h"
#include "InstancedCondition/InstancedCondition.h"
#include "InstancedEvent_Conditional.generated.h"

/**
 * Execute event if condition succeeds
 */
UCLASS(NotBlueprintable, meta = (DisplayName = ".Conditional"))
class INSTANCEDOBJECT_API UInstancedEvent_Conditional : public UInstancedEvent_Operator
{
	GENERATED_BODY()	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event")
	FInstancedConditionStruct Condition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(InheritFilters))
	FInstancedEventStruct Event;
	
protected:
	virtual void ExecuteEvent(const FInstancedEventContext& Context) override;
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const override;
};
