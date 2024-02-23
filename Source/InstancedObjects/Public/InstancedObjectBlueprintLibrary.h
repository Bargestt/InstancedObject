// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InstancedObjectBlueprintLibrary.generated.h"


struct FInstancedObject;

/**
 * 
 */
UCLASS()
class INSTANCEDOBJECTS_API UInstancedObjectBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintPure, Category = "Utility|InstancedObject")
	static bool IsInstancedObjectValid(const FInstancedObject& Object);

	UFUNCTION(BlueprintPure, Category = "Utility|InstancedObject")
	static FString GetInstancedObjectTitle(const FInstancedObject& Object, bool bFullTitle);
};
