// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.


#include "InstancedObjectInterface.h"


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
