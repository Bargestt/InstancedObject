// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"
#include "InstancedObjectStruct.generated.h"


/*
 * Base class for structs with instanced objects 
 * Provides better control with class filtering and custom DetailRow
 * Inherit from this struct to benefit from Detail customization
 *
 * Add metadata to USTRUCT declaration to configure filtering
 *	BaseClass, MustImplement, AllowedClasses, DisallowedClasses
 *	 - Selects first from: Container, StructProperty, ObjectProperty, StructClassDeclaration
 *	 
 *  - Supported metadata in struct UPROPERTY
 *    - MinWidth, MaxWidth - size of class selector
 *  
 *  - Uses InstancedObjectInterface to display object Title and Tooltip
 *  - Add IndentSize to configure tooltip auto-indent. Set to -1 to disable auto-indent
 *  - Start text with <NoIndent> to disable indentation for this text
 *  - Uses FAppStyle to decorate text
 *
 *  - Promote Object properties to header using meta Header="Key1:Value,Key2:Value"    
 *    - MinWidth, MaxWidth
 *    - Padding, PaddingL, PaddingT, PaddingR, PaddingB
 *    - HideName
 *    
 *  Common ClassHeader:
 *  UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, HideCategories=(Hidden), CollapseCategories)
 */
USTRUCT(BlueprintType, BlueprintInternalUseOnly, meta=(Hidden))
struct INSTANCEDOBJECT_API FInstancedObjectStructBase
{
	GENERATED_BODY();
public:
	virtual ~FInstancedObjectStructBase()
	{
		
	}	
	virtual UObject* Get() const
	{
		return nullptr;
	}	
	virtual bool IsValid() const 
	{ 
		return Get() != nullptr;
	}	
	virtual UClass* GetClass() const
	{
		const UObject* Obj = Get();
		return Obj ? Obj->GetClass() : nullptr;
	}
};


USTRUCT(BlueprintType)
struct INSTANCEDOBJECT_API FInstancedObjectView
{
	GENERATED_BODY();
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Instanced, Category=Instanced)
	TObjectPtr<UObject> Object;
};
