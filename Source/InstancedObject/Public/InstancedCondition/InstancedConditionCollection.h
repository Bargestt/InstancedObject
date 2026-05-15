// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedCondition.h"
#include "Engine/DataAsset.h"
#include "InstancedConditionCollection.generated.h"

USTRUCT(BlueprintType)
struct FInstancedConditionCollectionEntry
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InstancedEvent")
	FName Name;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InstancedEvent")
	FText DisplayName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InstancedEvent")
	FText Tooltip;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InstancedEvent")
	FInstancedConditionStruct Condition;
};

/**
 * 
 */
UCLASS(BlueprintType, ClassGroup=(InstancedEvent))
class INSTANCEDOBJECT_API UInstancedConditionCollection : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InstancedEvent", meta=(TitleProperty="Name"))
	TArray<FInstancedConditionCollectionEntry> Entries;	
	
public:
	UFUNCTION(BlueprintCallable, Category="InstancedEvent")
	FInstancedConditionCollectionEntry FindEntryByName(FName Name) const;
};
