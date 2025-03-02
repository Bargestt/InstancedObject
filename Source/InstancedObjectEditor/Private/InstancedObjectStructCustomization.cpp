// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#include "InstancedObjectStructCustomization.h"

#include "ClassViewerFilter.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "InstancedObjectInterface.h"
#include "InstancedObjectStruct.h"
#include "InstancedObjectUtilityLibrary.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyRestriction.h"
#include "String/ParseTokens.h"
#include "Widgets/Text/SRichTextBlock.h"


#define LOCTEXT_NAMESPACE "FInstancedObjectEditorModule"

namespace
{
	FName MD_ShowOnlyInnerProperties = TEXT("ShowOnlyInnerProperties");
	FName MD_IndentSize = TEXT("IndentSize");
	FName MD_BaseClass = TEXT("BaseClass");	
	FName MD_MustImplement = TEXT("MustImplement");
	FName MD_InheritFilters = TEXT("InheritFilters");
	FName MD_HideChild = TEXT("Hidden");

	
	FName MD_Header = TEXT("Header");
	FName MD_MinWidth = TEXT("MinWidth");
	FName MD_MaxWidth = TEXT("MaxWidth");
	
	FName MD_Padding = TEXT("Padding");
	FName MD_PaddingL = TEXT("PaddingL");
	FName MD_PaddingR = TEXT("PaddingR");
	FName MD_PaddingT = TEXT("PaddingT");
	FName MD_PaddingB = TEXT("PaddingB");
	
	FName MD_HideName = TEXT("HideName");

	template<typename T>
	T ReadMetaFromMap(const TMap<FName, FString>& Map, const FName& Meta, T DefaultValue)
	{
		T OutValue = DefaultValue;
		if (const FString* ValuePtr = Map.Find(Meta))
		{
			LexFromString(OutValue, *ValuePtr);
		}
		return OutValue;
	}
}


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




void FInstancedObjectStructCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	StructHandle = PropertyHandle;
	ObjectHandle = PropertyHandle->GetChildHandle("Object", false);
	
	if (PropertyHandle->HasMetaData(MD_ShowOnlyInnerProperties))
	{
		return;
	}
	
	ObjectHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FInstancedObjectStructCustomization::UpdateTitle));
	ObjectHandle->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FInstancedObjectStructCustomization::UpdateTitle));	
	
	HeaderWidget = CreateHeaderWidget();
	
	HeaderRow.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(250.0f)
	.MaxDesiredWidth(4096.0f)
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().VAlign(VAlign_Center).MinWidth(75).MaxWidth(600).AutoWidth()
		[
			HeaderWidget.ToSharedRef()
		]
		+ SHorizontalBox::Slot().VAlign(VAlign_Center).AutoWidth()
		[
			SAssignNew(ObjectHeaderPanel, SBox)
		]
	];
}

void FInstancedObjectStructCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	if (ObjectHandle.IsValid())
	{
		TSharedPtr<IPropertyHandle> ObjectInstanceHandle = ObjectHandle->GetChildHandle(0);
		if(ObjectInstanceHandle.IsValid())
		{
			uint32 NumChildren;
			if (ObjectInstanceHandle->GetNumChildren(NumChildren) == FPropertyAccess::Success)
			{
				if (ObjectHeaderPanel.IsValid())
				{
					ObjectHeaderPanel->SetContent(CreateObjectHeaderWidget(ChildBuilder, CustomizationUtils));
				}				
				
				for (uint32 Index = 0; Index < NumChildren; Index++)
				{
					TSharedRef<IPropertyHandle> ChildHandle = ObjectInstanceHandle->GetChildHandle(Index).ToSharedRef();
					if (CanDisplayChild(ChildHandle) &&	!ChildHandle->GetBoolMetaData(MD_HideChild))
					{
						ChildBuilder.AddProperty(ChildHandle);
					}					
				}
			}
		}
	}
}

