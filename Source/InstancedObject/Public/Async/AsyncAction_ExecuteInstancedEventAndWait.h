// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedEvent/InstancedEvent.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_ExecuteInstancedEventAndWait.generated.h"



/**
 * 
 */
UCLASS()
class INSTANCEDOBJECT_API UAsyncAction_ExecuteInstancedEventAndWait : public UCancellableAsyncAction
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEvent, const FInstancedEventResult&, Result);
	UPROPERTY(BlueprintAssignable)
	FOnEvent OnEnd;

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm="Context"))
	static UAsyncAction_ExecuteInstancedEventAndWait* ExecuteInstancedEventAndWait(UObject* WorldContextObject, const FInstancedEventStruct& Event, const FInstancedEventContext& Context, bool bOnce = true, bool bDuplicateEvent = false);

protected:
	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;
	virtual void Cancel() override;

protected:
	virtual UWorld* GetWorld() const override;

	void OnResult(const FInstancedEventResult& Result);

protected:
	UPROPERTY()
	TObjectPtr<UInstancedEvent> InstancedEvent;
	
	UPROPERTY()
	FInstancedEventContext InstancedEventContext;

	bool bOnce = false;
	bool bDuplicate = false;
	
};
