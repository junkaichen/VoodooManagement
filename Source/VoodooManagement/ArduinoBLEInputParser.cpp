// Fill out your copyright notice in the Description page of Project Settings.
#include "ArduinoBLEInputParser.h"
#include <optional>
#include "UObject/WeakInterfacePtr.h"
#include "Kismet/GameplayStatics.h"
#include "ArduinoBLEInputInterface.h"

std::optional<SimpleBLE::Adapter> AArduinoBLEInputParser::GetAdapter()
{
    if (!SimpleBLE::Adapter::bluetooth_enabled()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("Bluetooth is not enabled!"));
        return {};
    }

    auto adapterList = SimpleBLE::Adapter::get_adapters();

    // no adapter found
    if (adapterList.empty()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("No adapter was found."));
        return {};
    }
    auto adapter = adapterList.at(0);
    UE_LOG(LogTemp, Log, TEXT("Using adapter: %s [%s]"), *FString(adapter.identifier().c_str()), *FString(adapter.address().c_str()));
    return adapter;
}

void AArduinoBLEInputParser::ProcessAccelerationInput(const SimpleBLE::ByteArray& rx_data)
{
    TEnumAsByte<EMotionType> Type = GetAccelerationSensorInput(rx_data);
    TArray<AActor*> ArduinoInputReceiverActorArray;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UArduinoBLEInputInterface::StaticClass(), ArduinoInputReceiverActorArray);
    for (auto actor : ArduinoInputReceiverActorArray)
    {
        TWeakInterfacePtr<IArduinoBLEInputInterface> ArduinoInputReceiverActor = Cast<IArduinoBLEInputInterface>(actor);
        if (ArduinoInputReceiverActor.IsValid())
        {
            ArduinoInputReceiverActor->AccelerationInput_Implementation(Type);
        }
    }
}

void AArduinoBLEInputParser::ProcessButtonsSoundInput(const SimpleBLE::ByteArray& rx_data)
{
    uint32 BitData = GetButtonsSoundInput(rx_data);
    TArray<AActor*> ArduinoInputReceiverActorArray;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UArduinoBLEInputInterface::StaticClass(), ArduinoInputReceiverActorArray);
    for (int i = 0; i < 6; i++)
    {
        if (!(BitData | 1 << i)) continue;

        void (IArduinoBLEInputInterface:: * func)(void) = nullptr;
        switch (i)
        {
        case 1:
            func = &IArduinoBLEInputInterface::Button1Input_Implementation;
            break;
        case 2:
            func = &IArduinoBLEInputInterface::Button2Input_Implementation;
            break;
        case 3:
            func = &IArduinoBLEInputInterface::Button3Input_Implementation;
            break;
        case 4:
            func = &IArduinoBLEInputInterface::Button4Input_Implementation;
            break;
        case 5:
            func = &IArduinoBLEInputInterface::Button5Input_Implementation;
            break;
        case 6:
            func = &IArduinoBLEInputInterface::SoundInput_Implementation;
            break;
        default:
            break;
        }
        if (func == nullptr) continue;
        for (auto actor : ArduinoInputReceiverActorArray)
        {
            TWeakInterfacePtr<IArduinoBLEInputInterface> ArduinoInputReceiverActor = Cast<IArduinoBLEInputInterface>(actor);
            if (ArduinoInputReceiverActor.IsValid())
            {
                (ArduinoInputReceiverActor.Get()->*func)();
            }
        }
    }
}

void AArduinoBLEInputParser::ProcessRFIDInput(const SimpleBLE::ByteArray& rx_data)
{
    FString HexString = GetRFIDInput(rx_data);
    TArray<AActor*> ArduinoInputReceiverActorArray;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UArduinoBLEInputInterface::StaticClass(), ArduinoInputReceiverActorArray);
    for (auto actor : ArduinoInputReceiverActorArray)
    {
        TWeakInterfacePtr<IArduinoBLEInputInterface> ArduinoInputReceiverActor = Cast<IArduinoBLEInputInterface>(actor);
        if (ArduinoInputReceiverActor.IsValid())
        {
            ArduinoInputReceiverActor->RFIDInput_Implementation(HexString);
        }
    }
}

TEnumAsByte<EMotionType> AArduinoBLEInputParser::GetAccelerationSensorInput(const SimpleBLE::ByteArray& rx_data)
{
    FVector OutputData;
    static_assert(sizeof(float) == sizeof(uint32_t), "Float and uint32_t size dont match. Check another int type");
    float BitData[3];  // three float
    memcpy(BitData, &rx_data, sizeof(BitData));
    for (int i = 0; i < 3; ++i)
    {
        float AccelerationData;
        memcpy(&AccelerationData, BitData+i, sizeof(float));
        OutputData[i] = AccelerationData;
    }
    TEnumAsByte<EMotionType> Type;
    // DOTO:: Calculate Type of motion
    return Type;
}

