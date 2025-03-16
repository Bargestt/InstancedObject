#include "InstancedObjectEditorUtils.h"

#include "ClassViewerFilter.h"
#include "IDetailChildrenBuilder.h"
#include "InstancedObjectArrayStruct.h"
#include "InstancedObjectInterface.h"
#include "InstancedObjectStruct.h"
#include "InstancedObjectUtilityLibrary.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyRestriction.h"
#include "String/ParseTokens.h"

#define LOCTEXT_NAMESPACE "FInstancedObjectEditor"


const FName FInstancedObjectMeta::MD_ShowOnlyInnerProperties = TEXT("ShowOnlyInnerProperties");
const FName FInstancedObjectMeta::MD_AdvancedWidget = TEXT("AdvancedWidget");
const FName FInstancedObjectMeta::MD_IndentSize = TEXT("IndentSize");
const FName FInstancedObjectMeta::MD_BaseClass = TEXT("BaseClass");	
const FName FInstancedObjectMeta::MD_MustImplement = TEXT("MustImplement");
const FName FInstancedObjectMeta::MD_InheritFilters = TEXT("InheritFilters");
const FName FInstancedObjectMeta::MD_HideChild = TEXT("Hidden");


const FName FInstancedObjectMeta::MD_Header = TEXT("Header");
const FName FInstancedObjectMeta::MD_MinWidth = TEXT("MinWidth");
const FName FInstancedObjectMeta::MD_MaxWidth = TEXT("MaxWidth");
const FName FInstancedObjectMeta::MD_MinHeight = TEXT("MinHeight");
const FName FInstancedObjectMeta::MD_MaxHeight = TEXT("MaxHeight");

const FName FInstancedObjectMeta::MD_Padding = TEXT("Padding");
const FName FInstancedObjectMeta::MD_PaddingL = TEXT("PaddingL");
const FName FInstancedObjectMeta::MD_PaddingR = TEXT("PaddingR");
const FName FInstancedObjectMeta::MD_PaddingT = TEXT("PaddingT");
const FName FInstancedObjectMeta::MD_PaddingB = TEXT("PaddingB");

const FName FInstancedObjectMeta::MD_HideName = TEXT("HideName");


class FInstancedObjectClassFilter : public IClassViewerFilter
{
public:
	/** The meta class for the property that classes must be a child-of. */
	const UClass* ClassPropertyMetaClass = nullptr;

	/** The interface that must be implemented. */
	const UClass* InterfaceThatMustBeImplemented = nullptr;

	/** Whether or not abstract classes are allowed. */
	bool bAllowAbstract = false;

	/** Classes that can be picked */
	TArray<const UClass*> AllowedClassFilters;

	/** Classes that can't be picked */
	TArray<const UClass*> DisallowedClassFilters;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs ) override
	{
		return IsClassAllowedHelper(InClass);
	}
	
	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InBlueprint, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		return IsClassAllowedHelper(InBlueprint);
	}

private:

	template <typename TClass>
	bool IsClassAllowedHelper(TClass InClass)
	{
		bool bMatchesFlags = !InClass->HasAnyClassFlags(CLASS_Hidden | CLASS_HideDropDown | CLASS_Deprecated) &&
			(bAllowAbstract || !InClass->HasAnyClassFlags(CLASS_Abstract));

		if (bMatchesFlags && InClass->IsChildOf(ClassPropertyMetaClass)
			&& (!InterfaceThatMustBeImplemented || InClass->ImplementsInterface(InterfaceThatMustBeImplemented)))
		{
			auto PredicateFn = [InClass](const UClass* Class)
			{
				return InClass->IsChildOf(Class);
			};

			if (DisallowedClassFilters.FindByPredicate(PredicateFn) == nullptr &&
				(AllowedClassFilters.Num() == 0 || AllowedClassFilters.FindByPredicate(PredicateFn) != nullptr))
			{
				return true;
			}
		}

		return false;
	}
};