TSharedRef<SWidget> FInstancedObjectStructCustomization::CreateHeaderWidget()
{
	if (!ObjectHandle.IsValid())
	{
		return SNew(STextBlock)
			.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
			.Text(LOCTEXT("NoProperty", "Must have 'Object' property"));
	}

	UpdateTitle();


	auto CreateClassRestriction = [](const TSharedPtr<IPropertyHandle>& ObjectHandle, const FText& RestrictionName)
	{
		TSharedRef<FInstancedObjectClassFilter> ClassFilter = MakeShared<FInstancedObjectClassFilter>();		
		ClassFilter->bAllowAbstract = false;
		
		{
			const FString* ClassName = FindMetaData(ObjectHandle, MD_MustImplement);
			if (ClassName && !ClassName->IsEmpty())
			{
				ClassFilter->InterfaceThatMustBeImplemented = UClass::TryFindTypeSlow<UClass>(*ClassName);
			}
		}
		
		{
			const FString* ClassName = FindMetaData(ObjectHandle, MD_BaseClass);
			if (ClassName && !ClassName->IsEmpty())
			{
				ClassFilter->ClassPropertyMetaClass = UClass::TryFindTypeSlow<UClass>(*ClassName);
			}		
		}

		if (!ClassFilter->ClassPropertyMetaClass)
		{
			ClassFilter->ClassPropertyMetaClass = UObject::StaticClass();
		}

		if (const FString* Meta = FindMetaData(ObjectHandle, TEXT("AllowedClasses")))
		{
			ClassFilter->AllowedClassFilters = PropertyCustomizationHelpers::GetClassesFromMetadataString(*Meta);
		}
		if (const FString* Meta = FindMetaData(ObjectHandle, TEXT("DisallowedClasses")))
		{
			ClassFilter->DisallowedClassFilters = PropertyCustomizationHelpers::GetClassesFromMetadataString(*Meta);
		}

	
		TSharedRef<FPropertyRestriction> Restriction = MakeShared<FPropertyRestriction>(RestrictionName);
		Restriction->AddClassFilter(ClassFilter);
		return Restriction;
	};

	if (StructHandle->HasMetaData(MD_InheritFilters))
	{
		TSharedPtr<IPropertyHandle> SourceHandle = nullptr;

		for (TSharedPtr<IPropertyHandle> ParentHandle = StructHandle->GetParentHandle(); ParentHandle.IsValid(); ParentHandle = ParentHandle->GetParentHandle())
		{
			if (const FStructProperty* ParentProp = CastField<FStructProperty>(ParentHandle->GetProperty()))
			{
				if (ParentProp && ParentProp->Struct && ParentProp->Struct->IsChildOf<FInstancedObjectStructBase>())
				{
					SourceHandle = ParentHandle;
					break;
				}
			}
		}

		if (SourceHandle.IsValid())
		{
			TSharedPtr<IPropertyHandle> SourceHandle_Object = SourceHandle->GetChildHandle("Object", false);
			if (SourceHandle_Object.IsValid())
			{
				ObjectHandle->AddRestriction(CreateClassRestriction(SourceHandle_Object, LOCTEXT("InstancedObjectRestriction_Inherited", "Instanced Object Restriction (Inherited)")));
			}			
		}
	}	
	ObjectHandle->AddRestriction(CreateClassRestriction(ObjectHandle, LOCTEXT("InstancedObjectRestriction", "Instanced Object Restriction")));

	TSharedPtr<SWidget> ObjectEditor = ObjectHandle->CreatePropertyValueWidget();
	{	
		// Find all widgets in SPropertyEditorEditInline and override tooltips with better one
		TArray<TSharedPtr<SWidget>> TooltipHolders;
		TArray<TSharedPtr<SWidget>> WidgetsToCheck;
		WidgetsToCheck.Add(ObjectEditor);
		while (!WidgetsToCheck.IsEmpty())
		{
			TSharedPtr<SWidget> Widget = WidgetsToCheck.Pop();
			
			if (Widget->GetReadableLocation().StartsWith(TEXT("SPropertyEditorEditInline")))
			{
				TooltipHolders.Add(Widget);
			}
			
			FChildren* AllChildren = Widget->GetChildren();
			for (int32 Index = 0; Index < AllChildren->Num(); Index++)
			{
				TSharedPtr<SWidget> Child = AllChildren->GetChildAt(Index).ToSharedPtr();
				WidgetsToCheck.Add(Child);
			}
		}

		for (auto TooltipHolder : TooltipHolders)
		{
			TooltipHolder->SetToolTip(CreateTooltipWidget());
		}	
	}	
	ObjectEditor->SetVisibility(EVisibility::Hidden);

	
	int32 MinWidth = 0;
	if (const FString* Meta = FindMetaData(ObjectHandle, MD_MinWidth))
	{
		LexFromString(MinWidth, *Meta);
	}
	
	int32 MaxWidth = 0;
	if (const FString* Meta = FindMetaData(ObjectHandle, MD_MaxWidth))
	{
		LexFromString(MinWidth, *Meta);
	}
		
	return
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().MinWidth(MinWidth > 0 ? MinWidth : TAttribute<float>()).MaxWidth(MaxWidth > 0 ? MaxWidth : TAttribute<float>())//.Padding(5.0f, 0.0f)
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ContentPadding(FMargin(0))
			.ToolTip(CreateTooltipWidget())		
			.OnHovered_Lambda([this]()
			{
				if (Switcher.IsValid() && Switcher->GetChildren()->Num() == 2)
				{
					UpdateTooltip();
					Switcher->GetChildren()->GetChildAt(0)->SetVisibility(EVisibility::Hidden);
					Switcher->GetChildren()->GetChildAt(1)->SetVisibility(EVisibility::Visible);
				}
			})
			.OnUnhovered_Lambda([this]()
			{
				if (Switcher.IsValid() && Switcher->GetChildren()->Num() == 2)
				{
					UpdateTitle();
					Switcher->GetChildren()->GetChildAt(0)->SetVisibility(EVisibility::Visible);
					Switcher->GetChildren()->GetChildAt(1)->SetVisibility(EVisibility::Hidden);
				}
			})
			[				
				SAssignNew(Switcher, SOverlay)
				+ SOverlay::Slot().VAlign(VAlign_Center)
				[
					SNew(SRichTextBlock)
					.DecoratorStyleSet(&FAppStyle::Get())
					.TextStyle(FAppStyle::Get(), "NormalText")
					.Text(this, &FInstancedObjectStructCustomization::GetTitle)	
				]
				+ SOverlay::Slot().VAlign(VAlign_Center)
				[
					ObjectEditor.ToSharedRef()
				]					
			]
		];
}

