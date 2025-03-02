// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedEvent.h"
#include "InstancedEvent_Delay.generated.h"

/**
 * Execute child event after delay
 */
UCLASS(NotBlueprintable, meta = (DisplayName = ".Delay"))
class INSTANCEDOBJECT_API UInstancedEvent_Delay : public UInstancedEvent_Operator
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event")
	FVector2D DelayRange = FVector2D(1, -1);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(InheritFilters))
	FInstancedEventStruct Event;

protected:
	virtual void ExecuteEvent(const FInstancedEventContext& Context) override;
	virtual void Cancel() override;
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const override;
	virtual void Execute();
protected:
	FInstancedEventContext CachedContext;
	
	TWeakObjectPtr<UWorld> TimerWorld;	
	FTimerHandle TimerHandle;	
};
