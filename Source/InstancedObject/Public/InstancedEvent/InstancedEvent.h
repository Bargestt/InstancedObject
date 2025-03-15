// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InstancedObjectInterface.h"
#include "InstancedObjectStruct.h"
#include "InstancedStruct.h"
#include "InstancedEvent.generated.h"


class UInstancedEvent;

struct INSTANCEDOBJECT_API FInstancedEventTags
{
public:
	FGameplayTag Tag_EventEnd;
	FGameplayTag Tag_EventSuccess;
	FGameplayTag Tag_EventFail;
	
	FGameplayTag Tag_ReplaceInvalid;
	
public:
	static const FInstancedEventTags& Get() { return Instance; }
	static void Initialize(const FInstancedEventTags& Tags);

private:
	static FInstancedEventTags Instance;
};


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
	FGameplayTag Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FInstancedStruct Data;

	bool IsEndEvent() const { return Type.MatchesTag(FInstancedEventTags::Get().Tag_EventEnd); }
	bool IsSuccessEvent() const { return Type.MatchesTag(FInstancedEventTags::Get().Tag_EventSuccess); }
	bool IsFailEvent() const { return Type.MatchesTag(FInstancedEventTags::Get().Tag_EventFail); }
};
DECLARE_DYNAMIC_DELEGATE_OneParam(FInstancedEventResultDelegate, const FInstancedEventResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInstancedEventResultSignature, const FInstancedEventResult&, Result);
DECLARE_MULTICAST_DELEGATE_OneParam(FInstancedEventResultSignature_Native, const FInstancedEventResult&);





/*
 * Instanced object with custom display Title and Tooltip
 *
 * Can filter selection using BaseClass meta data(USTRUCT or UPROPERTY)
 */
USTRUCT(BlueprintType, meta=(IndentSize=4))
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

	UFUNCTION(BlueprintCallable, Category = "InstancedEvent")
	virtual void Cancel();

protected:		
	virtual void ExecuteEvent(const FInstancedEventContext& Context);
	
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "Execute Event"))
	void BP_ExecuteEvent(const FInstancedEventContext& Context);

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "Cancel Event"))
	void BP_CancelEvent();

protected:
	UFUNCTION(BlueprintCallable, Category = "InstancedEvent", meta=(BlueprintProtected, AutoCreateRefTerm="Type,Data", ShowTreeView))
	void BroadcastResult(const FGameplayTag& Type, const FInstancedStruct& Data = FInstancedStruct());

public:
	FInstancedEventResultSignature OnResult;
	FInstancedEventResultSignature_Native OnResultNative;
};

UCLASS(Abstract, NotBlueprintable)
class INSTANCEDOBJECT_API UInstancedEvent_Operator : public UInstancedEvent
{
	GENERATED_BODY()
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
	static void ExecuteInstancedEvent(UObject* WorldContextObject, const FInstancedEventStruct& Event, const FInstancedEventContext& Context, const FInstancedEventResultDelegate& Result);


	UFUNCTION(BlueprintPure, Category = "InstancedEvent")
	static FGameplayTag GetInstancedEventTag_End() { return FInstancedEventTags::Get().Tag_EventEnd; }

	UFUNCTION(BlueprintPure, Category = "InstancedEvent")
	static FGameplayTag GetInstancedEventTag_Success() { return FInstancedEventTags::Get().Tag_EventSuccess; }

	UFUNCTION(BlueprintPure, Category = "InstancedEvent")
	static FGameplayTag GetInstancedEventTag_Fail() { return FInstancedEventTags::Get().Tag_EventFail; }

	UFUNCTION(BlueprintPure, Category = "InstancedEvent", meta=(DisplayName="Is End Result"))
	static bool IsInstancedEventResult_End(const FInstancedEventResult& Result) { return Result.Type.MatchesTag(FInstancedEventTags::Get().Tag_EventEnd); }

	UFUNCTION(BlueprintPure, Category = "InstancedEvent", meta=(DisplayName="Is Success Resul"))
	static bool IsInstancedEventResult_Success(const FInstancedEventResult& Result) { return Result.Type.MatchesTag(FInstancedEventTags::Get().Tag_EventSuccess); }

	UFUNCTION(BlueprintPure, Category = "InstancedEvent", meta=(DisplayName="Is Fail Result"))
	static bool IsInstancedEventResult_Fail(const FInstancedEventResult& Result) { return Result.Type.MatchesTag(FInstancedEventTags::Get().Tag_EventFail); }
	
	
};