TSharedRef<IToolTip> FInstancedObjectStructCustomization::CreateTooltipWidget()
{
	UpdateTooltip();
	return SNew(SToolTip)
	[
		SNew(SBox)
		.MinDesiredWidth(200.0f)
		[
			SNew(SRichTextBlock)
			.DecoratorStyleSet(&FAppStyle::Get())
			.TextStyle(FAppStyle::Get(), "NormalText")
			.Text(this, &FInstancedObjectStructCustomization::GetTooltip)
		]
	];
}

TSharedRef<SWidget> FInstancedObjectStructCustomization::CreateObjectHeaderWidget(IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{	
	if (ObjectHandle.IsValid())
	{
		TSharedPtr<IPropertyHandle> ObjectInstanceHandle = ObjectHandle->GetChildHandle(0);
		if(ObjectInstanceHandle.IsValid())
		{
			uint32 NumChildren;
			if (ObjectInstanceHandle->GetNumChildren(NumChildren) == FPropertyAccess::Success)
			{
				TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox);			
				
				for (uint32 Index = 0; Index < NumChildren; Index++)
				{
					TSharedRef<IPropertyHandle> ChildHandle = ObjectInstanceHandle->GetChildHandle(Index).ToSharedRef();
					if (CanDisplayChild(ChildHandle) &&	ChildHandle->HasMetaData(MD_Header))
					{
						TMap<FName, FString> HeaderMetaData;
						{
							const FString& MetaData = ChildHandle->GetMetaData(MD_Header);
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

						if (!HeaderMetaData.Contains(MD_HideName))
						{
							Box->AddSlot().AutoWidth().Padding(10, 0, 5, 0).VAlign(VAlign_Center)
							[
								ChildHandle->CreatePropertyNameWidget()
							];
						}

						const int32 MinWidth = ReadMetaFromMap(HeaderMetaData, MD_MinWidth, 0);						
						const int32 MaxWidth = ReadMetaFromMap(HeaderMetaData, MD_MaxWidth, 0);
 
						const int32 Padding = ReadMetaFromMap(HeaderMetaData, MD_Padding, 0);
						const int32 PaddingL = ReadMetaFromMap(HeaderMetaData, MD_PaddingL, 0);
						const int32 PaddingR = ReadMetaFromMap(HeaderMetaData, MD_PaddingR, 0);
						const int32 PaddingT = ReadMetaFromMap(HeaderMetaData, MD_PaddingT, 0);
						const int32 PaddingB = ReadMetaFromMap(HeaderMetaData, MD_PaddingB, 0);
						
						Box->AddSlot()
						.AutoWidth()
						.MinWidth(MinWidth)
						.MaxWidth(MaxWidth)
						.Padding(Padding + PaddingL, Padding + PaddingT, Padding + PaddingR, Padding + PaddingB)
						.VAlign(VAlign_Center)
						[
							ValueWidget.ToSharedRef()
						];
					}					
				}

				if (Box->NumSlots() > 0)
				{
					return Box;
				}
			}
		}
	}
	
	return SNullWidget::NullWidget;
}


const FString* FInstancedObjectStructCustomization::FindMetaData(const TSharedPtr<IPropertyHandle>& ObjectHandle, const FName& Key)
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

bool FInstancedObjectStructCustomization::CanDisplayChild(const TSharedRef<IPropertyHandle>& ChildHandle)
{
	const FProperty* Property = ChildHandle->GetProperty();
	return Property &&
			!Property->HasAnyPropertyFlags(CPF_Protected) &&
			!ChildHandle->GetBoolMetaData(TEXT("BlueprintProtected")) &&
			!ChildHandle->GetBoolMetaData(TEXT("BlueprintPrivate"));
}

void FInstancedObjectStructCustomization::UpdateTitle()
{
	Title = ReadTitle(false);
}

void FInstancedObjectStructCustomization::UpdateTooltip()
{
	Tooltip = ReadTitle(true);
}

FText FInstancedObjectStructCustomization::ReadTitle(bool bFull) const
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

	FString String = IInstancedObjectInterface::GetTitleSafe(ObjectValue, bFull);

	int32 IndentSize = 4;
	{
		UScriptStruct* StructClass = nullptr;
		if (const FStructProperty* Prop = CastField<FStructProperty>(StructHandle->GetProperty()))
		{
			StructClass = Prop->Struct;
		}

		if(StructClass)
		{
			FString IndentMeta = StructClass->GetMetaData(MD_IndentSize);
			if (!IndentMeta.IsEmpty() && IndentMeta.IsNumeric())
			{
				IndentSize = FCString::Atoi(*IndentMeta);
			}
		}
	}
	
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
	return FText::FromString(String);		
}

#undef LOCTEXT_NAMESPACE