uint32 AArduinoBLEInputParser::GetButtonsSoundInput(const SimpleBLE::ByteArray& rx_data)
{
    uint32 BitData;
    memcpy(&BitData, &rx_data, sizeof(BitData));
    return BitData;
}

FString AArduinoBLEInputParser::GetRFIDInput(const SimpleBLE::ByteArray& rx_data)
{
    unsigned long Data;
    memcpy(&Data, &rx_data, sizeof(unsigned long));
    FString HexString = FString::Printf(TEXT("%08X"), Data);
    return HexString;
}

// Sets default values
AArduinoBLEInputParser::AArduinoBLEInputParser()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    bIsConnected = false;
    bIsReceivingRFIDInput = bIsReceivingButtonSoundInput = bIsReceivingAccelerationInput = false;
}

// Called when the game starts or when spawned
void AArduinoBLEInputParser::BeginPlay()
{
	Super::BeginPlay();
    auto adapter_optional = GetAdapter();
    bool found_device = false;

    if (!adapter_optional.has_value()) {
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Can't find the Arduino!"));
        return;
    }

    auto adapter = adapter_optional.value();

    std::vector<SimpleBLE::Peripheral> peripherals;

    adapter.set_callback_on_scan_found([&](SimpleBLE::Peripheral peripheral) {
        if (peripheral.address() == TCHAR_TO_UTF8(*FNano33Mac)) {
            adapter.scan_stop();
            TargetPeripheral = peripheral;
            found_device = true;
        }});

    adapter.set_callback_on_scan_start([]() { std::cout << "Scan started." << std::endl; });
    adapter.set_callback_on_scan_stop([]() { std::cout << "Scan stopped." << std::endl; });
    // Scan for 5 seconds and return.
    // TODO:: replace the 5000 to variable
    adapter.scan_for(5000);

    if (!found_device)
    {
        bIsConnected = false;
        return;
    }
    TargetPeripheral.connect();

    // Store all service and characteristic uuids in a vector.
    TArray<TPair<SimpleBLE::BluetoothUUID, SimpleBLE::BluetoothUUID>> uuids;
    for (auto service : TargetPeripheral.services()) {
        for (auto characteristic : service.characteristics()) {
            uuids.Add(TPair<SimpleBLE::BluetoothUUID, SimpleBLE::BluetoothUUID>(service.uuid(), characteristic.uuid()));
        }
    }

    // std::cout << "The following services and characteristics were found:" << std::endl;
    for (size_t i = 0; i < uuids.Num(); i++)
    {
        if (uuids[i].Value == TCHAR_TO_UTF8(*FRfidCharacteristicUUID))
        {
            bIsReceivingRFIDInput = true;
            RfidUUID = uuids[i];
        }
        if (uuids[i].Value == TCHAR_TO_UTF8(*FButtonSoundCharacteristicUUID))
        {
            bIsReceivingButtonSoundInput = true;
            ButtonSoundUUID = uuids[i];
        }
        if (uuids[i].Value == TCHAR_TO_UTF8(*FAccelerationCharacteristicUUID))
        {
            bIsReceivingAccelerationInput = true;
            AccelerationUUID = uuids[i];
        }
    }

    // Subscribe to the characteristic.
    if (bIsReceivingRFIDInput)
    {
        TargetPeripheral.notify(RfidUUID.Key, RfidUUID.Value, [&](SimpleBLE::ByteArray rx_data)
            {ProcessRFIDInput(rx_data); });
    }

    if (bIsReceivingButtonSoundInput)
    {
        TargetPeripheral.notify(ButtonSoundUUID.Key, ButtonSoundUUID.Value, [&](SimpleBLE::ByteArray rx_data)
            {ProcessButtonsSoundInput(rx_data); });
    }
    while (1);
    //if (bIsReceivingAccelerationInput)
    // {
    //    target_peripheral.notify(AccelerationUUID.Key, AccelerationUUID.Value, [&](SimpleBLE::ByteArray rx_data)
    //        {ProcessAccelerationInput(rx_data); });
    // }
}

void AArduinoBLEInputParser::Destroyed()
{
    if (bIsReceivingButtonSoundInput)
        TargetPeripheral.unsubscribe(RfidUUID.Key, RfidUUID.Value);
    if (bIsReceivingButtonSoundInput)
        TargetPeripheral.unsubscribe(ButtonSoundUUID.Key, ButtonSoundUUID.Value);
    /* if (bIsReceivingAccelerationInput)
         TargetPeripheral.unsubscribe(AccelerationUUID.Key, AccelerationUUID.Value);*/
    TargetPeripheral.disconnect();
}

// Called every frame
void AArduinoBLEInputParser::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

