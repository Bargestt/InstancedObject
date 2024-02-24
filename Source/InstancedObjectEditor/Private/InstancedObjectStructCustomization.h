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
private:
	

	
	TSharedRef<IToolTip> CreateTooltipWidget();
	void UpdateTitle();
	void UpdateTooltip();	
	FText ReadTitle(bool bFull) const;	

	int32 ParseIndent(FString InString, FString& OutString) const;

	
private:
	TSharedPtr<IPropertyHandle> ObjectHandle;
	TSharedPtr<SWidgetSwitcher> Switcher;
	bool bRemoveBrackets;
	int32 IndentSize = 4;
	FText Title;
	FText GetTitle() const { return Title; }
	FText Tooltip;
	FText GetTooltip() const { return Tooltip; }
};
