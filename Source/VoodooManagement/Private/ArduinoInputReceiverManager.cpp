// Fill out your copyright notice in the Description page of Project Settings.


#include "ArduinoInputReceiverManager.h"

// Sets default values
AArduinoInputReceiverManager::AArduinoInputReceiverManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AArduinoInputReceiverManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AArduinoInputReceiverManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AArduinoInputReceiverManager::AddToReceiveInputList(AActor* SelfPointer)
{
	if (SelfPointer)
		ReceiveInputList.Add(SelfPointer);
}

bool AArduinoInputReceiverManager::RemoveFromReceiveInputList(AActor* SelfPointer)
{
	if (SelfPointer && ReceiveInputList.Contains(SelfPointer))
	{
		ReceiveInputList.Remove(SelfPointer);
		return true;
	}
	return false;
}

