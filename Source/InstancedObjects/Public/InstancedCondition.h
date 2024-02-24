// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InstancedObjectInterface.h"
#include "InstancedObjectStruct.h"
#include "UObject/Object.h"
#include "InstancedCondition.generated.h"


class UInstancedCondition;


USTRUCT(BlueprintType)
struct INSTANCEDOBJECTS_API FConditionContext
{
	GENERATED_BODY()
public:
	virtual ~FConditionContext()
	{	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition")
	TObjectPtr<UObject> WorldContextObject;
};

/*
 * Instanced object with custom display Title and Tooltip
 *
 * Can filter selection using BaseClass meta data(USTRUCT or UPROPERTY)
 */
USTRUCT(BlueprintType, meta=(AdvancedWidget, IndentSize=4))
struct INSTANCEDOBJECTS_API FInstancedConditionStruct : public FInstancedObjectStructBase
{
	GENERATED_BODY();
public:
	UPROPERTY(EditAnywhere, Instanced, Category="Object")
	TObjectPtr<UInstancedCondition> Object;

	virtual UObject* Get() const override;

	/* Returns DefaultValue if condition is invalid */
	bool CheckCondition(const FConditionContext& Context, bool bDefaultValue = true) const;
};


/**
 * 
 */
UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, HideCategories=(Hidden), CollapseCategories)
class INSTANCEDOBJECTS_API UInstancedCondition : public UObject, public IInstancedObjectInterface
{
	GENERATED_BODY()
	
	TObjectPtr<UWorld> World;
	
protected:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition", meta=(DisplayPriority=-100, EditCondition="CanInvert", EditConditionHides))
	bool bInvert;
	
public:
	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintCallable, Category = "InstancedCondition")
	bool Check(const FConditionContext& Context);

protected:	
	UFUNCTION(BlueprintNativeEvent)
	bool CheckCondition(const FConditionContext& Context);
	virtual bool CheckCondition_Implementation(const FConditionContext& Context)
	{
		return false;
	}

	UFUNCTION(BlueprintPure, Category = "InstancedCondition", meta=(BlueprintProtected))
	static FString ApplyOperatorStyle(const FString& String)
	{
		return FString(TEXT("<RichTextBlock.Bold>")) + String + TEXT("</>");
	}

	UFUNCTION()
	virtual bool CanInvert() const { return true; }
	
};


UCLASS()
class INSTANCEDOBJECTS_API UInstancedConditionBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintPure, Category = "InstancedCondition")
	static bool IsInstancedConditionValid(const FInstancedConditionStruct& Condition);

	UFUNCTION(BlueprintCallable, Category = "InstancedCondition", meta=(WorldContext="WorldContextObject", AutoCreateRefTerm = "Context"))
	static bool CheckInstancedCondition(UObject* WorldContextObject, const FInstancedConditionStruct& Condition, const FConditionContext& Context);
};



/*--------------------------------------------
	 Default boolean operators
 *--------------------------------------------*/

/** To meet condition all nested conditions must be met */
UCLASS(NotBlueprintable, meta = (DisplayName = ".Bool"))
class INSTANCEDOBJECTS_API UInstancedCondition_Bool : public UInstancedCondition
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition")
	bool bBool = true;
public:
	virtual bool CanInvert() const override { return false; }
	virtual bool CheckCondition_Implementation(const FConditionContext& Context) override;
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const override;
};

UENUM(BlueprintType)
enum class EConditionLogicOperatorType : uint8
{
	/* All conditions return true */
	And UMETA(DisplayName="AND"),

	/* Any of conditions return true */
	Or UMETA(DisplayName="OR"),
	
	/* All conditions return same value */
	Equal
};

/** AND, OR, Equal operators */
UCLASS(NotBlueprintable, meta = (DisplayName = ".LogicOperator"))
class INSTANCEDOBJECTS_API UInstancedCondition_LogicOperator : public UInstancedCondition
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition")
	EConditionLogicOperatorType Type;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition")
	TArray<FInstancedConditionStruct> Conditions;

	virtual bool CheckCondition_Implementation(const FConditionContext& Context) override;
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const override;
};

