// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#include "SInstancedObjectHeader.h"

#include "ClassViewerFilter.h"
#include "ClassViewerModule.h"
#include "Styling/SlateIconFinder.h"


#define LOCTEXT_NAMESPACE "FInstancedObjectEditorModule"

void SInstancedObjectHeader::Construct(const FArguments& InArgs, TSharedPtr<IPropertyHandle> InPropertyHandle)
{
	ObjectHandle = InPropertyHandle;

	MetaClass = InArgs._MetaClass;
	RequiredInterface = InArgs._RequiredInterface;
	ShowDisplayNames = InArgs._ShowDisplayNames;
	ShowNoneOption = InArgs._ShowNoneOption;
	OnSetClassOverride = InArgs._OnSetClassOverride;

	ChildSlot
	[
		SAssignNew(ComboButton, SComboButton)
		.IsEnabled(this, &SInstancedObjectHeader::IsValueEnabled)
		.OnGetMenuContent(this, &SInstancedObjectHeader::GenerateClassPicker)
		.ContentPadding(0)
		.ButtonContent()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			 .AutoWidth()
			 .VAlign(VAlign_Center)
			 .Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew( SImage )
				.Image( this, &SInstancedObjectHeader::GetDisplayValueIcon )
			]
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew( STextBlock )
				.Text( this, &SInstancedObjectHeader::GetObjectDisplayText )
				.Font( IPropertyTypeCustomizationUtils::GetRegularFont() )
			]
		]
	];
}

bool SInstancedObjectHeader::IsValueEnabled() const
{
	return !ObjectHandle->IsEditConst();
}

const FSlateBrush* SInstancedObjectHeader::GetDisplayValueIcon() const
{
	UObject* CurrentValue = nullptr;
	FPropertyAccess::Result Result = ObjectHandle->GetValue( CurrentValue );
	if( Result == FPropertyAccess::Success && CurrentValue != nullptr )
	{
		return FSlateIconFinder::FindIconBrushForClass(CurrentValue->GetClass());
	}
	
	return nullptr;
}

FText SInstancedObjectHeader::GetObjectDisplayText() const
{
	UObject* ObjectValue = nullptr;
	FPropertyAccess::Result Result = ObjectHandle->GetValue(ObjectValue);

	if (Result == FPropertyAccess::Fail)
	{
		return LOCTEXT("Fail", "Fail");
	}
	else if (Result == FPropertyAccess::MultipleValues)
	{
		return LOCTEXT("MultipleValues", "Multiple Values");
	}
	if (ObjectValue == nullptr)
	{
		return LOCTEXT("NoObject", "None");
	}

	return ObjectValue->GetClass()->GetDisplayNameText();
}

class FPropertyEditorInlineClassFilter : public IClassViewerFilter
{
public:
	const UClass* BaseClass;
	const UClass* InterfaceClass;

	/** Hierarchy of objects that own this property. Used to check against ClassWithin. */
	TSet< const UObject* > OwningObjects;

	bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs ) override
	{
		const bool bMatchesFlags = InClass->HasAnyClassFlags(CLASS_EditInlineNew) && 
			!InClass->HasAnyClassFlags(CLASS_Hidden | CLASS_HideDropDown | CLASS_Deprecated | CLASS_Abstract);

		if( bMatchesFlags && InClass->IsChildOf(BaseClass) &&
			(!InterfaceClass || InClass->ImplementsInterface(InterfaceClass)) )
		{
			// Verify that the Owners of the property satisfy the ClassWithin constraint of the given class.
			// When ClassWithin is null, assume it can be owned by anything.
			return InClass->ClassWithin == nullptr || InFilterFuncs->IfMatchesAll_ObjectsSetIsAClass(OwningObjects, InClass->ClassWithin) != EFilterReturn::Failed;
		}

		return false;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		const bool bMatchesFlags = InClass->HasAnyClassFlags(CLASS_EditInlineNew) && 
			!InClass->HasAnyClassFlags(CLASS_Hidden | CLASS_HideDropDown | CLASS_Deprecated | CLASS_Abstract);

		if( bMatchesFlags && InClass->IsChildOf(BaseClass) &&
			(!InterfaceClass || InClass->ImplementsInterface(InterfaceClass)) )
		{
			const UClass* ClassWithin = InClass->GetClassWithin();

			// Verify that the Owners of the property satisfy the ClassWithin constraint of the given class.
			// When ClassWithin is null, assume it can be owned by anything.
			return ClassWithin == nullptr || InFilterFuncs->IfMatchesAll_ObjectsSetIsAClass(OwningObjects, ClassWithin) != EFilterReturn::Failed;
		}
		return false;
	}
};

