// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InstancedObjectInterface.h"
#include "InstancedObjectStruct.h"
#include "UObject/Object.h"
#include "InstancedObject.generated.h"


/*
 * Instanced object with custom display Title and Tooltip
 *
 * Can filter selection using BaseClass meta data(USTRUCT or UPROPERTY)
 */
USTRUCT(BlueprintType, meta=(AdvancedWidget, IndentSize=4))
struct INSTANCEDOBJECT_API FInstancedObjectStruct : public FInstancedObjectStructBase
{
	GENERATED_BODY();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category="Object", meta=(MustImplement="/Script/InstancedObjects.InstancedObjectInterface"))
	TObjectPtr<UObject> Object;

	virtual UObject* Get() const override { return Object; }
};



UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, HideCategories=(Hidden))
class INSTANCEDOBJECT_API UInstancedObject : public UObject, public IInstancedObjectInterface
{
	GENERATED_BODY()
};

UCLASS(Abstract, CollapseCategories)
class INSTANCEDOBJECT_API UInstancedObject_NoCategories : public UInstancedObject
{
	GENERATED_BODY()
};

UCLASS()
class INSTANCEDOBJECT_API UInstancedObjectBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "InstancedObject")
	static bool IsInstancedObjectValid(const FInstancedObjectStruct& Struct)
	{
		return IsValid(Struct.Object);
	}
};
