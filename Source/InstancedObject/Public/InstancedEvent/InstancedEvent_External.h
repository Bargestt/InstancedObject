// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedEvent.h"
#include "InstancedEvent_External.generated.h"

class UInstancedEventAssetBase;


UCLASS(meta=(DisplayName=".External"))
class INSTANCEDOBJECT_API UInstancedEvent_External : public UInstancedEvent_Operator
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InstancedEvent")
	bool bDuplicateOnExecution = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InstancedEvent")
	TObjectPtr<UInstancedEventAssetBase> Event;
protected:
	UPROPERTY()
	TObjectPtr<UInstancedEvent> LastExecution;

protected:
	virtual void ExecuteEvent(const FInstancedEventContext& Context) override;
	virtual void Cancel() override;
	virtual void GetSubEvents_Implementation(TArray<UInstancedEvent*>& OutEvents) const override;
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const override;

	virtual void OnResult(const FInstancedEventResult& Result);
};
