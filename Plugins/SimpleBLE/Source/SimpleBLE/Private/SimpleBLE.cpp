// Copyright Epic Games, Inc. All Rights Reserved.

#include "SimpleBLE.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FSimpleBLEModule"

void FSimpleBLEModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	//const FString BasePluginDir = IPluginManager::Get().FindPlugin("SimpleBLE")->GetBaseDir();

	//const FString LibExamplePath = FPaths::Combine(*BasePluginDir, TEXT("Source/ThirdParty/bin/simpleble.dll"));

	//DynamicLibExampleHandle = FPlatformProcess::GetDllHandle(*LibExamplePath);

	//if (DynamicLibExampleHandle != nullptr)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("simpleble.dll loaded successfully!"));
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Fatal, TEXT("simpleble.dll failed to load!"));
	//}
}

void FSimpleBLEModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	//// we call this function before unloading the module.
	//FPlatformProcess::FreeDllHandle(DynamicLibExampleHandle);
	//DynamicLibExampleHandle = nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSimpleBLEModule, SimpleBLE)