const FString* FInstancedObjectEditorUtils::FindMetaData(const TSharedPtr<IPropertyHandle>& ObjectHandle, const FName& Key)
{
	TSharedPtr<IPropertyHandle> StructHandle = ObjectHandle->GetParentHandle();
	
	TSharedPtr<IPropertyHandle> OwnerHandle = StructHandle->GetParentHandle();
	
	const bool bIsInContainer = OwnerHandle.IsValid() && OwnerHandle->GetProperty() &&
			(OwnerHandle->GetProperty()->IsA<FArrayProperty>() ||
			OwnerHandle->GetProperty()->IsA<FSetProperty>() ||
			OwnerHandle->GetProperty()->IsA<FMapProperty>());
	
	if (bIsInContainer)
	{
		if(OwnerHandle->HasMetaData(Key))
		{
			return &OwnerHandle->GetMetaData(Key);
		}
	}
	
	if(StructHandle->HasMetaData(Key))
	{
		return &StructHandle->GetMetaData(Key);
	}

	if(ObjectHandle->HasMetaData(Key))
	{
		return &ObjectHandle->GetMetaData(Key);
	}

	
	if (const FStructProperty* Prop = CastField<FStructProperty>(StructHandle->GetProperty()))
	{
		if (Prop->Struct)
		{
			if (Prop->Struct->HasMetaData(Key))
			{
				return &Prop->Struct->GetMetaData(Key);
			}		
		}
	}	

	return nullptr;
}

TSharedPtr<IClassViewerFilter> FInstancedObjectEditorUtils::CreateClassFilerFrom(const TSharedRef<IPropertyHandle>& SourceHandle)
{
	const UClass* ClassPropertyMetaClass = nullptr;
	const UClass* InterfaceThatMustBeImplemented = nullptr;
	TArray<const UClass*> AllowedClassFilters;
	TArray<const UClass*> DisallowedClassFilters;		
		
	{
		const FString* ClassName = FindMetaData(SourceHandle, FInstancedObjectMeta::MD_MustImplement);
		if (ClassName && !ClassName->IsEmpty())
		{
			InterfaceThatMustBeImplemented = UClass::TryFindTypeSlow<UClass>(*ClassName);
		}
	}
		
	{
		const FString* ClassName = FindMetaData(SourceHandle, FInstancedObjectMeta::MD_BaseClass);
		if (ClassName && !ClassName->IsEmpty())
		{
			ClassPropertyMetaClass = UClass::TryFindTypeSlow<UClass>(*ClassName);
		}		
	}

	if (!ClassPropertyMetaClass)
	{
		ClassPropertyMetaClass = UObject::StaticClass();
	}

	if (const FString* Meta = FindMetaData(SourceHandle, TEXT("AllowedClasses")))
	{
		AllowedClassFilters = PropertyCustomizationHelpers::GetClassesFromMetadataString(*Meta);
	}
	if (const FString* Meta = FindMetaData(SourceHandle, TEXT("DisallowedClasses")))
	{
		DisallowedClassFilters = PropertyCustomizationHelpers::GetClassesFromMetadataString(*Meta);
	}

	if (InterfaceThatMustBeImplemented == nullptr &&
		ClassPropertyMetaClass == UObject::StaticClass() &&
		AllowedClassFilters.IsEmpty() &&
		DisallowedClassFilters.IsEmpty())
	{
		return nullptr;
	}

	TSharedRef<FInstancedObjectClassFilter> ClassFilter = MakeShared<FInstancedObjectClassFilter>();		
	ClassFilter->bAllowAbstract = false;
	ClassFilter->InterfaceThatMustBeImplemented = InterfaceThatMustBeImplemented;
	ClassFilter->ClassPropertyMetaClass = ClassPropertyMetaClass;
	ClassFilter->AllowedClassFilters = AllowedClassFilters;
	ClassFilter->DisallowedClassFilters = DisallowedClassFilters;
	return ClassFilter;
}

void FInstancedObjectEditorUtils::CreateClassRestrictions(const TSharedRef<IPropertyHandle>& ObjectHandle, const TSharedPtr<IPropertyHandle>& SourceOverride)
{
	TSharedPtr<IClassViewerFilter> ClassViewerFilter = CreateClassFilerFrom(SourceOverride.IsValid() ? SourceOverride.ToSharedRef() : ObjectHandle);
	if (ClassViewerFilter.IsValid())
	{
		TSharedRef<FPropertyRestriction> Restriction = MakeShared<FPropertyRestriction>(LOCTEXT("InstancedObjectRestriction", "Instanced Object Restriction"));
		Restriction->AddClassFilter(ClassViewerFilter.ToSharedRef());		
		ObjectHandle->AddRestriction(Restriction);
	}	
}

