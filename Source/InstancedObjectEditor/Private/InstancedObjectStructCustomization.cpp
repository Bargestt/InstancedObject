// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#include "InstancedObjectStructCustomization.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "InstancedObjectInterface.h"
#include "PropertyCustomizationHelpers.h"
#include "SInstancedObjectHeader.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Text/SRichTextBlock.h"


#define LOCTEXT_NAMESPACE "FInstancedObjectEditorModule"



void FInstancedObjectStructCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	ObjectHandle = PropertyHandle->GetChildHandle("Object", false);	
	ObjectHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FInstancedObjectStructCustomization::UpdateTitle));
	ObjectHandle->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FInstancedObjectStructCustomization::UpdateTitle));

	TSharedPtr<SWidget> HeaderWidget;
	if (ObjectHandle.IsValid())
	{
		UpdateTitle();

		UScriptStruct* StructClass = nullptr;
		if (const FStructProperty* Prop = CastField<FStructProperty>(PropertyHandle->GetProperty()))
		{
			StructClass = Prop->Struct;
		}
		if (!StructClass)
		{
			return;
		}

		FString IndentMeta = StructClass->GetMetaData("IndentSize");
		if (!IndentMeta.IsEmpty() && IndentMeta.IsNumeric())
		{
			IndentSize = FCString::Atoi(*IndentMeta);
		}

		bool bShowAdvancedWidget = StructClass->HasMetaData(TEXT("AdvancedWidget"));
		
		
		TSharedPtr<IPropertyHandle> Container = PropertyHandle->GetParentHandle();
		bool bIsInContainer = Container.IsValid() && Container->GetProperty() &&
				(Container->GetProperty()->IsA<FArrayProperty>() ||
				Container->GetProperty()->IsA<FSetProperty>() ||
				Container->GetProperty()->IsA<FMapProperty>());

		auto ExtractMetaData = [&](const FName Key)-> FString
		{
			FString MetaData;
			if (bIsInContainer)
			{
				MetaData = Container->GetMetaData(Key);
				if (!MetaData.IsEmpty())
				{
					return MetaData;
				}
			}
		
			MetaData = PropertyHandle->GetMetaData(Key);
			if (!MetaData.IsEmpty())
			{
				return MetaData;
			}
		
			MetaData = ObjectHandle->GetMetaData(Key);
			if (!MetaData.IsEmpty())
			{
				return MetaData;
			}	
			
			MetaData = StructClass->GetMetaData(Key);
			if (!MetaData.IsEmpty())
			{
				return MetaData;
			}		

			return MetaData;
		};	
	
		UClass* BaseClass;
		{
			FString ClassName = ExtractMetaData(TEXT("BaseClass"));		
			BaseClass = ClassName.IsEmpty() ? nullptr : UClass::TryFindTypeSlow<UClass>(ClassName);
		}

		UClass* InterfaceClass;
		{
			FString ClassName = ExtractMetaData(TEXT("MustImplement"));		
			InterfaceClass = ClassName.IsEmpty() ? nullptr : UClass::TryFindTypeSlow<UClass>(ClassName);
		}

		bool bShowNoneOption =	!ObjectHandle->GetProperty()->HasAllPropertyFlags(CPF_NoClear) &&
								!PropertyHandle->GetProperty()->HasAllPropertyFlags(CPF_NoClear) &&	
								!(bIsInContainer && Container->GetProperty()->HasAllPropertyFlags(CPF_NoClear));


		TSharedRef<SWidget> ObjectEditor =
			SNew(SInstancedObjectHeader, ObjectHandle)
			.MetaClass(BaseClass)
			.RequiredInterface(InterfaceClass)
			.ShowDisplayNames(true)
			.ShowNoneOption(bShowNoneOption);

		if (!bShowAdvancedWidget)
		{
			HeaderWidget = ObjectEditor;
		}
		else
		{
			HeaderWidget =
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()//.Padding(5.0f, 0.0f)
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.ContentPadding(FMargin(0))
					.ToolTip(CreateTooltipWidget())		
					.OnHovered_Lambda([this]()
					{
						if (Switcher.IsValid())
						{
							Switcher->SetActiveWidgetIndex(1);
						}
					})
					.OnUnhovered_Lambda([this]()
					{
						if (Switcher.IsValid())
						{
							UpdateTitle();
							Switcher->SetActiveWidgetIndex(0);
						}
					})
					[				
						SAssignNew(Switcher, SWidgetSwitcher)
						+ SWidgetSwitcher::Slot()
						[
							SNew(SRichTextBlock)
							.DecoratorStyleSet(&FAppStyle::Get())
							.TextStyle(FAppStyle::Get(), "NormalText")
							.Text(Title)	
						]
						+ SWidgetSwitcher::Slot()
						[
							ObjectEditor
						]					
					]
				];
		}
	}
	else
	{
		HeaderWidget = SNew(STextBlock)
			.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
			.Text(LOCTEXT("NoProperty", "Must have 'Object' property"));
	}	
	
	HeaderRow.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(250.0f)
	.MaxDesiredWidth(600.0f)
	[
		HeaderWidget.ToSharedRef()
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
				for (uint32 Index = 0; Index < NumChildren; Index++)
				{
					ChildBuilder.AddProperty(ObjectInstanceHandle->GetChildHandle(Index).ToSharedRef());
				}
			}
		}
	}
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
			.Text(Tooltip)
		]
	];
}

void FInstancedObjectStructCustomization::UpdateTitle()
{
	Title = ReadTitle(false);
}

void FInstancedObjectStructCustomization::UpdateTooltip()
{
	Tooltip = ReadTitle(true);
}

FText FInstancedObjectStructCustomization::ReadTitle(bool bFull)
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
	
	bool bNoIndent = String.RemoveFromStart(TEXT("<NoIndent>"));	
	if (bFull && IndentSize >= 0 && !bNoIndent)
	{
		FString IndentedString;
		int32 Error = ParseIndent(String, IndentedString, IndentSize);
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

int32 FInstancedObjectStructCustomization::ParseIndent(FString InString, FString& OutString, int32 IndentSize)
{
	int32 Depth = 0;
	bool bDepthChanged = false;
	for (int32 Index = 0; Index < InString.Len(); Index++)
	{
		if (InString[Index] == '(')
		{
			Depth++;
			bDepthChanged = true;
		}
		else if (InString[Index] == ')')
		{
			Depth--;
			bDepthChanged = true;
		}
		else
		{
			if (bDepthChanged)
			{
				OutString += TEXT("\n") + FString::ChrN(FMath::Max(0, Depth * IndentSize), ' ');
			}
			OutString += InString[Index];
			bDepthChanged = false;
		}
	}
	OutString.RemoveFromStart(TEXT("\n"));
	return Depth;
}

#undef LOCTEXT_NAMESPACE