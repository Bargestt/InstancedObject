#include "ObjectWithDetailsCustomization.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "InstancedObjectEditorUtils.h"
#include "ObjectWithDetails.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyRestriction.h"
#include "ThumbnailRendering/ThumbnailManager.h"

class FObjectDetailsBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis<FObjectDetailsBuilder>
{
public:
	FObjectDetailsBuilder(const TSharedRef<IPropertyHandle>& InPropertyHandle, const TSharedPtr<FObjectDetailsBuilder>& InParent = nullptr)
		: PropertyHandle(InPropertyHandle)
		, Parent(InParent)
	{
		ObjectHandle = PropertyHandle;
		if (PropertyHandle->GetPropertyClass() && PropertyHandle->GetPropertyClass()->IsChildOf(FStructProperty::StaticClass()))
		{
			ObjectHandle = PropertyHandle->GetChildHandle("Object", false);
		}
	}

	FObjectDetailsBuilder(UObject* Object, FProperty* Property, const TSharedPtr<FObjectDetailsBuilder>& InParent)		
	{
		
	}
	
	virtual ~FObjectDetailsBuilder() override
	{
			
	}

	//~ Begin IDetailCustomNodeBuilder interface
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override
	{
		OnRegenerateChildren = InOnRegenerateChildren;
	};
	
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& HeaderRow) override
	{
		FSimpleDelegate RequestRebuild = FSimpleDelegate::CreateLambda([WeakPropertyHandle = PropertyHandle.ToWeakPtr()]()
		{
			if (TSharedPtr<IPropertyHandle> Handle = WeakPropertyHandle.Pin())
			{
				Handle->RequestRebuildChildren();
			}			
		});	
		ObjectHandle->SetOnPropertyValueChanged(RequestRebuild);
		ObjectHandle->SetOnPropertyResetToDefault(RequestRebuild);
		
		if (ObjectHandle->HasMetaData(TEXT("ShowOnlyInnerProperties")))
		{
			return;
		}

		TSharedRef<SWidget> EditorWidget = SNullWidget::NullWidget;

		FStructProperty* StructProperty = CastField<FStructProperty>(PropertyHandle->GetProperty());
		if (StructProperty &&
			StructProperty->Struct &&
			(StructProperty->Struct->IsChildOf(FObjectWithDetails::StaticStruct()) || StructProperty->Struct->IsChildOf(FSoftObjectWithDetails::StaticStruct()))
			)
		{
			
			const UClass* ClassPropertyMetaClass = nullptr;		
			{
				const FString* ClassName = FInstancedObjectEditorUtils::FindMetaData(ObjectHandle, FInstancedObjectMeta::MD_BaseClass);
				if (!ClassName)
				{
					ClassName = FInstancedObjectEditorUtils::FindMetaData(ObjectHandle, TEXT("MetaClass"));
				}
				
				if (ClassName && !ClassName->IsEmpty())
				{
					ClassPropertyMetaClass = UClass::TryFindTypeSlow<UClass>(*ClassName);
				}
			}			
			if (!ClassPropertyMetaClass)
			{				
				ClassPropertyMetaClass = UObject::StaticClass();
			}

			bool bDisplayThumbnail = true;
            if(const FString* DisplayThumbnailString = FInstancedObjectEditorUtils::FindMetaData(ObjectHandle, TEXT("DisplayThumbnail")))
            {
            	bDisplayThumbnail = *DisplayThumbnailString == TEXT("true");
            }
			
			EditorWidget = SNew(SObjectPropertyEntryBox)
			.AllowedClass(ClassPropertyMetaClass)
			.DisplayThumbnail(bDisplayThumbnail)
			.ThumbnailPool(UThumbnailManager::Get().GetSharedThumbnailPool())
			.PropertyHandle(ObjectHandle);

			if (ObjectHandle->DiffersFromDefault())
			{				
				HeaderRow.ResetToDefaultContent()
				[
					PropertyCustomizationHelpers::MakeResetButton(FSimpleDelegate::CreateLambda([WeakHandle = ObjectHandle.ToWeakPtr()]()
					{
						TSharedPtr<IPropertyHandle> Handle = WeakHandle.Pin();
						if (Handle)
						{
							Handle->ResetToDefault();
						}
					}))
				];
			}			
		}
		else
		{		
			EditorWidget = ObjectHandle->CreatePropertyValueWidget(true);
		}

		FUIAction CopyAction, PasteAction;
		ObjectHandle->CreateDefaultPropertyCopyPasteActions(CopyAction, PasteAction);
		
		HeaderRow
		.CopyAction(CopyAction)
		.PasteAction(PasteAction)
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(250)
		.MaxDesiredWidth(350)
		[
			EditorWidget
		];
	}
	
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildBuilder) override
	{
		if (!ObjectHandle.IsValid())
		{
			return;
		}
		
		FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(ObjectHandle->GetProperty());
		if (!ObjectProperty)
		{
			return;
		}
	

		if (ObjectProperty->HasAllPropertyFlags(CPF_InstancedReference))
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
			return;
		}

	
		UObject* Value = nullptr;
		if (ObjectHandle->GetValue(Value) != FPropertyAccess::Success || Value == nullptr)
		{
			return;
		}


		FText LoopMessage = NSLOCTEXT("FInstancedObjectEditor", "NoLoops", "Loops are not allowed");
		for (TSharedPtr<IPropertyHandle> ParentHandle = ObjectHandle->GetParentHandle(); ParentHandle.IsValid(); ParentHandle = ParentHandle->GetParentHandle())
		{			
			UObject* ParentValue = nullptr;
			if (ParentHandle->GetValue(ParentValue) == FPropertyAccess::Success && ParentValue == Value)
			{
				ChildBuilder.AddCustomRow(LoopMessage).ValueContent()
				[
					SNew(STextBlock)
					.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
					.Text(LoopMessage)
				];
				return;
			}
		}

		for (TSharedPtr<FObjectDetailsBuilder> ParentBuilder = Parent.Pin(); ParentBuilder.IsValid(); ParentBuilder = ParentBuilder->Parent.Pin())
		{
			UObject* ParentValue = nullptr;
			if (ParentBuilder->ObjectHandle->GetValue(ParentValue) == FPropertyAccess::Success && ParentValue == Value)
			{			
				ChildBuilder.AddCustomRow(LoopMessage).ValueContent()
				[
					SNew(STextBlock)
					.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
					.Text(LoopMessage)
				];
				return;
			}
		}
		

		TArray<FString> Categories;
		PropertyHandle->GetMetaData(TEXT("Categories")).ParseIntoArray(Categories, TEXT(","));
	
		FString Padding = PropertyHandle->GetMetaData(TEXT("Padding"));
		if (!Padding.IsEmpty())
		{
			int32 PaddingSize = 0;
			LexFromString(PaddingSize, *Padding);
			Padding = FString::ChrN(FMath::Max(1, PaddingSize), ' ');
		}

		for (TFieldIterator<FProperty> PropIt(Value->GetClass()); PropIt; ++PropIt)
		{
			FProperty* Property = *PropIt;
			if (Property &&
				!Property->HasAnyPropertyFlags(CPF_Protected | CPF_Deprecated) &&
				Property->HasAllPropertyFlags(CPF_Edit) &&
				!Property->HasMetaData(TEXT("InlineEditConditionToggle")) &&
				(Categories.IsEmpty() || Categories.Contains(Property->GetMetaData(TEXT("Category")))))
			{
				bool bExpandChildren = false;
				if (Property->HasMetaData("ObjectWithDetails"))
				{
					bExpandChildren = true;
				}
				else
				{
					FStructProperty* StructProperty = CastField<FStructProperty>(Property);
					if (StructProperty &&
						StructProperty->Struct &&
						(StructProperty->Struct->IsChildOf(FObjectWithDetails::StaticStruct()) ||
							StructProperty->Struct->IsChildOf(FSoftObjectWithDetails::StaticStruct()))
						)
					{
						bExpandChildren = true;
					}
				}				

				if (IDetailPropertyRow* Row = ChildBuilder.AddExternalObjectProperty({Value}, Property->GetFName()))
				{
					if (!Padding.IsEmpty())
					{						
						const FText BaseName = Property->GetDisplayNameText();
						Row->DisplayName(FText::Format(INVTEXT("{0}{1}"), FText::FromString(Padding), BaseName));
					}
					
					if (bExpandChildren)
					{					
						Row->Visibility(EVisibility::Collapsed);
						Row->CustomWidget(false);
						TSharedPtr<IPropertyHandle> Handle = Row->GetPropertyHandle();
						if (Handle.IsValid())
						{
							ChildBuilder.AddCustomBuilder(MakeShared<FObjectDetailsBuilder>(Handle.ToSharedRef(), AsShared()));
						}						
					}								
				}
			}
		}
	}
	
	virtual bool InitiallyCollapsed() const override { return false; }
	virtual FName GetName() const override
	{
		static const FName Name("FObjectDetailsBuilder");
		return Name;
	}
	//~ End IDetailCustomNodeBuilder interface
	
	TSharedPtr<IPropertyHandle> GetObjectHandle() const { return ObjectHandle; }
private:
	FSimpleDelegate OnRegenerateChildren;
	
	TSharedPtr<IPropertyHandle> PropertyHandle;
	TSharedPtr<IPropertyHandle> ObjectHandle;

	TWeakPtr<FObjectDetailsBuilder> Parent;
};

void FObjectWithDetailsCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	Builder = MakeShared<FObjectDetailsBuilder>(PropertyHandle);	
	Builder->GenerateHeaderRowContent(HeaderRow);
}

void FObjectWithDetailsCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	Builder->GenerateChildContent(ChildBuilder);
}
