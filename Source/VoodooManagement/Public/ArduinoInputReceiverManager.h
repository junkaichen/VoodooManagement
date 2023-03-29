// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArduinoInputReceiverManager.generated.h"

//UDELEGATE(BlueprintCallable)
//DECLARE_DYNAMIC_DELEGATE_RetVal(TSet<AActor*>, FOnGetInputReceiverListDelegate);

UCLASS()
class VOODOOMANAGEMENT_API AArduinoInputReceiverManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AArduinoInputReceiverManager();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void AddToReceiveInputList(AActor* SelfPointer);

	UFUNCTION(BlueprintCallable)
	bool RemoveFromReceiveInputList(AActor* SelfPointer);

	//FOnGetInputReceiverListDelegate GetInputReceiverList;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSet<AActor*> ReceiveInputList;
};
