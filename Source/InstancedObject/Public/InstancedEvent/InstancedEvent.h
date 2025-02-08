// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InstancedObjectInterface.h"
#include "InstancedObjectStruct.h"
#include "StructUtils/InstancedStruct.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event")
	FInstancedStruct Payload;
};


USTRUCT(BlueprintType)
struct FInstancedEventResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TObjectPtr<UInstancedEvent> Event;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FInstancedStruct Data;
};
DECLARE_DYNAMIC_DELEGATE_OneParam(FInstancedEventResultDelegate, const FInstancedEventResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInstancedEventResultSignature, const FInstancedEventResult&, Result);
DECLARE_MULTICAST_DELEGATE_OneParam(FInstancedEventResultSignature_Native, const FInstancedEventResult&);


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
protected:
	TWeakObjectPtr<UWorld> CachedWorld;
	
public:
	UInstancedEvent();
	virtual UWorld* GetWorld() const override;
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif //


	UFUNCTION(BlueprintCallable, Category = "InstancedEvent")
	void Execute(const FInstancedEventContext& Context);

protected:		
	virtual void ExecuteEvent(const FInstancedEventContext& Context);
	
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "Execute Event"))
	void BP_ExecuteEvent(const FInstancedEventContext& Context);

protected:
	UFUNCTION(BlueprintCallable, Category = "InstancedEvent", meta=(BlueprintProtected, AutoCreateRefTerm="Data"))
	void BroadcastResult(const FInstancedStruct& Data);

public:
	FInstancedEventResultSignature OnResult;
	FInstancedEventResultSignature_Native OnResultNative;
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

	UFUNCTION(BlueprintCallable, Category = "InstancedEvent", meta=(WorldContext="WorldContextObject", CallableWithoutWorldContext, AutoCreateRefTerm = "Context,Result", AdvancedDisplay=3))
	static void ExecuteInstancedEvent(UObject* WorldContextObject, const FInstancedEventStruct& Event, const FInstancedEventContext& Context, const FInstancedEventResultDelegate& OnEnd);


};