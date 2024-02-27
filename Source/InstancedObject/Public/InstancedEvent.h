// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InstancedObjectInterface.h"
#include "InstancedObjectStruct.h"
#include "UObject/Object.h"
#include "InstancedEvent.generated.h"


class UInstancedEvent;


USTRUCT(BlueprintType)
struct INSTANCEDOBJECT_API FInstancedEventContext
{
	GENERATED_BODY()
public:
	virtual ~FInstancedEventContext()
	{	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event")
	TObjectPtr<UObject> WorldContextObject;
};

/*
 * Instanced object with custom display Title and Tooltip
 *
 * Can filter selection using BaseClass meta data(USTRUCT or UPROPERTY)
 */
USTRUCT(BlueprintType, meta=(AdvancedWidget, IndentSize=4))
struct INSTANCEDOBJECT_API FInstancedEventStruct : public FInstancedObjectStructBase
{
	GENERATED_BODY();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category="Object")
	TObjectPtr<UInstancedEvent> Object;

	virtual UObject* Get() const override;

	void ExecuteEvent(const FInstancedEventContext& Context) const;
};


/**
 * 
 */
UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, HideCategories=(Hidden), CollapseCategories)
class INSTANCEDOBJECT_API UInstancedEvent : public UObject, public IInstancedObjectInterface
{
	GENERATED_BODY()
	
	TObjectPtr<UWorld> World;	
public:
	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintCallable, Category = "InstancedEvent")
	void Execute(const FInstancedEventContext& Context);

protected:	
	UFUNCTION(BlueprintNativeEvent)
	void ExecuteEvent(const FInstancedEventContext& Context);
	virtual void ExecuteEvent_Implementation(const FInstancedEventContext& Context)
	{
		
	}
};


UCLASS()
class INSTANCEDOBJECT_API UInstancedEventBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintPure, Category = "InstancedEvent")
	static bool IsInstancedEventValid(const FInstancedEventStruct& Struct)
	{
		return IsValid(Struct.Object);
	}

	UFUNCTION(BlueprintCallable, Category = "InstancedEvent", meta=(WorldContext="WorldContextObject", AutoCreateRefTerm = "Context"))
	static void ExecuteInstancedEvent(UObject* WorldContextObject, const FInstancedEventStruct& Event, const FInstancedEventContext& Context);
};


/** Multiple events executed in order */
UCLASS(NotBlueprintable, meta = (DisplayName = ".Multi"))
class INSTANCEDOBJECT_API UInstancedEvent_Multi : public UInstancedEvent
{
	GENERATED_BODY()
public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event")
	TArray<FInstancedEventStruct> Events;

	virtual void ExecuteEvent_Implementation(const FInstancedEventContext& Context) override;
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const override;
};

