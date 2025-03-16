


#include "InstancedObjectArrayCustomization.h"
#include <DetailWidgetRow.h>

#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailDragDropHandler.h"
#include "InstancedObjectBuilder.h"
#include "InstancedObjectEditorUtils.h"
#include "InstancedObjectUtilityLibrary.h"
#include "IPropertyRowGenerator.h"
#include "IPropertyUtilities.h"
#include "ISinglePropertyView.h"
#include "IStructureDataProvider.h"
#include "IStructureDetailsView.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyRestriction.h"
#include "SInstancedObjectHeader.h"
#include "DragAndDrop/DecoratedDragDropOp.h"
#include "Widgets/Text/SRichTextBlock.h"

#define LOCTEXT_NAMESPACE "FInstancedObjectEditor"

class FInstancedObjectArrayStructDragDropOp : public FDecoratedDragDropOp
{
public:
	DRAG_DROP_OPERATOR_TYPE(FInstancedObjectArrayStructDragDropOp, FDecoratedDragDropOp)

	FInstancedObjectArrayStructDragDropOp(const TWeakPtr<IPropertyHandle>& InArrayItemHandleWeak)
	{
		ArrayItemHandleWeak = InArrayItemHandleWeak;
		MouseCursor = EMouseCursor::GrabHandClosed;
	}

	void Init()
	{
		SetValidDrop(false);
		SetupDefaults();
		Construct();
	}

	void SetValidDrop(bool bIsValidDrop)
	{
		if (bIsValidDrop)
		{
			CurrentHoverText = LOCTEXT("PlaceRowHere", "Place Row Here");
			CurrentIconBrush = FAppStyle::GetBrush("Graph.ConnectorFeedback.OK");
		}
		else
		{
			CurrentHoverText = LOCTEXT("CannotPlaceRowHere", "Cannot Place Row Here");
			CurrentIconBrush = FAppStyle::GetBrush("Graph.ConnectorFeedback.Error");
		}
	}

	TWeakPtr<IPropertyHandle> ArrayItemHandleWeak;
};

class FInstancedObjectArrayStructDragDropHandler : public IDetailDragDropHandler
{
public:
	explicit FInstancedObjectArrayStructDragDropHandler(const TSharedRef<IPropertyHandle>& InArrayItemHandle, const TSharedPtr<SWidget>& InRowWidget, const TWeakPtr<IPropertyUtilities>& InPropertyUtilities)
		: ArrayItemHandleWeak(InArrayItemHandle)		
		, PropertyUtilitiesWeak(InPropertyUtilities)
		, RowWidgetWeak(InRowWidget)
	{
		
	}

	//~ Begin IDetailDragDropHandler
	virtual TSharedPtr<FDragDropOperation> CreateDragDropOperation() const override
	{
		TSharedRef<FInstancedObjectArrayStructDragDropOp> Op = MakeShared<FInstancedObjectArrayStructDragDropOp>(ArrayItemHandleWeak);
		Op->Init();
		return Op;
	}
	virtual TOptional<EItemDropZone> CanAcceptDrop(const FDragDropEvent& InDragDropEvent, EItemDropZone InDropZone) const override
	{
		TSharedPtr<FInstancedObjectArrayStructDragDropOp> ArrayItemDragDropOp = InDragDropEvent.GetOperationAs<FInstancedObjectArrayStructDragDropOp>();
		TSharedPtr<IPropertyHandle> ArrayItemHandle = ArrayItemHandleWeak.Pin();;
		TSharedPtr<SWidget> RowWidget = RowWidgetWeak.Pin();

		if (!RowWidget.IsValid() || !ArrayItemHandle.IsValid() || !ArrayItemDragDropOp.IsValid())
		{
			return TOptional<EItemDropZone>();
		}

		TSharedPtr<IPropertyHandle> DraggedArrayItemHandle = ArrayItemDragDropOp->ArrayItemHandleWeak.Pin();
		if (!DraggedArrayItemHandle.IsValid())
		{
			return TOptional<EItemDropZone>();
		}

		const FGeometry& Geometry = RowWidget->GetTickSpaceGeometry();
		const float LocalPointerY = Geometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition()).Y;

