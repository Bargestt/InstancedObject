// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedEvent/InstancedEvent.h"
#include "InstancedEvent_ExecutionLimit.generated.h"

/**
 * 
 */
UCLASS(NotBlueprintable, meta = (DisplayName = ".Execution Limit"))
class INSTANCEDOBJECT_API UInstancedEvent_ExecutionLimit : public UInstancedEvent_Operator
{
	GENERATED_BODY()
public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category="Event")
	bool bReset = false;
#endif 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(InlineEditConditionToggle))
	bool bExecutionTimeout = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(EditCondition="bExecutionTimeout"))
	FVector2D ExecutionTimeout = FVector2D(1, -1);

	double Timeout = -1;
	double LastExecution = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(InlineEditConditionToggle))
	bool bLimitNumber;
	
	/** Min, Max */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(EditCondition="bLimitNumber"))
	FIntPoint LimitNumber = FIntPoint(1, -1);

	int32 ExecutionsLeft;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(InheritFilters))
	FInstancedEventStruct Event;

public:
	virtual void ExecuteEvent(const FInstancedEventContext& Context) override;
	virtual void Cancel() override;
	virtual void GetSubEvents_Implementation(TArray<UInstancedEvent*>& OutEvents) const override;
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif 
};


