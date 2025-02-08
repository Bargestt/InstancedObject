// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedCondition.h"
#include "Engine/DataAsset.h"
#include "InstancedConditionAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class INSTANCEDOBJECT_API UInstancedConditionAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Condition)
	FInstancedConditionStruct Condition;
};
