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

	
	static UInstancedEvent* GetEvent(const UInstancedEventAssetBase* Asset) { return Asset ? Asset->GetEvent() : nullptr; }

	template<typename T>
	static T* GetEvent(const UInstancedEventAssetBase* Asset) { return Asset ? Cast<T>(Asset->GetEvent()) : nullptr; }

	template<typename T>
	static bool GetEvent(const UInstancedEventAssetBase* Asset, T*& OutEvent)
	{
		OutEvent = Asset ? Cast<T>(Asset->GetEvent()) : nullptr;
		return OutEvent != nullptr;
	}
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
