// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedEvent/InstancedEventCollection.h"

FInstancedEventCollectionEntry UInstancedEventCollection::FindEntryByName(FName Name) const
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