void FInstancedObjectEditorUtils::CreateInheritedClassRestrictions(const TSharedRef<IPropertyHandle>& ObjectHandle, const TSharedPtr<IPropertyHandle>& StartSearchFrom)
{
	TSharedPtr<IPropertyHandle> SourceHandle;
	
	for (TSharedPtr<IPropertyHandle> ParentHandle = StartSearchFrom.IsValid() ? StartSearchFrom->GetParentHandle() : ObjectHandle->GetParentHandle(); ParentHandle.IsValid(); ParentHandle = ParentHandle->GetParentHandle())
	{
		const FStructProperty* ParentProp = CastField<FStructProperty>(ParentHandle->GetProperty());
		if (ParentProp && ParentProp->Struct)
		{				
			if (ParentProp->Struct->IsChildOf(FInstancedObjectStructBase::StaticStruct()))
			{
				SourceHandle = ParentHandle->GetChildHandle("Object", false);
				break;
			}
			else if (ParentProp->Struct->IsChildOf(FInstancedObjectArrayStructBase::StaticStruct()))
			{
				SourceHandle = ParentHandle->GetChildHandle("Objects", false);
				break;
			}				
		}
	}

	if (SourceHandle.IsValid())
	{
		TSharedPtr<IClassViewerFilter> ClassViewerFilter = CreateClassFilerFrom(SourceHandle.ToSharedRef());
		if (ClassViewerFilter.IsValid())
		{
			TSharedRef<FPropertyRestriction> Restriction = MakeShared<FPropertyRestriction>(LOCTEXT("InstancedObjectRestriction_Inherited", "Instanced Object Restriction (Inherited)"));
			Restriction->AddClassFilter(ClassViewerFilter.ToSharedRef());		
			ObjectHandle->AddRestriction(Restriction);
		}
	}
}

bool FInstancedObjectEditorUtils::CanDisplayChild(const TSharedRef<IPropertyHandle>& ChildHandle, bool bAllowCategories)
{
	if (ChildHandle->IsCategoryHandle())
	{
		return bAllowCategories;
	}
	
	const FProperty* Property = ChildHandle->GetProperty();
	return Property &&
			!Property->HasAnyPropertyFlags(CPF_Protected) &&
			!ChildHandle->GetBoolMetaData(TEXT("BlueprintProtected")) &&
			!ChildHandle->GetBoolMetaData(TEXT("BlueprintPrivate")) &&
			!ChildHandle->GetBoolMetaData(FInstancedObjectMeta::MD_HideChild);
}

FText FInstancedObjectEditorUtils::ReadObjectTitle(const TSharedPtr<IPropertyHandle>& ObjectHandle, bool bFull, bool bRemoveStyle)
{
	UObject* ObjectValue = nullptr;
	FPropertyAccess::Result Result = ObjectHandle.IsValid() ? ObjectHandle->GetValue(ObjectValue) : FPropertyAccess::Fail;

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

	FString String = IInstancedObjectInterface::GetTitleSafe(ObjectValue, bFull);

	int32 IndentSize = 4;
	
	bool bNoIndent = String.RemoveFromStart(TEXT("<NoIndent>"));	
	if (bFull && IndentSize >= 0 && !bNoIndent)
	{
		FIndentParser Parser;
		Parser.IndentSize = IndentSize;

		int32 Error;
		FString IndentedString = Parser.Apply(String, Error);
		if (Error > 0)
		{
			IndentedString += FString::Printf(TEXT("\nmissing %d brackets"), Error);
		}
		else if (Error < 0)
		{
			IndentedString += FString::Printf(TEXT("\nextra %d brackets"), Error);
		}
		String = IndentedString;
	}

	if (bRemoveStyle)
	{
		String = UInstancedObjectUtilityLibrary::ClearStylesFromString(String);
	}
	
	return FText::FromString(String);		
}

