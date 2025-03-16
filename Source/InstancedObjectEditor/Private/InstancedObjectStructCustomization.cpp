// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#include "InstancedObjectStructCustomization.h"
#include "InstancedObjectBuilder.h"


void FInstancedObjectStructCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	const TSharedPtr<IPropertyHandle> ObjectHandle = PropertyHandle->GetChildHandle("Object", false);
	Builder = MakeShared<FInstancedObjectBuilder>(ObjectHandle.ToSharedRef(), PropertyHandle);	
	Builder->GenerateHeaderRowContent(HeaderRow);
}

void FInstancedObjectStructCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	Builder->GenerateChildContent(ChildBuilder);	
}