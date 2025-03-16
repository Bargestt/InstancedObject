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
	virtual ~FInstancedObjectArrayStructBase()
	{
		
	}
	
	virtual TArray<UObject*> GetObjects() const { return {}; }
	virtual int32 GetNumObjects() const { return 0; }
	virtual UObject* GetObjectAt(int32 Index) const { return nullptr;}
	virtual void RemoveObjectAt(int32 Index) { }
	virtual int32 InsertObjectAt(UObject* Object, int32 Index) { return -1; }
};

