// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InstancedObjectArrayStruct.generated.h"



/**
 * Allows use of horizontal editor with AdvancedWidget metadata
 *  ---------------------------------
 *  |  Obj1		|	Obj2 Details	|
 *  | >Obj2		|   -Prop1			|
 *  |  Obj3		|   -Prop2			|
 *  |			|					|
 *  ---------------------------------
 */
USTRUCT(BlueprintType, BlueprintInternalUseOnly, meta=(Hidden))
struct INSTANCEDOBJECT_API FInstancedObjectArrayStructBase
{
	GENERATED_BODY();
public:
	virtual ~FInstancedObjectArrayStructBase() { }
};