// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedEvent.h"
#include "InstancedCondition/InstancedCondition.h"
#include "InstancedEvent_WaitCondition.generated.h"

/**
 * Wait condition success to execute event and finish 
 */
UCLASS(NotBlueprintable, meta = (DisplayName = ".Wait Condition"))
class INSTANCEDOBJECT_API UInstancedEvent_WaitCondition : public UInstancedEvent_Operator
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event")
	float CheckPeriod = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event")
	FInstancedConditionStruct Condition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(InheritFilters))
	FInstancedEventStruct Event;
protected:
	virtual void ExecuteEvent(const FInstancedEventContext& Context) override;
	virtual void Cancel() override;
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const override;
	virtual void Check();
protected:
	FInstancedEventContext CachedEventContext;
	FInstancedConditionContext CachedConditionContext;
	
	TWeakObjectPtr<UWorld> TimerWorld;	
	FTimerHandle TimerHandle;	
};
