// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedEvent.h"
#include "Engine/DataAsset.h"
#include "InstancedEventCollection.generated.h"


USTRUCT(BlueprintType)
struct FInstancedEventCollectionEntry
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InstancedEvent")
	FName Name;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InstancedEvent")
	FText DisplayName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InstancedEvent")
	FText Tooltip;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InstancedEvent")
	FInstancedEventStruct Event;
};


/**
 * 
 */
UCLASS(BlueprintType, ClassGroup=(InstancedEvent))
class INSTANCEDOBJECT_API UInstancedEventCollection : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InstancedEvent", meta=(TitleProperty="Name"))
	TArray<FInstancedEventCollectionEntry> Entries;	
	
public:
	UFUNCTION(BlueprintCallable, Category="InstancedEvent")
	FInstancedEventCollectionEntry FindEntryByName(FName Name) const;
};
