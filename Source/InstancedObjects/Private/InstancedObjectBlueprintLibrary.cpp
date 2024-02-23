// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.


#include "InstancedObjectBlueprintLibrary.h"

#include "InstancedObjectInterface.h"
#include "InstancedObjectStruct.h"

bool UInstancedObjectBlueprintLibrary::IsInstancedObjectValid(const FInstancedObject& Object)
{
	return Object.IsValid();
}

FString UInstancedObjectBlueprintLibrary::GetInstancedObjectTitle(const FInstancedObject& Object, bool bFullTitle)
{
	return IInstancedObjectInterface::GetTitleSafe(Object.Get(), bFullTitle);
}
