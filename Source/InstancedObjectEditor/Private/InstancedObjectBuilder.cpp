#include "InstancedObjectBuilder.h"

#include "DetailWidgetRow.h"

#include "IDetailChildrenBuilder.h"
#include "InstancedObjectEditorUtils.h"
#include "PropertyEditorClipboard.h"
#include "SInstancedObjectHeader.h"

FInstancedObjectBuilder::FInstancedObjectBuilder(const TSharedRef<IPropertyHandle>& InObjectHandle, const TSharedPtr<IPropertyHandle>& InStructHandle)
{
	ObjectHandle = InObjectHandle;
	StructHandle = InStructHandle;
	if (!StructHandle.IsValid())
	{
		TSharedPtr<IPropertyHandle> ParentHandle = ObjectHandle->GetParentHandle();
		if (ParentHandle.IsValid() && ParentHandle->GetPropertyClass() && ParentHandle->GetPropertyClass()->IsChildOf(FStructProperty::StaticClass()))
		{
			StructHandle = ParentHandle;
		}
	}
}

FInstancedObjectBuilder::~FInstancedObjectBuilder()
{
	
}

void FInstancedObjectBuilder::GenerateHeaderRowContent(FDetailWidgetRow& HeaderRow)
{
	if (StructHandle.IsValid() && StructHandle->HasMetaData(FInstancedObjectMeta::MD_ShowOnlyInnerProperties))
	{
		return;
	}

	if (ObjectHandle->HasMetaData(FInstancedObjectMeta::MD_ShowOnlyInnerProperties))
	{
		return;
	}

	FInstancedObjectEditorUtils::CreateClassRestrictions(ObjectHandle.ToSharedRef(), nullptr);
	FInstancedObjectEditorUtils::CreateInheritedClassRestrictions(ObjectHandle.ToSharedRef(), StructHandle);

	FUIAction CopyAction, PasteAction;
	ObjectHandle->CreateDefaultPropertyCopyPasteActions(CopyAction, PasteAction);	
	PasteAction.ExecuteAction.BindLambda([Prop = ObjectHandle]()
	{
		FString ClipboardContent;
		FPropertyEditorClipboard::ClipboardPaste(ClipboardContent);
		if (Prop.IsValid())
		{
			Prop->SetValueFromFormattedString(ClipboardContent, EPropertyValueSetFlags::InstanceObjects);
		}		
	});
	
	HeaderRow
	.CopyAction(CopyAction)
	.PasteAction(PasteAction)
	.NameContent()
	[
		StructHandle.IsValid() ? StructHandle->CreatePropertyNameWidget() : ObjectHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(250.0f)
	.MaxDesiredWidth(4096.0f)
	.HAlign(HAlign_Fill)
	[
		SAssignNew(HeaderWidget, SInstancedObjectHeader, ObjectHandle)
		.bAlwaysShowPropertyButtons(AlwaysShowPropertyButtons.Get(true))
		.bDisplayDefaultPropertyButtons(DisplayDefaultPropertyButtons.Get(true))
	];

	OnHeaderRowGenerated.ExecuteIfBound(HeaderRow);
}

void FInstancedObjectBuilder::GenerateChildContent(IDetailChildrenBuilder& ChildBuilder)
{
	if (!ObjectHandle.IsValid())
	{
		return;
	}

	TSharedPtr<IPropertyHandle> ObjectInstanceHandle = ObjectHandle->GetChildHandle(0);
	if(!ObjectInstanceHandle.IsValid())
	{
		return;
	}

	uint32 NumChildren;
	if (ObjectInstanceHandle->GetNumChildren(NumChildren) != FPropertyAccess::Success || NumChildren == 0)
	{
		return;
	}

	HeaderWidget->GetHeaderExtensionPanel()->SetContent(FInstancedObjectEditorUtils::CreateHeader_ChildrenEditors(ObjectHandle, ChildBuilder));		
	for (uint32 Index = 0; Index < NumChildren; Index++)
	{
		TSharedRef<IPropertyHandle> ChildHandle = ObjectInstanceHandle->GetChildHandle(Index).ToSharedRef();
		if (FInstancedObjectEditorUtils::CanDisplayChild(ChildHandle))
		{
			ChildBuilder.AddProperty(ChildHandle);
		}					
	}
}
