// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedEvent.h"
#include "InstancedEvent_Wait.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName=".Wait"))
class INSTANCEDOBJECT_API UInstancedEvent_Wait : public UInstancedEvent_Operator
{
	GENERATED_BODY()
public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(ForceInlineRow))
	TMap<FGameplayTag, FGameplayTag> EventRemap;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(InheritFilters))
	FInstancedEventStruct Event;

	TArray<FGameplayTag> ReceivedEvents;
	
protected:
	virtual void ExecuteEvent(const FInstancedEventContext& Context) override;
public:
	virtual void Cancel() override;
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const override;

	virtual void OnResult(const FInstancedEventResult& Result);
};
