// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#include "InstancedObjects.h"

#define LOCTEXT_NAMESPACE "FInstancedObjectsModule"

void FInstancedObjectsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FInstancedObjectsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInstancedObjectsModule, InstancedObjects)