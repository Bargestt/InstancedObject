// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InstancedEventInterface.h"
#include "InstancedObjectArrayStruct.h"
#include "InstancedObjectInterface.h"
#include "InstancedObjectStruct.h"

#if ENGINE_MINOR_VERSION >= 5
	#include "StructUtils/InstancedStruct.h"
#else
	#include "InstancedStruct.h"
#endif 

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

	FInstancedEventContext()
		: WorldContextObject(nullptr)
	{ }

	FInstancedEventContext(const FInstancedStruct& InPayload)
		: WorldContextObject(nullptr)
		, Payload(InPayload)
	{ }

	FInstancedEventContext(UObject* Object)
		: WorldContextObject(Object)
	{ }
	
	FInstancedEventContext(UObject* Object, const FInstancedStruct& InPayload)
		: WorldContextObject(Object)
		, Payload(InPayload)
	{ }

	template<typename TPayload>
	static FInstancedEventContext Make(UObject* WorldContext, const TPayload& InPayload)
	{
		return FInstancedEventContext(WorldContext, FInstancedStruct::Make(InPayload));
	}

	template<typename TPayload>
	static FInstancedEventContext Make(const TPayload& InPayload)
	{
		return FInstancedEventContext(nullptr, FInstancedStruct::Make(InPayload));
	}

public:

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

	static FInstancedEventResult MakeSuccess(UInstancedEvent* InEvent = nullptr, const FInstancedStruct& InData = {})
	{
		return FInstancedEventResult{
			.Event = InEvent,
			.Type = FInstancedEventTags::Get().Tag_EventSuccess,
			.Data = InData
		};
	}

	static FInstancedEventResult MakeFail(UInstancedEvent* InEvent = nullptr, const FInstancedStruct& InData = {})
	{
		return FInstancedEventResult{
			.Event = InEvent,
			.Type = FInstancedEventTags::Get().Tag_EventFail,
			.Data = InData
		};
	}
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

	virtual UObject* GetObject() const override;
	virtual void SetObject(UObject* NewObject) override;
	void ExecuteEvent(const FInstancedEventContext& Context) const;
};

USTRUCT(BlueprintType, meta=(IndentSize=4))
struct INSTANCEDOBJECT_API FInstancedEventArrayStruct : public FInstancedObjectArrayStructBase
{
	GENERATED_BODY();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category="Object")
	TArray<TObjectPtr<UInstancedEvent>> Objects;
};


/**
 * 
 */
UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, HideCategories=(Hidden), CollapseCategories)
class INSTANCEDOBJECT_API UInstancedEvent : public UObject
	, public IInstancedObjectInterface
	, public IInstancedEventInterface
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


	UFUNCTION(BlueprintCallable, Category = "InstancedEvent", meta=(AutoCreateRefTerm="Context"))
	virtual void Execute(const FInstancedEventContext& Context) override final;

	UFUNCTION(BlueprintCallable, Category = "InstancedEvent")
	virtual void Cancel() override;

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
	UPROPERTY(BlueprintAssignable, Category="InstancedEvent")
	FInstancedEventResultSignature OnResult;
	
	FInstancedEventResultSignature_Native OnResultNative;
};

UCLASS(Abstract, NotBlueprintable)
class INSTANCEDOBJECT_API UInstancedEvent_Operator : public UInstancedEvent
{
	GENERATED_BODY()

public:
	TArray<UInstancedEvent*> GetSubEvents(bool bRecursive = false) const;

protected:
	virtual void GetSubEvents_Implementation(TArray<UInstancedEvent*>& OutEvents) const { }
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

	UFUNCTION(BlueprintCallable, Category = "InstancedEvent")
	static void CancelInstancedEvent(const FInstancedEventStruct& Event);

	UFUNCTION(BlueprintPure, Category = "InstancedEvent", meta=(DeterminesOutputType="Class"))
	static UInstancedEvent* GetInstancedEventAs(const FInstancedEventStruct& Event, TSubclassOf<UInstancedEvent> Class)
	{
		return (Event.Object && (!Class || Event.Object->IsA(Class))) ? Event.Object : nullptr;
	}


	UFUNCTION(BlueprintPure, Category = "InstancedEvent")
	static FGameplayTag GetInstancedEventTag_End() { return FInstancedEventTags::Get().Tag_EventEnd; }

	UFUNCTION(BlueprintPure, Category = "InstancedEvent")
	static FGameplayTag GetInstancedEventTag_Success() { return FInstancedEventTags::Get().Tag_EventSuccess; }

	UFUNCTION(BlueprintPure, Category = "InstancedEvent")
	static FGameplayTag GetInstancedEventTag_Fail() { return FInstancedEventTags::Get().Tag_EventFail; }

	UFUNCTION(BlueprintPure, Category = "InstancedEvent", meta=(DisplayName="Is End Result"))
	static bool IsInstancedEventResult_End(const FInstancedEventResult& Result) { return Result.Type.MatchesTag(FInstancedEventTags::Get().Tag_EventEnd); }

	UFUNCTION(BlueprintPure, Category = "InstancedEvent", meta=(DisplayName="Is Success Result"))
	static bool IsInstancedEventResult_Success(const FInstancedEventResult& Result) { return Result.Type.MatchesTag(FInstancedEventTags::Get().Tag_EventSuccess); }

	UFUNCTION(BlueprintPure, Category = "InstancedEvent", meta=(DisplayName="Is Fail Result"))
	static bool IsInstancedEventResult_Fail(const FInstancedEventResult& Result) { return Result.Type.MatchesTag(FInstancedEventTags::Get().Tag_EventFail); }


	UFUNCTION(BlueprintCallable, Category = "InstancedEvent", meta=(ExpandBoolAsExecs="ReturnValue"))
	static bool SwitchOnResultType(const FInstancedEventResult& Result, FGameplayTag Type, bool bExact)
	{
		return bExact ? Result.Type == Type : Result.Type.MatchesTag(Type);
	}
	
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "InstancedEvent", meta = (CustomStructureParam = "Value", ExpandBoolAsExecs="ReturnValue"))
	static bool GetResultData(const FInstancedEventResult& Result, FGameplayTag Type, bool bExact, int32& Value) { checkNoEntry(); return false; }
	DECLARE_FUNCTION(execGetResultData);
};