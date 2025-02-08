// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedCondition/InstancedCondition.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_WaitInstancedCondition.generated.h"

/**
 * 
 */
UCLASS()
class INSTANCEDOBJECT_API UAsyncAction_WaitInstancedCondition : public UCancellableAsyncAction
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConditionResultEvent, bool, bResult);
	
	UPROPERTY(BlueprintAssignable)
	FConditionResultEvent OnResultChanged;
	
public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm="Context"))
	static UAsyncAction_WaitInstancedCondition* WaitInstancedCondition(UObject* WorldContextObject, const FInstancedConditionStruct& Condition, const FInstancedConditionContext& Context, bool bFinishOnSuccess = true, bool bFinishOnFail = false, float CheckPeriod = 0.2f, bool bNotifyEveryCheck = false);

	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;
	virtual UWorld* GetWorld() const override;
	void Check();
protected:
	UPROPERTY()
	TObjectPtr<UInstancedCondition> Condition;
	
	UPROPERTY()
	FInstancedConditionContext ConditionContext;
	
	bool bFinishOnSuccess;
	bool bFinishOnFail;

	float CheckPeriod;
	FTimerHandle Timer;

	bool bNotifyEveryCheck;
	bool bHasInitialized = false;
	bool bLastResult = false;
};
