// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.


#include "InstancedObjectInterface.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(InstancedObjectInterface)

FString IInstancedObjectInterface::GetTitleSafe(UObject* Object, bool bFullTitle)
{
	if (!Object)
	{
		return TEXT("None");
	}

	if (!Object->Implements<UInstancedObjectInterface>())
	{
		return TEXT("Must implement UInstancedObjectInterface: ") + Object->GetName();
	}
		
	return GetTitleChecked(Object, bFullTitle);
}

FString IInstancedObjectInterface::GetTitleChecked(UObject* Object, bool bFullTitle)
{
	if (!Object)
	{
		return TEXT("None");
	}
	
	FString Title = IInstancedObjectInterface::Execute_GetInstancedObjectTitle(Object, bFullTitle);
	if (Title.IsEmpty())
	{
#if WITH_EDITORONLY_DATA
		Title = Object->GetClass()->GetDisplayNameText().ToString();
		if (Title.IsEmpty())
		{
			Title = Object->GetClass()->GetName();
		}
#else
		Title = Object->GetClass()->GetName();
#endif
	}

	return Title;
}