		const EItemDropZone OverrideDropZone = LocalPointerY < Geometry.GetLocalSize().Y * 0.5f
			? EItemDropZone::AboveItem
			: EItemDropZone::BelowItem;

		if (!IsValidDrop(ArrayItemHandle, DraggedArrayItemHandle, OverrideDropZone))
		{
			ArrayItemDragDropOp->SetValidDrop(false);
			return TOptional<EItemDropZone>();
		}

		ArrayItemDragDropOp->SetValidDrop(true);
		return OverrideDropZone;
	}
	virtual bool AcceptDrop(const FDragDropEvent& InDragDropEvent, EItemDropZone InDropZone) const override
	{
		TSharedPtr<FInstancedObjectArrayStructDragDropOp> ArrayItemDragDropOp = InDragDropEvent.GetOperationAs<FInstancedObjectArrayStructDragDropOp>();
		TSharedPtr<IPropertyHandle> ArrayItemHandle = ArrayItemHandleWeak.Pin();;
		TSharedPtr<SWidget> RowWidget = RowWidgetWeak.Pin();

		if (!RowWidget.IsValid() || !ArrayItemHandle.IsValid() || !ArrayItemDragDropOp.IsValid())
		{
			return false;
		}

		TSharedPtr<IPropertyHandle> DraggedArrayItemHandle = ArrayItemDragDropOp->ArrayItemHandleWeak.Pin();
		TSharedPtr<IPropertyHandle> ParentHandle = ArrayItemHandle->GetParentHandle();

		if (!DraggedArrayItemHandle.IsValid() || !ParentHandle.IsValid())
		{
			return false;
		}

		TSharedPtr<IPropertyHandleArray> ParentArrayHandle = ParentHandle->AsArray();

		if (!ParentArrayHandle.IsValid())
		{
			return false;
		}

		const int32 SourceIndex = DraggedArrayItemHandle->GetArrayIndex();
		const int32 TargetIndex = CalculateTargetIndex(SourceIndex, ArrayItemHandle->GetArrayIndex(), InDropZone);

		FScopedTransaction Transaction(LOCTEXT("MoveRow", "Move Row"));

		ParentHandle->NotifyPreChange();
		ParentArrayHandle->MoveElementTo(SourceIndex, TargetIndex);
		ParentHandle->NotifyPostChange(EPropertyChangeType::ArrayMove);

		// IPropertyHandle::NotifyFinishedChangingProperties is not called as it hard codes the change type to ValueSet, when this is an Array Move
		// Instead, PropertyUtilities function is called directly with the correct change type
		if (TSharedPtr<IPropertyUtilities> PropertyUtilities = PropertyUtilitiesWeak.Pin())
		{
			FPropertyChangedEvent MoveEvent(ParentHandle->GetProperty(), EPropertyChangeType::ArrayMove);
			PropertyUtilities->NotifyFinishedChangingProperties(MoveEvent);
		}

		return true;
	}
	//~ End IDetailDragDropHandler	

	static int32 CalculateTargetIndex(int32 InSourceIndex, int32 InTargetIndex, EItemDropZone DropZone)
	{
		if (DropZone == EItemDropZone::BelowItem)
		{
			// If the drop zone is below, then move it to the next item's index
			InTargetIndex++;
		}

		if (InSourceIndex < InTargetIndex)
		{
			// If the item is moved down the list, then all the other elements below it are shifted up one
			InTargetIndex--;
		}

		return ensure(InTargetIndex >= 0) ? InTargetIndex : 0;
	}

	static bool IsValidDrop(const TSharedPtr<IPropertyHandle>& InTargetItemHandle, const TSharedPtr<IPropertyHandle>& InDraggedItemHandle, EItemDropZone InDropZone)
	{
		// Can't drop onto another array item; need to drop above or below. Likewise, cannot drop above/below itself (redundant operation)
		if (InDropZone == EItemDropZone::OntoItem || InTargetItemHandle == InDraggedItemHandle)
		{
			return false;
		}

		const int32 SourceIndex = InDraggedItemHandle->GetIndexInArray();
		const int32 TargetIndex = CalculateTargetIndex(SourceIndex, InTargetItemHandle->GetArrayIndex(), InDropZone);

		if (SourceIndex != TargetIndex)
		{
			TSharedPtr<IPropertyHandle> TargetParentHandle  = InTargetItemHandle->GetParentHandle();
			TSharedPtr<IPropertyHandle> DraggedParentHandle = InDraggedItemHandle->GetParentHandle();

			// Ensure that these two property handles belong to the same array parent property
			return TargetParentHandle.IsValid() && DraggedParentHandle.IsValid()
				&& TargetParentHandle->IsSamePropertyNode(DraggedParentHandle)
				&& TargetParentHandle->AsArray().IsValid();
		}

		return false;
	}

