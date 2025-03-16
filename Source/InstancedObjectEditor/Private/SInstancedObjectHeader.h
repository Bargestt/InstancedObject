#pragma once


class SInstancedObjectHeader : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInstancedObjectHeader)
		: _bDisplayDefaultPropertyButtons(true)
		, _bAlwaysShowPropertyButtons(false)
		, _bShowEditorsInHeader(false)
		, _MinDesiredWidth(75)
		, _MaxDesiredWidth(600)
	{ }	
	SLATE_ARGUMENT( bool, bDisplayDefaultPropertyButtons )
	SLATE_ARGUMENT( bool, bAlwaysShowPropertyButtons )
	SLATE_ARGUMENT( bool, bShowEditorsInHeader )

	SLATE_ARGUMENT( int32, MinDesiredWidth )
	SLATE_ARGUMENT( int32, MaxDesiredWidth )
	
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TSharedPtr<IPropertyHandle>& InObjectHandle);

	void UpdateTitle();
	void UpdateTooltip();
	void SetEditorVisible(bool bNewVisible);

	TSharedPtr<SBox> GetHeaderExtensionPanel() const { return HeaderExtensionPanel; }

	

	FText GetTitle() const { return Title; }
	FText GetTooltip() const { return Tooltip; }	
protected:
	TSharedRef<IToolTip> GetTooltipWidget();
	
private:
	TSharedPtr<IPropertyHandle> ObjectHandle;
	
	TSharedPtr<SWidget> Switcher;
	TSharedPtr<IToolTip> TooltipWidget;

	TSharedPtr<SBox> HeaderExtensionPanel;
	
	FText Title;	
	FText Tooltip;	
};