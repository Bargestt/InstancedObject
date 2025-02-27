// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

using UnrealBuildTool;

public class InstancedObject : ModuleRules
{
	public InstancedObject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"StructUtils",
				"GameplayTags"
			}
			);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
			}
			);
	}
}