private:
	TWeakPtr<IPropertyHandle> ArrayItemHandleWeak;
	TWeakPtr<IPropertyUtilities> PropertyUtilitiesWeak;

public:
	TWeakPtr<SWidget> RowWidgetWeak;
};


void FInstancedObjectArrayStructCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	ArrayHandle = PropertyHandle->GetChildHandle("Objects", false);
	check(ArrayHandle->AsArray().IsValid());
	
	if (PropertyHandle->HasMetaData(FInstancedObjectMeta::MD_ShowOnlyInnerProperties))
	{
		return;
	}

	HeaderRow.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()		
		[
			SNew(SBox)
			.MinDesiredWidth(120)
			.MaxDesiredWidth(500)
			.Padding( 0.0f, 0.0f, 5.0f, 0.0f )
			[
				ArrayHandle->CreatePropertyValueWidget(false)
			]	
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			ArrayHandle->CreateDefaultPropertyButtonWidgets()
		]
	];
}

void FInstancedObjectArrayStructCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FSimpleDelegate RequestRebuild = FSimpleDelegate::CreateLambda([PropertyHandle]()
	{
		PropertyHandle->RequestRebuildChildren();
	});	
	ArrayHandle->SetOnPropertyValueChanged(RequestRebuild);
	ArrayHandle->SetOnPropertyResetToDefault(RequestRebuild);
	
	uint32 ChildrenNum = 0;
    ArrayHandle->GetNumChildren(ChildrenNum);
	if(ChildrenNum == 0)
	{
		return;
	}
	
    for(uint32 Index = 0; Index < ChildrenNum; Index++)
    {
    	if (TSharedPtr<IPropertyHandle> ChildProperty = ArrayHandle->GetChildHandle(Index))
    	{
    		FInstancedObjectEditorUtils::CreateClassRestrictions(ChildProperty.ToSharedRef(), PropertyHandle);
    		FInstancedObjectEditorUtils::CreateInheritedClassRestrictions(ChildProperty.ToSharedRef(), PropertyHandle);
    	}
    }
	
	const FString* AdvancedWidget = FInstancedObjectEditorUtils::FindMetaData(ArrayHandle, FInstancedObjectMeta::MD_AdvancedWidget);
	if (!AdvancedWidget)
	{
		for(uint32 Index = 0; Index < ChildrenNum; Index++)
		{
			if (TSharedPtr<IPropertyHandle> ChildProperty = ArrayHandle->GetChildHandle(Index))
			{								
				IDetailPropertyRow& Row = ChildBuilder.AddProperty(ChildProperty.ToSharedRef());
				
				TSharedPtr<SWidget> NameWidget, ValueWidget;
				Row.GetDefaultWidgets(NameWidget, ValueWidget, false);
				
				Row.CustomWidget(true)
				.NameContent()
				[
					NameWidget.ToSharedRef()
				]
				.ValueContent()
				[
					SNew(SInstancedObjectHeader, ChildProperty)
					.bDisplayDefaultPropertyButtons(false)
					.bAlwaysShowPropertyButtons(false)
				];
			}
		}		
		return;
	}
	

	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");	

	SelectionDetails = PropertyModule.FindDetailView(FName(PropertyHandle->GetPropertyPath()));
	if (!SelectionDetails.IsValid())
	{
		FDetailsViewArgs Args;
		{
			Args.ViewIdentifier = FName(PropertyHandle->GetPropertyPath());
			Args.bAllowSearch = false;
			Args.bShowOptions = false;
			Args.bShowObjectLabel = false;
			Args.bShowScrollBar = true;
			Args.bHideSelectionTip = true;
			Args.NameAreaSettings = FDetailsViewArgs::HideNameArea;
		}	
		SelectionDetails = PropertyModule.CreateDetailView(Args);
	}

	int32 MinHeight = 0;
	if (const FString* Meta = FInstancedObjectEditorUtils::FindMetaData(ArrayHandle, FInstancedObjectMeta::MD_MinHeight))
	{
		LexFromString(MinHeight, **Meta);
	}
	
	int32 MaxHeight = 0;
	if (const FString* Meta = FInstancedObjectEditorUtils::FindMetaData(ArrayHandle, FInstancedObjectMeta::MD_MaxHeight))
	{
		LexFromString(MaxHeight, **Meta);
	}

	ChildBuilder.AddCustomRow(ArrayHandle->GetToolTipText())
	.NameContent()
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Fill)
	[
		SNew(SBox)
		.MinDesiredHeight(MinHeight > 0 ? MinHeight : FOptionalSize())
		.MaxDesiredHeight(MaxHeight > 0 ? MaxHeight : FOptionalSize())
		.Padding(-22, 0, 0, 0)
		[
			SAssignNew(List, SListView<TSharedPtr<FListHandleEntry>>)
			.ListViewStyle(&FAppStyle::GetWidgetStyle<FTableViewStyle>("PropertyTable.InViewport.ListView"))
			.ListItemsSource(&ListEntries)
			.SelectionMode(ESelectionMode::Multi)
			.ClearSelectionOnClick(true)
			.OnGenerateRow(this, &FInstancedObjectArrayStructCustomization::OnGenerateWidgetForList)
			.OnSelectionChanged(this, &FInstancedObjectArrayStructCustomization::OnSelectionChanged)
		]
	]
	.ValueContent()
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Fill)
		[
			SNew(SBox)
			.Padding(-12, 0, 0, 0)
			.MinDesiredHeight(MinHeight > 0 ? MinHeight : FOptionalSize())
			.MaxDesiredHeight(MaxHeight > 0 ? MaxHeight : FOptionalSize())
			[
				SelectionDetails.ToSharedRef()
			]
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
			.Text(LOCTEXT("SelectionHint", "Select objects to display details"))
			.Visibility_Lambda([WeakSelectionDetails = SelectionDetails.ToWeakPtr()]()
			{
				TSharedPtr<IDetailsView> DetailsView = WeakSelectionDetails.Pin();
				return (DetailsView.IsValid() && DetailsView->GetSelectedObjects().ContainsByPredicate([](const TWeakObjectPtr<>& Entry) { return Entry.IsValid(); }) ) ? EVisibility::Collapsed : EVisibility::Visible;
			})
		]
	];

	const TSharedPtr<IPropertyHandleArray> AsArray = ArrayHandle->AsArray();
	if (AsArray.IsValid())
	{
		TArray<TWeakObjectPtr<UObject>> OldSelectedObjects = SelectionDetails->GetSelectedObjects();
		TArray<TSharedPtr<FListHandleEntry>> NewSelection;
		
		uint32 NumItems = 0;
		ListEntries.Empty(NumItems);
		if (AsArray->GetNumElements(NumItems) == FPropertyAccess::Success)
		{
			for (uint32 Index = 0; Index < NumItems; Index++)
			{
				TSharedRef<IPropertyHandle> ElementHandle = AsArray->GetElement(Index);
				
				FListHandleEntry* Entry = ListEntries.Add_GetRef(MakeShared<FListHandleEntry>()).Get();
				Entry->Handle = ElementHandle;
				Entry->DragDropHandler = MakeShared<FInstancedObjectArrayStructDragDropHandler>(ElementHandle, nullptr, CustomizationUtils.GetPropertyUtilities());


				UObject* ObjectValue;
				if (ElementHandle->GetValue(ObjectValue) == FPropertyAccess::Success && ObjectValue && OldSelectedObjects.Contains(ObjectValue))
				{
					NewSelection.Add(ListEntries.Last());
				}	
			}
		}		
		List->ClearSelection();
		List->SetItemSelection(NewSelection, true, ESelectInfo::Direct);
		if (NewSelection.IsEmpty())
		{
			SelectionDetails->SetObject(nullptr);
		}
	}
}


