// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InstancedObjectInterface.h"
#include "InstancedObjectStruct.h"
#if ENGINE_MINOR_VERSION >= 5
	#include "StructUtils/InstancedStruct.h"
#else
	#include "InstancedStruct.h"
#endif 
#include "InstancedCondition.generated.h"


class UInstancedCondition;


USTRUCT(BlueprintType)
struct INSTANCEDOBJECT_API FInstancedConditionContext
{
	GENERATED_BODY()
public:
	virtual ~FInstancedConditionContext()
	{	}

	FInstancedConditionContext()
		: WorldContextObject(nullptr)
	{ }
	
	FInstancedConditionContext(UObject* Object)
		: WorldContextObject(Object)
	{ }	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition")
	TObjectPtr<UObject> WorldContextObject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition")
	FInstancedStruct Payload;
};

/*
 * Instanced object with custom display Title and Tooltip
 *
 * Can filter selection using BaseClass meta data(USTRUCT or UPROPERTY)
 */
USTRUCT(BlueprintType, meta=(IndentSize=4))
struct INSTANCEDOBJECT_API FInstancedConditionStruct : public FInstancedObjectStructBase
{
	GENERATED_BODY();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category="Object")
	TObjectPtr<UInstancedCondition> Object;

	virtual UObject* Get() const override;

	/* Returns DefaultValue if condition is invalid */
	bool CheckCondition(const FInstancedConditionContext& Context, bool bDefaultValue = true) const;
};


/**
 * 
 */
UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, HideCategories=(Hidden), CollapseCategories)
class INSTANCEDOBJECT_API UInstancedCondition : public UObject, public IInstancedObjectInterface
{
	GENERATED_BODY()
	
	TObjectPtr<UWorld> World;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Condition", meta=(DisplayPriority=-100, EditCondition="IsClassDefaultObject", EditConditionHides))
	bool bCanInvert = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition", meta=(DisplayPriority=-100, EditCondition="bCanInvert", EditConditionHides))
	bool bInvert = false;
	
public:
	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintCallable, Category = "InstancedCondition")
	bool Check(const FInstancedConditionContext& Context);
	
	bool Check(const FInstancedConditionContext& Context, UWorld* WorldOverride);

protected:	
	UFUNCTION(BlueprintNativeEvent)
	bool CheckCondition(const FInstancedConditionContext& Context);
	virtual bool CheckCondition_Implementation(const FInstancedConditionContext& Context)
	{
		return false;
	}

	UFUNCTION(BlueprintPure, Category = "InstancedCondition", meta=(BlueprintProtected))
	static FString ApplyOperatorStyle(const FString& String)
	{
		return FString(TEXT("<RichTextBlock.Bold>")) + String + TEXT("</>");
	}

#if WITH_EDITOR
	UFUNCTION()
	bool IsClassDefaultObject() const { return this->HasAllFlags(RF_ClassDefaultObject); }
#endif //
};


UCLASS()
class INSTANCEDOBJECT_API UInstancedConditionBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintPure, Category = "InstancedCondition")
	static bool IsInstancedConditionValid(const FInstancedConditionStruct& Struct)
	{
		return IsValid(Struct.Object);
	}

	/*
	 * @param	Context		Condition context passed to execution
	 * @param	bDefaultValue	ReturnValue when condition is invalid
	 */
	UFUNCTION(BlueprintCallable, Category = "InstancedCondition", meta=(WorldContext="WorldContextObject", CallableWithoutWorldContext, AutoCreateRefTerm = "Context", AdvancedDisplay=2))
	static bool CheckInstancedCondition(UObject* WorldContextObject, const FInstancedConditionStruct& Condition, const FInstancedConditionContext& Context, bool bDefaultValue = true);
};



/*--------------------------------------------
	 Default boolean operators
 *--------------------------------------------*/

/** Simple bool */
UCLASS(NotBlueprintable, meta = (DisplayName = ".Bool"))
class INSTANCEDOBJECT_API UInstancedCondition_Bool : public UInstancedCondition
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition")
	bool bBool = true;
public:
	UInstancedCondition_Bool();
	virtual bool CheckCondition_Implementation(const FInstancedConditionContext& Context) override;
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
class INSTANCEDOBJECT_API UInstancedCondition_LogicOperator : public UInstancedCondition
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition")
	EConditionLogicOperatorType Type;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition")
	TArray<FInstancedConditionStruct> Conditions;

	virtual bool CheckCondition_Implementation(const FInstancedConditionContext& Context) override;
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const override;
};

/** External condition asset */
UCLASS(NotBlueprintable, meta = (DisplayName = ".External"))
class INSTANCEDOBJECT_API UInstancedCondition_External : public UInstancedCondition
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition")
	TObjectPtr<class UInstancedConditionAsset> Condition;
public:
	virtual bool CheckCondition_Implementation(const FInstancedConditionContext& Context) override;
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const override;
};