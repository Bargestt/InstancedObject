// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedObjectUtilityLibrary.h"
#include "InstancedObjectInterface.h"
#include "InstancedObjectStruct.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InstancedObjectUtilityLibrary)

FString FIndentParser::Apply(const FString& InString, int32& OutFinalDepth) const
{
	FString OutString;

	int32 Depth = 0;
	bool bDepthChanged = false;
	for (int32 Index = 0; Index < InString.Len(); Index++)
	{
		int32 CharIndex;
		if (InString[Index] == '\n')
		{
			bDepthChanged = true;
		}
		else if (IncreaseIndentChars.FindChar(InString[Index], CharIndex))
		{
			Depth++;
			bDepthChanged = true;
		}
		else if (DecreaseIndentChars.FindChar(InString[Index], CharIndex))
		{
			Depth--;
			bDepthChanged = true;
		}
		else
		{
			if (bDepthChanged)
			{
				OutString += TEXT("\n") + FString::ChrN(FMath::Max(0, Depth * IndentSize), ' ');
			}
			OutString += InString[Index];			
			bDepthChanged = false;	
		}
	}
	
	OutString.RemoveFromStart(TEXT("\n"));
	OutFinalDepth = Depth;
	return OutString;
}

FString UInstancedObjectUtilityLibrary::GetInstancedObjectTitle(const UObject* Object, bool bFullTitle)
{
	return IInstancedObjectInterface::GetTitleSafe(Object, bFullTitle);
}

FString UInstancedObjectUtilityLibrary::ApplyIndent(const FIndentParser& Parser, const FString& InString, int32& OutDepth)
{
	return Parser.Apply(InString, OutDepth);
}
