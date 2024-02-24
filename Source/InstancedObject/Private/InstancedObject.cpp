// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.


#include "InstancedObject.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(InstancedObject)


bool UInstancedObjectBlueprintLibrary::IsInstancedObjectValid(const FInstancedObjectStruct& Object)
{
	return Object.IsValid();
}

FString UInstancedObjectBlueprintLibrary::GetInstancedObjectTitle(const FInstancedObjectStruct& Object, bool bFullTitle)
{
	return IInstancedObjectInterface::GetTitleSafe(Object.Get(), bFullTitle);
}