TSharedRef<SWidget> FInstancedObjectEditorUtils::CreateHeader_ChildrenEditors(const TSharedPtr<IPropertyHandle>& ObjectHandle, IDetailChildrenBuilder& ChildBuilder)
{
	TSharedRef<SWidget> Result = SNullWidget::NullWidget;
	if (!ObjectHandle.IsValid())
	{
		return Result;
	}	
	
	TSharedPtr<IPropertyHandle> ObjectInstanceHandle = ObjectHandle->GetChildHandle(0);
	if(!ObjectInstanceHandle.IsValid())
	{
		return Result;
	}

	uint32 NumChildren;
	if (ObjectInstanceHandle->GetNumChildren(NumChildren) != FPropertyAccess::Success || NumChildren == 0)
	{
		return Result;
	}

	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox);		
	for (uint32 Index = 0; Index < NumChildren; Index++)
	{
		TSharedRef<IPropertyHandle> ChildHandle = ObjectInstanceHandle->GetChildHandle(Index).ToSharedRef();
		if (!CanDisplayChild(ChildHandle))
		{
			continue;
		}

		if(!ChildHandle->HasMetaData(FInstancedObjectMeta::MD_Header))
		{
			continue;
		}		
		
		TMap<FName, FString> HeaderMetaData;
		{
			const FString& MetaData = ChildHandle->GetMetaData(FInstancedObjectMeta::MD_Header);
			ParseTokens(MetaData, ',', [&HeaderMetaData](const FStringView& View)
			{
				int32 Index = INDEX_NONE;
				if (View.FindChar(':', Index))
				{
					HeaderMetaData.Add(*FString(View.Left(Index)), FString(View.RightChop(Index + 1)).TrimStartAndEnd());
					
				}
				else
				{
					HeaderMetaData.Add(*FString(View));
				}
			
			}, UE::String::EParseTokensOptions::Trim | UE::String::EParseTokensOptions::SkipEmpty);
		}

		TSharedPtr<SWidget> ValueWidget;
		if (CastField<FStructProperty>(ChildHandle->GetProperty()))
		{
			ValueWidget = ChildBuilder.GenerateStructValueWidget(ChildHandle);
		}
		else
		{
			ValueWidget = ChildHandle->CreatePropertyValueWidget();
		}

		if (!HeaderMetaData.Contains(FInstancedObjectMeta::MD_HideName))
		{
			Box->AddSlot().AutoWidth().Padding(10, 0, 5, 0).VAlign(VAlign_Center)
			[
				ChildHandle->CreatePropertyNameWidget()
			];
		}

		const int32 MinWidth = FInstancedObjectMeta::ReadMetaFromMap(HeaderMetaData, FInstancedObjectMeta::MD_MinWidth, 0);						
		const int32 MaxWidth = FInstancedObjectMeta::ReadMetaFromMap(HeaderMetaData, FInstancedObjectMeta::MD_MaxWidth, 0);

		const int32 Padding = FInstancedObjectMeta::ReadMetaFromMap(HeaderMetaData, FInstancedObjectMeta::MD_Padding, 0);
		const int32 PaddingL = FInstancedObjectMeta::ReadMetaFromMap(HeaderMetaData, FInstancedObjectMeta::MD_PaddingL, 0);
		const int32 PaddingR = FInstancedObjectMeta::ReadMetaFromMap(HeaderMetaData, FInstancedObjectMeta::MD_PaddingR, 0);
		const int32 PaddingT = FInstancedObjectMeta::ReadMetaFromMap(HeaderMetaData, FInstancedObjectMeta::MD_PaddingT, 0);
		const int32 PaddingB = FInstancedObjectMeta::ReadMetaFromMap(HeaderMetaData, FInstancedObjectMeta::MD_PaddingB, 0);
		
		Box->AddSlot()
		   .AutoWidth()
		   .Padding(Padding + PaddingL, Padding + PaddingT, Padding + PaddingR, Padding + PaddingB)
		   .VAlign(VAlign_Center)
		[
			SNew(SBox)
			.MinDesiredWidth(MinWidth > 0 ? MinWidth : FOptionalSize())
			.MaxDesiredWidth(MaxWidth > 0 ? MaxWidth : FOptionalSize())
			[
				ValueWidget.ToSharedRef()
			]
		];						
	}

	if (Box->NumSlots() > 0)
	{
		Result = Box;
	}	
	
	return Result;
}


#undef LOCTEXT_NAMESPACE
