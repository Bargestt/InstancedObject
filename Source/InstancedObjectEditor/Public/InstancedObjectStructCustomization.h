// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

class IPropertyHandle;

class INSTANCEDOBJECTEDITOR_API FInstancedObjectStructCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new FInstancedObjectStructCustomization);
	}

	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	//~ End IPropertyTypeCustomization Interface
protected:
	
	TSharedRef<SWidget> CreateHeaderWidget();	
	TSharedRef<IToolTip> CreateTooltipWidget();

	TSharedRef<SWidget> CreateObjectHeaderWidget(IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils);	
	
	static const FString* FindMetaData(const TSharedPtr<IPropertyHandle>& ObjectHandle, const FName& Key);

	static bool CanDisplayChild(const TSharedRef<IPropertyHandle>& ChildHandle);
	
	void UpdateTitle();
	void UpdateTooltip();	
	FText ReadTitle(bool bFull) const;	


	
protected:
	TSharedPtr<IPropertyHandle> StructHandle;
	TSharedPtr<IPropertyHandle> ObjectHandle;

	
	TSharedPtr<SWidget> HeaderWidget;
	TSharedPtr<SWidget> Switcher;

	TSharedPtr<SBox> ObjectHeaderPanel;
	
	FText Title;
	FText GetTitle() const { return Title; }
	FText Tooltip;
	FText GetTooltip() const { return Tooltip; }
};
