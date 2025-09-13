// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedEvent.h"
#include "InstancedEvent_Branch.generated.h"

/**
 * Execute event and select branch on end
 */
UCLASS(NotBlueprintable, meta = (DisplayName = ".Branch"))
class INSTANCEDOBJECT_API UInstancedEvent_Branch : public UInstancedEvent_Operator
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(InheritFilters))
	FInstancedEventStruct Event;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(InheritFilters))
	FInstancedEventStruct OnSuccess;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(InheritFilters))
	FInstancedEventStruct OnFail;
	
protected:
	virtual void ExecuteEvent(const FInstancedEventContext& Context) override;
	virtual void Cancel() override;
	virtual void GetSubEvents_Implementation(TArray<UInstancedEvent*>& OutEvents) const override;
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const override;

	virtual void OnResult(const FInstancedEventResult& Result);
protected:
	UPROPERTY()
	FInstancedEventContext CachedContext;
};
