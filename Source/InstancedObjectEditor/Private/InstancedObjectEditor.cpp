// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#include "InstancedObjectEditor.h"

#include "InstancedObjectStruct.h"
#include "InstancedObjectStructCustomization.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FInstancedObjectEditorModule"


class FInstancedObjectEditorModuleInterface : public IInstancedObjectEditorModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked< FPropertyEditorModule >("PropertyEditor");
		
		for (TObjectIterator<UScriptStruct> It; It; ++It)
		{
			UScriptStruct* Struct  = *It;
			if (!Struct->HasMetaData(TEXT("Hidden")) && Struct->IsChildOf(FInstancedObjectBase::StaticStruct()))
			{
				FName StructName = Struct->GetFName();
				RegisteredStructs.Add(StructName);
				PropertyModule.RegisterCustomPropertyTypeLayout(StructName, FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FInstancedObjectStructCustomization::MakeInstance));
			}			
		}
	}
	virtual void ShutdownModule() override
	{
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