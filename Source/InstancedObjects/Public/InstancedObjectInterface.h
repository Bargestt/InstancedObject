// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InstancedObjectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class INSTANCEDOBJECTS_API UInstancedObjectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INSTANCEDOBJECTS_API IInstancedObjectInterface
{
	GENERATED_BODY()
public:
	
	/**
	 * Automatically applies indent to braces ().
	 * Start text with <NoIndent> to disable indentation for this text
	 * Returning empty will result in falling back to Class.GetDisplayNameText then to Class.GetName
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Instanced Object")
	FString GetInstancedObjectTitle(bool bFullTitle) const;	
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const
	{
		return TEXT("");
	}

public:
	static FString GetTitleSafe(UObject* Object, bool bFullTitle);
};
