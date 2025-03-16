// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#include "InstancedObjectEditor.h"

#include "InstancedObjectArrayCustomization.h"
#include "InstancedObjectArrayStruct.h"
#include "InstancedObjectStruct.h"
#include "InstancedObjectStructCustomization.h"
#include "InstancedObjectViewCustomization.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FInstancedObjectEditorModule"


class FInstancedObjectEditorModuleInterface : public IInstancedObjectEditorModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FCoreDelegates::OnPostEngineInit.AddRaw(this, &FInstancedObjectEditorModuleInterface::OnPostEngineInit);		
	}

	void OnPostEngineInit()
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked< FPropertyEditorModule >("PropertyEditor");
		for (TObjectIterator<UScriptStruct> It; It; ++It)
		{
			UScriptStruct* Struct  = *It;
			if (Struct->HasMetaData(TEXT("Hidden")))
			{
				continue;
			}
			
			if (Struct->IsChildOf(FInstancedObjectStructBase::StaticStruct()))
			{
				FName StructName = Struct->GetFName();
				RegisteredStructs.Add(StructName);
				PropertyModule.RegisterCustomPropertyTypeLayout(StructName, FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FInstancedObjectStructCustomization::MakeInstance));
			}
			else if (Struct->IsChildOf(FInstancedObjectArrayStructBase::StaticStruct()))
			{
				FName StructName = Struct->GetFName();
				RegisteredStructs.Add(StructName);
				PropertyModule.RegisterCustomPropertyTypeLayout(StructName, FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FInstancedObjectArrayStructCustomization::MakeInstance));
			}	
		}

		
		PropertyModule.RegisterCustomPropertyTypeLayout(RegisteredStructs.Add_GetRef(FInstancedObjectView::StaticStruct()->GetFName()),
			FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FInstancedObjectViewCustomization::MakeInstance));

	}
	
	virtual void ShutdownModule() override
	{
		FCoreDelegates::OnPostEngineInit.RemoveAll(this);
		if (FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
		{
			for (const FName& Name : RegisteredStructs)
			{
				PropertyModule->UnregisterCustomPropertyTypeLayout(Name);
			}
		}
	}
private:
	TArray<FName> RegisteredStructs;
};



#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FInstancedObjectEditorModuleInterface, InstancedObjectEditor)