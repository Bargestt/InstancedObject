// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedCondition/InstancedConditionCollection.h"

FInstancedConditionCollectionEntry UInstancedConditionCollection::FindEntryByName(FName Name) const
{
	for (auto& Element : Entries)
	{
		if (Element.Name == Name)
		{
			return Element;
		}
	}
	return {};
}