class SInstancedObjectArrayListEntry : public STableRow<TSharedPtr<FInstancedObjectArrayStructCustomization::FListHandleEntry>> 
{
	typedef FInstancedObjectArrayStructCustomization::FListHandleEntry FEntryType;
public:
	SLATE_BEGIN_ARGS(SInstancedObjectArrayListEntry) { }
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, const TSharedPtr<FEntryType>& InEntry)
	{
		ListEntry = InEntry;
		if (!ListEntry.IsValid())
		{
			return;
		}
		ListEntry->DragDropHandler->RowWidgetWeak = AsShared();

		STableRow::Construct(
			STableRow::FArguments()
			.Style( &FAppStyle::GetWidgetStyle<FTableRowStyle>("SimpleTableView.Row") )
			.Padding(0)
			.OnDragDetected(this, &SInstancedObjectArrayListEntry::OnRowDragDetected)
			.OnCanAcceptDrop(this, &SInstancedObjectArrayListEntry::OnRowCanAcceptDrop)			
			.OnAcceptDrop(this, &SInstancedObjectArrayListEntry::OnRowAcceptDrop)
			.Content()
			[
				SNew(SVerticalBox)				
				+ SVerticalBox::Slot()				
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 10, 0)
					[
						SNew(SBox)
						.Padding(0.0f)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.WidthOverride(16.0f)
						.MinDesiredHeight(26.0f)
						[
							SNew(SImage)
							.Image(FCoreStyle::Get().GetBrush("VerticalBoxDragIndicatorShort"))
							.Visibility_Lambda([this]()
							{
								return IsHovered() ? EVisibility::Visible : EVisibility::Hidden;
							})
						]
					]
					+ SHorizontalBox::Slot()
					[
						SNew(SInstancedObjectHeader, ListEntry->Handle)
						.bAlwaysShowPropertyButtons(true)
					]
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SImage)
					.Image(FAppStyle::Get().GetBrush("DetailsView.GridLine"))
					.DesiredSizeOverride(FVector2D(1, 1.5))
				]
			]
		, InOwnerTableView);
	}

	FReply OnRowDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
	{
		if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
		{
			TSharedPtr<FDragDropOperation> DragDropOp = ListEntry->DragDropHandler->CreateDragDropOperation();
			if (DragDropOp.IsValid())
			{
				return FReply::Handled().BeginDragDrop(DragDropOp.ToSharedRef());
			}
		}
		return FReply::Unhandled();
	}

	TOptional<EItemDropZone> OnRowCanAcceptDrop(const FDragDropEvent& InDragDropEvent, EItemDropZone InDropZone, TSharedPtr<FEntryType> Item)
	{		
		return Item->DragDropHandler->CanAcceptDrop(InDragDropEvent, InDropZone);
	}

	FReply OnRowAcceptDrop(const FDragDropEvent& InDragDropEvent, EItemDropZone InDropZone, TSharedPtr<FEntryType> Item)
	{			
		if (Item->DragDropHandler->AcceptDrop(InDragDropEvent, InDropZone))
		{
			return FReply::Handled();
		}
		return FReply::Unhandled();
	}
	
private:
	TSharedPtr<FEntryType> ListEntry;
};

TSharedRef<ITableRow> FInstancedObjectArrayStructCustomization::OnGenerateWidgetForList(TSharedPtr<FListHandleEntry> ListEntry, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SInstancedObjectArrayListEntry, OwnerTable, ListEntry);
}

void FInstancedObjectArrayStructCustomization::OnSelectionChanged(TSharedPtr<FListHandleEntry> InItem, ESelectInfo::Type SelectInfo)
{
	if (List.IsValid())
	{
		TArray<TSharedPtr<FListHandleEntry>> Selected;
		List->GetSelectedItems(Selected);

		TArray<UObject*> Objects;
		for (const TSharedPtr<FListHandleEntry>& Item : Selected)
		{
			UObject* ObjectValue = nullptr;
			if (Item->Handle.IsValid() && Item->Handle->GetValue(ObjectValue) == FPropertyAccess::Success && ObjectValue)
			{
				Objects.Add(ObjectValue);
			}	
		}
		
		SelectionDetails->SetObjects(Objects);
	}
}

#undef LOCTEXT_NAMESPACE

