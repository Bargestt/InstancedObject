#include "SInstancedObjectHeader.h"

#include "InstancedObjectEditorUtils.h"
#include "Widgets/Text/SRichTextBlock.h"


void SInstancedObjectHeader::Construct(const FArguments& InArgs, const TSharedPtr<IPropertyHandle>& InObjectHandle)
{
	ObjectHandle = InObjectHandle;
	if (!ObjectHandle.IsValid())
	{
		return;
	}

	ObjectHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &SInstancedObjectHeader::UpdateTitle));
	ObjectHandle->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &SInstancedObjectHeader::UpdateTitle));	

	bool bShowDefaultPropertyButtons = InArgs._bDisplayDefaultPropertyButtons && !InArgs._bAlwaysShowPropertyButtons;
	TSharedRef<SWidget> ObjectEditor = ObjectHandle->CreatePropertyValueWidget(bShowDefaultPropertyButtons);
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
			TooltipHolder->SetToolTip(GetTooltipWidget());
		}	
	}		
	UpdateTitle();

	int32 MinWidth = InArgs._MinDesiredWidth;
	if (const FString* Meta = FInstancedObjectEditorUtils::FindMetaData(ObjectHandle, FInstancedObjectMeta::MD_MinWidth))
	{
		LexFromString(MinWidth, **Meta);
	}
	
	int32 MaxWidth = InArgs._MaxDesiredWidth;
	if (const FString* Meta = FInstancedObjectEditorUtils::FindMetaData(ObjectHandle, FInstancedObjectMeta::MD_MaxWidth))
	{
		LexFromString(MaxWidth, **Meta);
	}
		
	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SBox)
			.MinDesiredWidth(MinWidth > 0 ? MinWidth : FOptionalSize())
			.MaxDesiredWidth(MaxWidth > 0 ? MaxWidth : FOptionalSize())
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ContentPadding(FMargin(0))
				.ToolTip(GetTooltipWidget())	
				.OnHovered(this, &SInstancedObjectHeader::SetEditorVisible, true)
				.OnUnhovered(this, &SInstancedObjectHeader::SetEditorVisible, false)
				[				
					SAssignNew(Switcher, SOverlay)
					+ SOverlay::Slot().VAlign(VAlign_Center)
					[
						SNew(SRichTextBlock)
						.DecoratorStyleSet(&FAppStyle::Get())
						.TextStyle(FAppStyle::Get(), "NormalText")
						.Text(this, &SInstancedObjectHeader::GetTitle)	
					]
					+ SOverlay::Slot().VAlign(VAlign_Center).HAlign(HAlign_Left)
					[
						SNew(SBox)
						.MinDesiredWidth(75)
						.MaxDesiredWidth(400)
						[
							ObjectEditor
						]						
					]					
				]
			]
		]
		+ SHorizontalBox::Slot().HAlign(HAlign_Left)
		[
			SAssignNew(HeaderExtensionPanel, SBox)
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			InArgs._bAlwaysShowPropertyButtons ? ObjectHandle->CreateDefaultPropertyButtonWidgets() : SNullWidget::NullWidget
		]
	];

	SetEditorVisible(false);
}

void SInstancedObjectHeader::UpdateTitle()
{
	Title = FInstancedObjectEditorUtils::ReadObjectTitle(ObjectHandle, false, false);
}

void SInstancedObjectHeader::UpdateTooltip()
{
	Tooltip = FInstancedObjectEditorUtils::ReadObjectTitle(ObjectHandle, true, false);
}

void SInstancedObjectHeader::SetEditorVisible(bool bNewVisible)
{
	if (Switcher.IsValid() && Switcher->GetChildren()->Num() == 2)
	{
		if (bNewVisible)
		{
			UpdateTooltip();
			Switcher->GetChildren()->GetChildAt(0)->SetVisibility(EVisibility::Hidden);
			Switcher->GetChildren()->GetChildAt(1)->SetVisibility(EVisibility::Visible);
		}
		else
		{
			UpdateTitle();
			Switcher->GetChildren()->GetChildAt(0)->SetVisibility(EVisibility::Visible);
			Switcher->GetChildren()->GetChildAt(1)->SetVisibility(EVisibility::Hidden);
		}
	}
}

TSharedRef<IToolTip> SInstancedObjectHeader::GetTooltipWidget()
{
	if (!TooltipWidget.IsValid())
	{
		TooltipWidget = SNew(SToolTip)
		[
			SNew(SBox)
			.MinDesiredWidth(200.0f)
			[
				SNew(SRichTextBlock)
					.DecoratorStyleSet(&FAppStyle::Get())
					.TextStyle(FAppStyle::Get(), "NormalText")
					.Text(this, &SInstancedObjectHeader::GetTooltip)
			]
		];
	}
	return TooltipWidget.ToSharedRef();
}
