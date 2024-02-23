// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InstancedObjectInterface.h"
#include "UObject/Object.h"
#include "DefaultInstancedObject.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, HideCategories=(Hidden))
class INSTANCEDOBJECTS_API UDefaultInstancedObject : public UObject, public IInstancedObjectInterface
{
	GENERATED_BODY()
};

UCLASS(Abstract, CollapseCategories)
class INSTANCEDOBJECTS_API UDefaultInstancedObject_NoCategories : public UDefaultInstancedObject
{
	GENERATED_BODY()
};