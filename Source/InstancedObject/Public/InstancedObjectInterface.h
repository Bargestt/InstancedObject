// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InstancedObjectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class INSTANCEDOBJECT_API UInstancedObjectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INSTANCEDOBJECT_API IInstancedObjectInterface
{
	GENERATED_BODY()
public:
	
	/**
	 * Returning empty will result in falling back to Class.GetDisplayNameText then to Class.GetName
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Instanced Object")
	FString GetInstancedObjectTitle(bool bFullTitle) const;	
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const
	{
		return TEXT("");
	}

public:
	static FString GetTitleSafe(const UObject* Object, bool bFullTitle);
	/* Object must implement interface or be null*/
	static FString GetTitleChecked(const UObject* Object, bool bFullTitle);
};
