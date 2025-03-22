// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InstancedObjectArrayStruct.generated.h"



struct IInstancedObjectArrayStruct
{
	virtual ~IInstancedObjectArrayStruct() { }
	
	virtual TArray<UObject*> GetObjects() const = 0;
	virtual int32 GetNumObjects() const = 0;
	virtual UObject* GetObjectAt(int32 Index) const = 0;
	virtual void RemoveObjectAt(int32 Index) = 0;
	virtual int32 InsertObjectAt(UObject* Object, int32 Index) = 0;
};


/**
 * Allows use of horizontal editor with AdvancedWidget metadata
 *  ---------------------------------
 *  |  Obj1		|	Obj2 Details	|
 *  | >Obj2		|   -Prop1			|
 *  |  Obj3		|   -Prop2			|
 *  |			|					|
 *  ---------------------------------
 */
USTRUCT(BlueprintType, BlueprintInternalUseOnly, meta=(Hidden))
struct INSTANCEDOBJECT_API FInstancedObjectArrayStructBase
{
	GENERATED_BODY();
public:
	virtual ~FInstancedObjectArrayStructBase() { }
};




struct CInstancedObjectArrayMemberProvider
{
	template <typename TStructType, typename TObjectType>
	auto Requires(const TStructType& InStruct, TArray<TObjectPtr<TObjectType>>& OutObjects) -> decltype(
		OutObjects = InStruct.Objects
	);
};

template<typename TStructType, typename TObjectType>
struct TInstancedObjectArrayStruct : public IInstancedObjectArrayStruct
{
public:
	const TArray<TObjectPtr<TObjectType>>& Get() const
	{
		static_assert(TModels_V<CInstancedObjectArrayMemberProvider, TStructType, TObjectType>, "Must have Objects member of type TArray<TObjectPtr<TObjectType>>");
		
		return static_cast<const TStructType*>(this)->Objects;
	}
	
	TArray<TObjectPtr<TObjectType>>& Get()
	{
		static_assert(TModels_V<CInstancedObjectArrayMemberProvider, TStructType, TObjectType>, "Must have Objects member of type TArray<TObjectPtr<TObjectType>>");
		
		return static_cast<TStructType*>(this)->Objects;
	}

	void Set(const TArray<TObjectPtr<TObjectType>>& NewValue)
	{	
		static_cast<TStructType*>(this)->Objects = NewValue;		
	}

	UClass* GetBaseClass() const
	{
		return TObjectType::StaticClass();
	}
	
	//~ Begin IInstancedObjectArrayStruct Interface
	virtual TArray<UObject*> GetObjects() const override
	{
		TArray<UObject*> Result;
		Result.Append(Get());
		return Result;
	}
	
	virtual int32 GetNumObjects() const override
	{
		return Get().Num();
	}
	
	virtual UObject* GetObjectAt(int32 Index) const override
	{
		return Get().IsValidIndex(Index) ? Get()[Index] : nullptr;
	}
	
	virtual void RemoveObjectAt(int32 Index) override
	{
		Get().RemoveAt(Index);
	}
	
	virtual int32 InsertObjectAt(UObject* Object, int32 Index) override
	{
		if (Object == nullptr)
		{
			return Get().Insert(nullptr, Index);
		}
		
		if(TObjectType* Casted = Cast<TObjectType>(Object))
		{
			return Get().Insert(Casted, Index);
		}
		
		return -1;
	}
	//~ End IInstancedObjectArrayStruct Interface
};
