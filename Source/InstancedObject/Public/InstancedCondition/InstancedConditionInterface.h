// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InstancedConditionInterface.generated.h"

struct FInstancedConditionContext;


UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class INSTANCEDOBJECT_API UInstancedConditionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INSTANCEDOBJECT_API IInstancedConditionInterface
{
	GENERATED_BODY()
public:
	virtual bool Check(const FInstancedConditionContext& Context) = 0;
};
