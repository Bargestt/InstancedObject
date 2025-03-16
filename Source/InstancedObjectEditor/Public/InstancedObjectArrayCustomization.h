

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

class IPropertyHandle;


class INSTANCEDOBJECTEDITOR_API FInstancedObjectArrayStructCustomization : public IPropertyTypeCustomization
{	
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new FInstancedObjectArrayStructCustomization);
	}

	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	//~ End IPropertyTypeCustomization Interface

	struct FListHandleEntry
	{
		TSharedPtr<IPropertyHandle> Handle;
		TSharedPtr<class FInstancedObjectArrayStructDragDropHandler> DragDropHandler;
	};

protected:	
	TSharedRef<ITableRow> OnGenerateWidgetForList(TSharedPtr<FListHandleEntry> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void OnSelectionChanged(TSharedPtr<FListHandleEntry> Item, ESelectInfo::Type SelectInfo);

protected:
	TSharedPtr<IPropertyHandle> ArrayHandle;
	
	
	TWeakPtr<IPropertyUtilities> PropertyUtilitiesWeak;


	TArray<TSharedPtr<FListHandleEntry>> ListEntries;	
	TSharedPtr<SListView<TSharedPtr<FListHandleEntry>>> List;
	
	TSharedPtr<IDetailsView> SelectionDetails;
};

