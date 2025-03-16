// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ObjectWithDetails.generated.h"



/** 
 * Editor only struct that will display object properties
 *
 * Use meta to control
 *   AutoExpand - always expand
 *   Categories - comma separated list of categories to display
 *   Padding - offset displayed properties
 *   MetaClass - To filter selectable objects
 *   ShowEditWidget - Toggles asset picker
 */
USTRUCT(BlueprintType, meta=(ShowEditWidget = true))
struct FObjectWithDetails
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Object)
	TObjectPtr<UObject> Object;

	template<typename T>
	T* Get() const { return Cast<T>(Object); }

	template<typename T>
	void Get(T*& Out) const { Out = Cast<T>(Object); }
};



USTRUCT(BlueprintType, meta=(ShowEditWidget = true))
struct FSoftObjectWithDetails
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Object)
	TSoftObjectPtr<UObject> Object;

	template<typename T>
	TSoftObjectPtr<T> Get() const { return TSoftObjectPtr<T>(Object); }
};