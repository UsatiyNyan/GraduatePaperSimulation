// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

#include "CL/opencl.h"

#include <string>

#include "OCLData.h"

DECLARE_LOG_CATEGORY_EXTERN(LogOpenCL, All, All);

class OpenCLPlugin : public IModuleInterface
{
public:
	static inline OpenCLPlugin& Get()
	{
		return FModuleManager::LoadModuleChecked<OpenCLPlugin>("OpenCL");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("OpenCL");
	}

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void EnumerateDevices(TArray<FOpenCLDeviceData>& OutDevices, bool bForceRefresh = false);
	void RunKernelOnDevices(const FString& KernelString,
	                        const FString& KernelName,
	                        const FString& Args,
	                        TFunction<void(const FString&, bool)> ResultCallback,
	                        const TArray<FOpenCLDeviceData>& OutDevices);
private:
	void FreeDeviceMemory();

private:
	TArray<FOpenCLDeviceData> Devices;
	TArray<cl_device_id*> DeviceIdsMemoryList;
	bool bHasEnumeratedOnce{};
};
