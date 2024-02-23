// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once


DECLARE_DELEGATE_TwoParams( FSetInstanceClassOverride, const UClass* /*PickedClass*/, bool& /*WasHandled*/);

/*
 * Provides class picked for instanced objects with custom filtering
 */
class INSTANCEDOBJECTEDITOR_API SInstancedObjectHeader : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInstancedObjectHeader)
		: _MetaClass(UObject::StaticClass())
		, _RequiredInterface(nullptr)
		, _ShowDisplayNames(true)
		, _ShowNoneOption(true)
	{}
	/** The meta class that the selected class must be a child-of (required) */
	SLATE_ARGUMENT(const UClass*, MetaClass)
	/** An interface that the selected class must implement (optional) */
	SLATE_ARGUMENT(const UClass*, RequiredInterface)

	/** true to show class display names rather than their native names, false otherwise */
	SLATE_ARGUMENT(bool, ShowDisplayNames)

	/** Should we be able to select "None" as a class? (optional) */
	SLATE_ARGUMENT(bool, ShowNoneOption)

	/* Override default logic with custom one */
	SLATE_EVENT(FSetInstanceClassOverride, OnSetClassOverride)
	
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<IPropertyHandle> InPropertyHandle);
	
private:
	bool IsValueEnabled() const;	
	const FSlateBrush* GetDisplayValueIcon() const;
	FText GetObjectDisplayText() const;
	
	TSharedRef<SWidget> GenerateClassPicker();
	void OnClassPicked(UClass* InClass);

public:
	void SetClass(UClass* InClass);
	
	/* Use for debugging */
	FString GetObjectFlags() const;
	
private:
	/** The meta class that the selected class must be a child-of */
	const UClass* MetaClass;
	/** An interface that the selected class must implement */
	const UClass* RequiredInterface;

	bool ShowDisplayNames;
	bool ShowNoneOption;

	FSetInstanceClassOverride OnSetClassOverride;
	
	TSharedPtr<IPropertyHandle> ObjectHandle;
	TSharedPtr<SComboButton> ComboButton;
};