// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InstancedEvent.h"
#include "InstancedEvent_Multi.generated.h"


struct FInstancedEventExecutor;


/** Execute multiple events */
UCLASS(NotBlueprintable, meta = (DisplayName = ".Multi"))
class INSTANCEDOBJECT_API UInstancedEvent_Multi : public UInstancedEvent_Operator
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", NoClear, meta=(ExcludebaseStruct, AllowedClasses="AllowedExecutors", DisallowedClasses="DisallowedExecutors"))
	TInstancedStruct<FInstancedEventExecutor> Execution;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(InheritFilters))
	TArray<FInstancedEventStruct> Events;
	
	UInstancedEvent_Multi();
	virtual void ExecuteEvent(const FInstancedEventContext& Context) override;
	virtual void Cancel() override;
	virtual void GetSubEvents_Implementation(TArray<UInstancedEvent*>& OutEvents) const override;
	virtual FString GetInstancedObjectTitle_Implementation(bool bFullTitle) const override;

	virtual void BeginDestroy() override;

protected:
	virtual void OnExecutorResult(const FInstancedEventResult& Result);


#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TArray<TObjectPtr<UScriptStruct>> AllowedExecutors;

	UPROPERTY()
	TArray<TObjectPtr<UScriptStruct>> DisallowedExecutors;
#endif //
};