TSharedRef<SWidget> SInstancedObjectHeader::GenerateClassPicker()
{
	FClassViewerInitializationOptions Options;
	Options.bShowBackgroundBorder = false;
	Options.bShowUnloadedBlueprints = true;
	Options.NameTypeToDisplay = ShowDisplayNames ? EClassViewerNameTypeToDisplay::DisplayName : EClassViewerNameTypeToDisplay::ClassName;
	Options.bShowNoneOption = ShowNoneOption;
	Options.PropertyHandle = ObjectHandle;
	
	TSharedPtr<FPropertyEditorInlineClassFilter> ClassFilter = MakeShareable( new FPropertyEditorInlineClassFilter );
	Options.ClassFilters.Add(ClassFilter.ToSharedRef());
	ClassFilter->BaseClass = MetaClass ? MetaClass : UObject::StaticClass();
	ClassFilter->InterfaceClass = RequiredInterface;	

	TArray<UObject*> Outers;
	ObjectHandle->GetOuterObjects(Outers);
	for (UObject* Outer : Outers)
	{
		ClassFilter->OwningObjects.Add(Outer);	
	}

	FOnClassPicked OnPicked( FOnClassPicked::CreateSP( this, &SInstancedObjectHeader::OnClassPicked ) );
	return FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer").CreateClassViewer(Options, OnPicked);
}

void SInstancedObjectHeader::OnClassPicked(UClass* InClass)
{
	if (OnSetClassOverride.IsBound())
	{
		bool bWasHandled = false;
		OnSetClassOverride.Execute(InClass, bWasHandled);
		if (bWasHandled)
		{
			return;
		}
	}
	SetClass(InClass);
}

