// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#include "InstancedObjectModule.h"

#define LOCTEXT_NAMESPACE "FInstancedObjectModule"


DEFINE_LOG_CATEGORY(LogInstancedObject);

void FInstancedObjectModule::StartupModule()
{
	
}

void FInstancedObjectModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInstancedObjectModule, InstancedObject);