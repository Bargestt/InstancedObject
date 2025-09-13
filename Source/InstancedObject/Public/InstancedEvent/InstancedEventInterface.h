// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InstancedEventInterface.generated.h"


struct FInstancedEventContext;

UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class INSTANCEDOBJECT_API UInstancedEventInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INSTANCEDOBJECT_API IInstancedEventInterface
{
	GENERATED_BODY()
public:
	virtual void Execute(const FInstancedEventContext& Context) = 0;

	virtual void Cancel() { };
};
