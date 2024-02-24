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
 *	BaseClass, MustImplement
 *	 - Selects first from: Container, StructProperty, ObjectProperty, StructClassDeclaration
 *	 - Adding filtering will switch to custom EditInlineNew editor, it has limited functionality
 *	
 * 
 *  AdvancedWidget - Display with fancy widget
 *  - Uses InstancedObjectInterface to display object Title and Tooltip
 *  - Add IndentSize to configure tooltip auto-indent. Set to -1 to disable auto-indent
 *  - Start text with <NoIndent> to disable indentation for this text
 *  - Uses FAppStyle to decorate text
 */
USTRUCT(meta=(Hidden))
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
