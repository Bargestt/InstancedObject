
#pragma once

#include "IDetailCustomNodeBuilder.h"

class SInstancedObjectHeader;

class FInstancedObjectBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis<FInstancedObjectBuilder>
{
public:
	TOptional<bool> AlwaysShowPropertyButtons;
	TOptional<bool> DisplayDefaultPropertyButtons;
	
public:
	FInstancedObjectBuilder(const TSharedRef<IPropertyHandle>& InObjectHandle, const TSharedPtr<IPropertyHandle>& InStructHandle = nullptr);
	virtual ~FInstancedObjectBuilder() override;;

	//~ Begin IDetailCustomNodeBuilder interface
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override
	{
		OnRegenerateChildren = InOnRegenerateChildren;
	};
	
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& HeaderRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildBuilder) override;
	
	virtual bool InitiallyCollapsed() const override { return false; }
	virtual FName GetName() const override
	{
		static const FName Name("FInstancedObjectBuilder");
		return Name;
	}

	virtual TSharedPtr<IPropertyHandle> GetPropertyHandle() const override { return ObjectHandle; }
	//~ End IDetailCustomNodeBuilder interface


	
	DECLARE_DELEGATE_OneParam( FOnHeaderRowGenerated, FDetailWidgetRow& );
	void OnHeaderGenerated( FOnHeaderRowGenerated InDelegate )
	{
		OnHeaderRowGenerated = InDelegate;
	}
private:
	FSimpleDelegate OnRegenerateChildren;
	FOnHeaderRowGenerated OnHeaderRowGenerated;

	TSharedPtr<IPropertyHandle> StructHandle;
	TSharedPtr<IPropertyHandle> ObjectHandle;

	TSharedPtr<SInstancedObjectHeader> HeaderWidget;	
};
