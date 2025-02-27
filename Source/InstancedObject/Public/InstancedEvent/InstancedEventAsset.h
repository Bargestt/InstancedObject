// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedEvent.h"
#include "UObject/Object.h"
#include "InstancedEventAsset.generated.h"


UCLASS(Abstract, BlueprintType, ClassGroup=(InstancedEvent))
class INSTANCEDOBJECT_API UInstancedEventAssetBase : public UDataAsset
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="InstancedEvent")
	virtual UInstancedEvent* GetEvent() const { return nullptr; }

	static UInstancedEvent* GetEventSafe(const UInstancedEventAssetBase* Asset) { return Asset ? Asset->GetEvent() : nullptr; }
};

/**
 * 
 */
UCLASS()
class INSTANCEDOBJECT_API UInstancedEventAsset : public UInstancedEventAssetBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InstancedEvent")
	FInstancedEventStruct Event;

	virtual UInstancedEvent* GetEvent() const override { return Event.Object; }
};
