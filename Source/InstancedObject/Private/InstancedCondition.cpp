// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.


#include "InstancedCondition.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(InstancedCondition)

UObject* FInstancedConditionStruct::Get() const
{
	return Object;
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
	TRACE_CPUPROFILER_EVENT_SCOPE(UInstancedCondition::Check);
	bool bResult = false;
	
	World = GEngine->GetWorldFromContextObject(Context.WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bResult = CheckCondition(Context) ^ (bCanInvert && bInvert);
	World = nullptr;
	
	return bResult;
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
			
			for (const FInstancedConditionStruct Condition : Conditions)
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
			
			for (const FInstancedConditionStruct Condition : Conditions)
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





