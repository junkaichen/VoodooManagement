// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
THIRD_PARTY_INCLUDES_START
#include "simpleble/SimpleBLE.h"
THIRD_PARTY_INCLUDES_END
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FSimpleBLEModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	//void* DynamicLibExampleHandle;
};
