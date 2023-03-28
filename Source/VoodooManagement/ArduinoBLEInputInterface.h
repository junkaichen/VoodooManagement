// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ArduinoBLEInputInterface.generated.h"

UENUM(BlueprintType)
enum EMotionType
{
	up UMETA(DisplayName = "up"),
	down UMETA(DisplayName = "down"),
	left UMETA(DisplayName = "left"),
	right UMETA(DisplayName = "right"),
};

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UArduinoBLEInputInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class VOODOOMANAGEMENT_API IArduinoBLEInputInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Button1Input();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Button2Input();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Button3Input();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Button4Input();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Button5Input();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SoundInput();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RFIDInput(const FString& ID);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void AccelerationInput(TEnumAsByte<EMotionType>& Type);
};
