// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InstancedObjectUtilityLibrary.generated.h"

struct FInstancedObjectStructBase;

USTRUCT(BlueprintType)
struct INSTANCEDOBJECT_API FIndentParser
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Indent")
	int32 IndentSize = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Indent")
	FString IncreaseIndentChars = TEXT("{");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Indent")
	FString DecreaseIndentChars = TEXT("}");


	FString Apply(const FString& InString, int32& OutFinalDepth) const;
};

/**
 * 
 */
UCLASS()
class INSTANCEDOBJECT_API UInstancedObjectUtilityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "InstancedObject")
	static FString GetInstancedObjectTitle(const UObject* Object, bool bFullTitle);

	UFUNCTION(BlueprintCallable, Category = "InstancedObject")
	static FString ApplyIndent(const FIndentParser& Parser, const FString& InString, int32& OutDepth);
};
