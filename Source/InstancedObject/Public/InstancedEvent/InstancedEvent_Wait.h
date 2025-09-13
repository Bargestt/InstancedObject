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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(InlineEditConditionToggle))
	bool bEnable_CancelEvent = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(EditCondition="bEnable_CancelEvent"))
	FGameplayTagContainer CancelEvent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(InheritFilters))
	FInstancedEventStruct Event;


	TWeakObjectPtr<UInstancedEvent> LastResult_Event;
	FGameplayTag LastResult_Type;
	FInstancedStruct LastResult_Data;
	TArray<FGameplayTag> ReceivedEvents;	
	
protected:
	virtual void ExecuteEvent(const FInstancedEventContext& Context) override;
public:
	virtual void Cancel() override;
	virtual void GetSubEvents_Implementation(TArray<UInstancedEvent*>& OutEvents) const override;
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const override;

	virtual void OnResult(const FInstancedEventResult& Result);


public:
	UFUNCTION(BlueprintCallable, Category=Event)
	UInstancedEvent* GetEvent() const { return Event.Object; }
	
	
	UFUNCTION(BlueprintCallable, Category=Event)
	bool HasResult() const;

	UFUNCTION(BlueprintCallable, Category=Event)
	bool HasMultipleResults() const;
	
	
	UFUNCTION(BlueprintCallable, Category=Event)
	bool HasSucceeded() const;

	UFUNCTION(BlueprintCallable, Category=Event)
	bool HasFailed() const;



	
	UFUNCTION(BlueprintCallable, Category=Event)
	FInstancedEventResult GetLastResult() const;

	UFUNCTION(BlueprintCallable, Category=Event)
	bool HasLastResultSucceeded() const;

	UFUNCTION(BlueprintCallable, Category=Event)
	bool HasLastResultFailed() const;
};
