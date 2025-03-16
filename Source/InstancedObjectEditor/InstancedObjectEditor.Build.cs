// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

using UnrealBuildTool;

public class InstancedObjectEditor : ModuleRules
{
    public InstancedObjectEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        OptimizeCode = CodeOptimization.Never;
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "InstancedObject"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "InputCore",
                "Slate",
                "SlateCore", 
                "PropertyEditor",
                "UnrealEd"
            }
        );
    }
}