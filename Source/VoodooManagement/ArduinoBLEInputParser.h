// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimpleBLE.h"
#include "ArduinoBLEInputParser.generated.h"

enum EMotionType;

UCLASS()
class VOODOOMANAGEMENT_API AArduinoBLEInputParser : public AActor
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsConnected;
private:
	const FString FNano33Mac = "15:5b:49:e7:17:27";
	const FString FAccelerationCharacteristicUUID = "ba118772-c36d-494a-a8e0-c0cc9f569b89";
	const FString FRfidCharacteristicUUID = "9d6e6653-fe77-449d-a1c9-58061a811483";
	const FString FButtonSoundCharacteristicUUID = "8cb974de-1f87-4f2f-9942-ac1d421fa34d";

	SimpleBLE::Peripheral TargetPeripheral;
	bool bIsReceivingRFIDInput, bIsReceivingButtonSoundInput, bIsReceivingAccelerationInput;
	TPair<SimpleBLE::BluetoothUUID, SimpleBLE::BluetoothUUID> RfidUUID, ButtonSoundUUID, AccelerationUUID;
public:	
	// Sets default values for this actor's properties
	AArduinoBLEInputParser();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

private:
	std::optional<SimpleBLE::Adapter> GetAdapter();
	void ProcessAccelerationInput(const SimpleBLE::ByteArray& rx_data);
	void ProcessButtonsSoundInput(const SimpleBLE::ByteArray& rx_data);
	void ProcessRFIDInput(const SimpleBLE::ByteArray& rx_data);
	TEnumAsByte<EMotionType> GetAccelerationSensorInput(const SimpleBLE::ByteArray& rx_data);
	// Returns a bit array as uint32, repesent button1,2,3,4,5 and sound
	uint32 GetButtonsSoundInput(const SimpleBLE::ByteArray& rx_data);
	FString GetRFIDInput(const SimpleBLE::ByteArray& rx_data);
};