void SInstancedObjectHeader::SetClass(UClass* InClass)
{
	// This is copy of SPropertyEditorEditInline::OnClassPicked(UClass* InClass)
	// With some limitations because GetDefaultValueAsString is not available
	
	TArray<FString> NewValues;
		
	TArray<UObject*> Outers;
	ObjectHandle->GetOuterObjects(Outers);
		
	GEditor->BeginTransaction(TEXT("PropertyEditor"), NSLOCTEXT("PropertyEditor", "OnClassPicked", "Set Class"), nullptr /* PropertyNode->GetProperty()) */ );

	auto ExtractClassAndObjectNames = [](FStringView PathName, FStringView& ClassName, FStringView& ObjectName)
	{
		int32 ClassEnd;
		PathName.FindChar(TCHAR('\''), ClassEnd);
		if (ensure(ClassEnd != INDEX_NONE))
		{
			ClassName = PathName.Left(ClassEnd);
		}

		int32 LastPeriod, LastColon;
		PathName.FindLastChar(TCHAR('.'), LastPeriod);
		PathName.FindLastChar(TCHAR(':'), LastColon);
		const int32 ObjectNameStart = FMath::Max(LastPeriod, LastColon);

		if (ensure(ObjectNameStart != INDEX_NONE))
		{
			ObjectName = PathName.RightChop(ObjectNameStart + 1).LeftChop(1);
		}
	};

	FString NewObjectName;
	UObject* NewObjectTemplate = nullptr;

	// If we've picked the same class as our archetype, then we want to create an object with the same name and properties
//		if (InClass)
//		{
//			FString DefaultValue = PropertyNode->GetDefaultValueAsString(/*bUseDisplayName=*/false);
//			if (!DefaultValue.IsEmpty() && DefaultValue != FName(NAME_None).ToString())
//			{
//				FStringView ClassName, ObjectName;
//				ExtractClassAndObjectNames(DefaultValue, ClassName, ObjectName);
//				if (InClass->GetName() == ClassName)
//				{
//					NewObjectName = ObjectName;
//
//					ConstructorHelpers::StripObjectClass(DefaultValue);
//					NewObjectTemplate = StaticFindObject(InClass, nullptr, *DefaultValue);
//				}
//			}
//		}
		
	const TSharedRef<IPropertyHandle> PropertyHandle = ObjectHandle.ToSharedRef(); // PropertyEditor->GetPropertyHandle();

	// If this is an instanced component property collect current names so we can clean them properly if necessary
	TArray<FString> PrevPerObjectValues;
	FObjectProperty* ObjectProperty = CastFieldChecked<FObjectProperty>(PropertyHandle->GetProperty());
	if (ObjectProperty && ObjectProperty->HasAnyPropertyFlags(CPF_InstancedReference))
	{
		PropertyHandle->GetPerObjectValues(PrevPerObjectValues);
	}	

	for (UObject* OuterObject : Outers)
	{
		FString NewValue;
		if (InClass)
		{
			FStringView CurClassName, CurObjectName;
			if (PrevPerObjectValues.IsValidIndex(NewValues.Num()) && PrevPerObjectValues[NewValues.Num()] != FName(NAME_None).ToString())
			{
				ExtractClassAndObjectNames(PrevPerObjectValues[NewValues.Num()], CurClassName, CurObjectName);
			}

			if (CurObjectName == NewObjectName && InClass->GetName() == CurClassName)
			{
				NewValue = MoveTemp(PrevPerObjectValues[NewValues.Num()]);
				PrevPerObjectValues[NewValues.Num()].Reset();
			}
			else
			{
				UObject*		Object =  OuterObject; //Itor->Get();
				UObject*		UseOuter = (InClass->IsChildOf(UClass::StaticClass()) ? Cast<UClass>(Object)->GetDefaultObject() : Object);
				EObjectFlags	MaskedOuterFlags = UseOuter ? UseOuter->GetMaskedFlags(RF_PropagateToSubObjects) : RF_NoFlags;
				if (UseOuter && UseOuter->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
				{
					MaskedOuterFlags |= RF_ArchetypeObject;
				}

				if (NewObjectName.Len() > 0)
				{
					if (UObject* SubObject = StaticFindObject(UObject::StaticClass(), UseOuter, *NewObjectName))
					{
						SubObject->Rename(*MakeUniqueObjectName(GetTransientPackage(), SubObject->GetClass()).ToString(), GetTransientPackage(), REN_DontCreateRedirectors);

						// If we've renamed the object out of the way here, we don't need to do it again below
						if (PrevPerObjectValues.IsValidIndex(NewValues.Num()))
						{
							PrevPerObjectValues[NewValues.Num()].Reset();
						}
					}
				}

				UObject* NewUObject = NewObject<UObject>(UseOuter, InClass, *NewObjectName, MaskedOuterFlags, NewObjectTemplate);

				// Wrap the value in quotes before setting - in some cases for editinline-instanced values, the outer object path
				// can potentially contain a space token (e.g. if the outer object was instanced as a Blueprint template object
				// based on a user-facing variable name). While technically such characters should not be allowed, historically there
				// has been no issue with most tokens in the INVALID_OBJECTNAME_CHARACTERS set being present in in-memory object names,
				// other than some systems failing to resolve the object's path if the string representation of the value is not quoted.
				NewValue = FString::Printf(TEXT("\"%s\""), *NewUObject->GetPathName().ReplaceQuotesWithEscapedQuotes());
			}
		}
		else
		{
			NewValue = FName(NAME_None).ToString();
		}
		NewValues.Add(MoveTemp(NewValue));
	}
	
	PropertyHandle->SetPerObjectValues(NewValues);
	check(PrevPerObjectValues.Num() == 0 || PrevPerObjectValues.Num() == NewValues.Num());

	for (int32 Index = 0; Index < PrevPerObjectValues.Num(); ++Index)
	{
		if (PrevPerObjectValues[Index].Len() > 0 && PrevPerObjectValues[Index] != NewValues[Index])
		{
			// Move the old subobject to the transient package so GetObjectsWithOuter will not return it
			// This is particularly important for UActorComponent objects so resetting owned components on the parent doesn't find it
			ConstructorHelpers::StripObjectClass(PrevPerObjectValues[Index]);
			if (UObject* SubObject = StaticFindObject(UObject::StaticClass(), nullptr, *PrevPerObjectValues[Index]))
			{
				SubObject->Rename(nullptr, GetTransientOuterForRename(SubObject->GetClass()), REN_DontCreateRedirectors);
			}
		}
	}

	// End the transaction if we called PreChange
	GEditor->EndTransaction();
}

FString SInstancedObjectHeader::GetObjectFlags() const
{
	UObject* CurrentValue = nullptr;
	FPropertyAccess::Result Result = ObjectHandle->GetValue( CurrentValue );
	if( Result == FPropertyAccess::Success && CurrentValue != nullptr )
	{
		return LexToString(CurrentValue->GetFlags());
	}
	return TEXT("None");
}


#undef LOCTEXT_NAMESPACE