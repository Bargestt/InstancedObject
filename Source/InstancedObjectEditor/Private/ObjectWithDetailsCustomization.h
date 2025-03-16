// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

class IPropertyHandle;

class FObjectWithDetailsCustomization : public IPropertyTypeCustomization
{
public:
	class FPropertyTypeIdentifier : public IPropertyTypeIdentifier
	{
	public:
		virtual bool IsPropertyTypeCustomized(const IPropertyHandle& PropertyHandle) const override
		{
			return PropertyHandle.HasMetaData(TEXT("ObjectWithDetails")) &&
				PropertyHandle.GetProperty() &&
				PropertyHandle.GetProperty()->IsA(FObjectPropertyBase::StaticClass());
		}
	};
	
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new FObjectWithDetailsCustomization);
	}

	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	//~ End IPropertyTypeCustomization Interface
protected:
	TSharedPtr<class FObjectDetailsBuilder> Builder;
};
