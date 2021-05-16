// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class OpenCL : ModuleRules
{
	public OpenCL(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
			}
		);


		PrivateIncludePaths.AddRange(
			new string[]
			{
				"OpenCL/Private"
			}
		);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"RenderCore",
				"RHI"
			}
		);


		string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "Win64" : "Win32";
		string OpenCLLibrariesPath = Path.Combine(ModuleDirectory, "Lib");
		string NvidiaLibrariesPath = Path.Combine(OpenCLLibrariesPath, "NVIDIA", PlatformString);
		string IntelLibrariesPath = Path.Combine(OpenCLLibrariesPath, "Intel", PlatformString);
		string AmdLibrariesPath = Path.Combine(OpenCLLibrariesPath, "AMD", PlatformString);
		if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Win32)
		{
			PublicAdditionalLibraries.Add(Path.Combine(NvidiaLibrariesPath, "OpenCL.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(IntelLibrariesPath, "OpenCL.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(AmdLibrariesPath, "OpenCL.lib"));
		}
	}
}