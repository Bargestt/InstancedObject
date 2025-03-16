// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#include "InstancedObjectViewCustomization.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "InstancedObjectEditorUtils.h"


#define LOCTEXT_NAMESPACE "FInstancedObjectEditorModule"



void FInstancedObjectViewCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	if (PropertyHandle->HasMetaData(TEXT("ShowOnlyInnerProperties")))
	{
		return;
	}

	HeaderRow.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	];
}

void FInstancedObjectViewCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TSharedPtr<IPropertyHandle> ObjectHandle = PropertyHandle->GetChildHandle("Object", false);	
	if (ObjectHandle.IsValid())
	{
		TSharedPtr<IPropertyHandle> ObjectInstanceHandle = ObjectHandle->GetChildHandle(0);
		if(ObjectInstanceHandle.IsValid())
		{
			uint32 NumChildren;
			if (ObjectInstanceHandle->GetNumChildren(NumChildren) == FPropertyAccess::Success)
			{
				for (uint32 Index = 0; Index < NumChildren; Index++)
				{
					TSharedRef<IPropertyHandle> ChildHandle = ObjectInstanceHandle->GetChildHandle(Index).ToSharedRef();
					if (FInstancedObjectEditorUtils::CanDisplayChild(ChildHandle))
					{
						ChildBuilder.AddProperty(ChildHandle);
					}		
				}
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE