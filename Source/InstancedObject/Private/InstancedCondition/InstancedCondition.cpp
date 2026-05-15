// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.


#include "InstancedCondition/InstancedCondition.h"

#include "InstancedCondition/InstancedConditionAsset.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InstancedCondition)

UObject* FInstancedConditionStruct::GetObject() const
{
	return Object;
}

void FInstancedConditionStruct::SetObject(UObject* NewObject)
{
	Object = Cast<UInstancedCondition>(NewObject);
}

bool FInstancedConditionStruct::CheckCondition(const FInstancedConditionContext& Context, bool bDefaultValue) const
{
	return Object ? Object->Check(Context) : bDefaultValue;
}



UWorld* UInstancedCondition::GetWorld() const
{
	return World ? World : nullptr;
}

bool UInstancedCondition::Check(const FInstancedConditionContext& Context)
{
	return Check(Context, nullptr);
}

bool UInstancedCondition::Check(const FInstancedConditionContext& Context, UWorld* WorldOverride)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UInstancedCondition::Check);
	
	if (WorldOverride == nullptr)
	{	
		if (UActorComponent* ActorComponent = Cast<UActorComponent>(GetOuter()))
		{
			WorldOverride = ActorComponent->GetWorld();
		}
		if (AActor* Actor = Cast<AActor>(GetOuter()))
		{
			WorldOverride = Actor->GetWorld();
		}
		if (UInstancedCondition* Object = Cast<UInstancedCondition>(GetOuter()))
		{
			WorldOverride = Object->GetWorld();
		}
		else
		{
			WorldOverride = GEngine->GetWorldFromContextObject(Context.WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
		}
	}
	
	World = WorldOverride;
	bool bResult = CheckCondition(Context) ^ (bCanInvert && bInvert);
	World = nullptr;
	
	return bResult;
}

TArray<UInstancedCondition*> UInstancedCondition::GetSubConditions(bool bRecursive) const
{
	TArray<UInstancedCondition*> Result;
	GetSubConditions_Implementation(Result);
	
	if (bRecursive)
	{
		TArray<UInstancedCondition*> ConditionsToCheck = Result;
		TSet<UInstancedCondition*> CheckedConditions;
		while (!ConditionsToCheck.IsEmpty())
		{
			UInstancedCondition* Condition = ConditionsToCheck[0];
			ConditionsToCheck.RemoveAt(0);
			
			if (Condition)
			{
				bool bAlreadyChecked = false;
				CheckedConditions.Add(Condition, &bAlreadyChecked);
				if (!bAlreadyChecked)
				{
					CheckedConditions.Add(Condition);
				
					TArray<UInstancedCondition*> Sub; 
					Condition->GetSubConditions_Implementation(Sub);
					ConditionsToCheck.Append(Sub);
				}							
			}
		}
		
		Result = CheckedConditions.Array();
	}
	else
	{
		// Easier to remove all here instead of hoping users will ensure no nullptr
		Result.Remove(nullptr);
	}

	return Result;
}

/*--------------------------------------------
	UInstancedConditionBlueprintLibrary
 *--------------------------------------------*/


bool UInstancedConditionBlueprintLibrary::CheckInstancedCondition(UObject* WorldContextObject, const FInstancedConditionStruct& Condition, const FInstancedConditionContext& Context, bool bDefaultValue)
{
	if (Context.WorldContextObject == nullptr)
	{
		FInstancedConditionContext NewContext = Context;
		NewContext.WorldContextObject = WorldContextObject;
		return Condition.CheckCondition(NewContext, bDefaultValue);
	}
	return Condition.CheckCondition(Context, bDefaultValue);
}

TArray<UInstancedCondition*> UInstancedConditionBlueprintLibrary::GetSubConditions(UInstancedCondition* Condition, bool bIncludeSelf, bool bRecursiveAdd)
{
	TArray<UInstancedCondition*> Result;

	if (Condition)
	{
		if (bIncludeSelf)
		{
			Result.Add(Condition);
		}
		
		Result.Append(Condition->GetSubConditions(bRecursiveAdd));
	}	
	
	return Result;
}


/*--------------------------------------------
	 Default boolean operators
 *--------------------------------------------*/

UInstancedCondition_Bool::UInstancedCondition_Bool()
	: Super()
{
	bCanInvert = false;
}

bool UInstancedCondition_Bool::CheckCondition_Implementation(const FInstancedConditionContext& Context)
{
	return bBool;
}

FString UInstancedCondition_Bool::GetInstancedObjectTitle_Implementation(bool bFullTitle) const
{
	return bBool ? TEXT("true") : TEXT("false");
}


bool UInstancedCondition_LogicOperator::CheckCondition_Implementation(const FInstancedConditionContext& Context)
{
	switch (Type)
	{
	case EConditionLogicOperatorType::And:
		{
			const bool bDefaultValue = true;
			
			for (const FInstancedConditionStruct& Condition : Conditions)
			{
				if (!Condition.CheckCondition(Context, bDefaultValue))
				{
					return false;
				}			
			}
			return true;
		}
	case EConditionLogicOperatorType::Or:
		{
			const bool bDefaultValue = false;
			
			for (const FInstancedConditionStruct& Condition : Conditions)
			{
				if (Condition.CheckCondition(Context, bDefaultValue))
				{
					return true;
				}			
			}
			return false;
		}
	case EConditionLogicOperatorType::Equal:
		{
			const bool bDefaultValue = true;
			
			bool bResult = (Conditions.Num() > 0) ? Conditions[0].CheckCondition(Context, bDefaultValue) : bDefaultValue;			
			for (int32 Index = 1; Index < Conditions.Num(); Index++)
			{
				if (bResult != Conditions[Index].CheckCondition(Context, bDefaultValue))
				{
					return false;
				}
			}			
			return true;
		}
	}
	return  false;
}

FString UInstancedCondition_LogicOperator::GetInstancedObjectTitle_Implementation(bool bFullTitle) const
{	
	switch (Type)
	{
	case EConditionLogicOperatorType::And:
		{
			const FString DefaultValue = TEXT("true");
			const FString ConditionName = ApplyOperatorStyle(bInvert ? TEXT("!AND") : TEXT("AND"));
			
			if (!bFullTitle)
			{
				return FString::Printf(TEXT("%s{ %d }"), *ConditionName, Conditions.Num());
			}
			
			TArray<FString> Lines;
			for (int32 Index = 0; Index < Conditions.Num(); Index++)
			{
				FString Line = Conditions[Index].IsValid() ? *GetTitleSafe(Conditions[Index].Get(), bFullTitle) : *DefaultValue;
				Lines.Add(Line);
			}
			if (Lines.Num() < 2)
			{
				Lines.Add(DefaultValue);
			}
			if (Lines.Num() < 2)
			{
				Lines.Add(DefaultValue);
			}
			
			return FString::Printf(TEXT("%s{%s}"), *ConditionName, *FString::Join(Lines, TEXT("\n")));
			
		}
		
	case EConditionLogicOperatorType::Or:
		{
			const FString DefaultValue = TEXT("false");
			const FString ConditionName = ApplyOperatorStyle(bInvert ? TEXT("!OR") : TEXT("OR"));
			
			if (!bFullTitle)
			{
				return FString::Printf(TEXT("%s{ %d }"), *ConditionName, Conditions.Num());
			}
			
			TArray<FString> Lines;
			for (int32 Index = 0; Index < Conditions.Num(); Index++)
			{
				FString Line = Conditions[Index].IsValid() ? *GetTitleSafe(Conditions[Index].Get(), bFullTitle) : *DefaultValue;
				Lines.Add(Line);
			}
			if (Lines.Num() < 2)
			{
				Lines.Add(DefaultValue);
			}
			if (Lines.Num() < 2)
			{
				Lines.Add(DefaultValue);
			}

			return FString::Printf(TEXT("%s{%s}"), *ConditionName, *FString::Join(Lines, TEXT("\n")));
		}
	case EConditionLogicOperatorType::Equal:
		{
			const FString DefaultValue = TEXT("true");			
			const FString ConditionName = ApplyOperatorStyle(bInvert ? TEXT("!Equal") : TEXT("Equal"));
			
			if (!bFullTitle)
			{
				return FString::Printf(TEXT("%s{ %d }"), *ConditionName, Conditions.Num());
			}
			
			TArray<FString> Lines;
			for (int32 Index = 0; Index < Conditions.Num(); Index++)
			{
				FString Line = Conditions[Index].IsValid() ? *GetTitleSafe(Conditions[Index].Get(), bFullTitle) : *DefaultValue;
				Lines.Add(Line);
			}
			if (Lines.Num() < 2)
			{
				Lines.Add(DefaultValue);
			}
			if (Lines.Num() < 2)
			{
				Lines.Add(DefaultValue);
			}
			
			return FString::Printf(TEXT("%s{%s}"), *ConditionName, *FString::Join(Lines, TEXT("\n")));
		}
	}
	
	return TEXT("");
}

void UInstancedCondition_LogicOperator::GetSubConditions_Implementation(TArray<UInstancedCondition*>& OutConditions) const
{
	for (auto& Condition : Conditions)
	{
		OutConditions.Add(Condition.Object);
	}
}


bool UInstancedCondition_External::CheckCondition_Implementation(const FInstancedConditionContext& Context)
{
	return Condition ? Condition->Condition.CheckCondition(Context, true) : false;
}

FString UInstancedCondition_External::GetInstancedObjectTitle_Implementation(bool bFullTitle) const
{
	return Condition ? (bInvert ? TEXT("!") : TEXT("")) + GetTitleSafe(Condition->Condition.Object, bFullTitle) : TEXT("None(false)");
}

void UInstancedCondition_External::GetSubConditions_Implementation(TArray<UInstancedCondition*>& OutConditions) const
{
	if (Condition)
	{
		OutConditions.Add(Condition->Condition.Object);
	}
}





