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

uint32 AArduinoBLEInputParser::GetButtonsSoundInput(const SimpleBLE::ByteArray& rx_data)
{
    uint32 BitData;
    FMemory::Memcpy(&BitData, &rx_data, sizeof(BitData));
    return BitData;
}

FString AArduinoBLEInputParser::GetRFIDInput(const SimpleBLE::ByteArray& rx_data)
{
    unsigned long Data;
    FMemory::Memcpy(&Data, &rx_data, sizeof(unsigned long));
    FString HexString = FString::Printf(TEXT("%08X"), Data);
    return HexString;
}

TEnumAsByte<EMotionType> AArduinoBLEInputParser::GetAccelerationSensorInput(const SimpleBLE::ByteArray& rx_data)
{
    FVector OutputData;
    static_assert(sizeof(float) == sizeof(uint32_t), "Float and uint32_t size dont match. Check another int type");
    float BitData[3];  // three float
    FMemory::Memcpy(BitData, &rx_data, sizeof(BitData));
    for (int i = 0; i < 3; ++i)
    {
        float AccelerationData;
        FMemory::Memcpy(&AccelerationData, BitData + i, sizeof(float));
        OutputData[i] = AccelerationData;
    }
    TEnumAsByte<EMotionType> Type;
    // DOTO:: Calculate Type of motion
    return Type;
}

void AArduinoBLEInputParser::ProcessAccelerationInput(const SimpleBLE::ByteArray& rx_data)
{
    TEnumAsByte<EMotionType> Type = GetAccelerationSensorInput(rx_data);
    auto ArduinoInputReceiverActorArray = ReceiveInputList;
    for (auto actor : ArduinoInputReceiverActorArray)
    {
        if (!actor->Implements<UArduinoBLEInputInterface>())
            continue;
        IArduinoBLEInputInterface::Execute_AccelerationInput(actor, Type);
    }
}

void AArduinoBLEInputParser::ProcessButtonsSoundInput(const SimpleBLE::ByteArray& rx_data)
{
    uint32 BitData = GetButtonsSoundInput(rx_data);
    auto ArduinoInputReceiverActorArray = ReceiveInputList;
    for (int i = 0; i < 6; i++)
    {
        if (!(BitData | 1 << i)) continue;

        void (* func)(UObject*) = nullptr;
        switch (BitData | 1 << i)
        {
        case 1:
            func = IArduinoBLEInputInterface::Execute_Button1Input;
            break;
        case 2:
            func = IArduinoBLEInputInterface::Execute_Button2Input;
            break;
        case 3:
            func = IArduinoBLEInputInterface::Execute_Button3Input;
            break;
        case 4:
            func = IArduinoBLEInputInterface::Execute_Button4Input;
            break;
        case 5:
            func = IArduinoBLEInputInterface::Execute_Button5Input;
            break;
        case 6:
            func = IArduinoBLEInputInterface::Execute_SoundInput;
            break;
        default:
            break;
        }
        if (func == nullptr) continue;
        for (auto actor : ArduinoInputReceiverActorArray)
        {
            if (!actor->Implements<UArduinoBLEInputInterface>())
                continue;
            func(actor);
        }
    }
}

void AArduinoBLEInputParser::ProcessRFIDInput(const SimpleBLE::ByteArray& rx_data)
{
    FString HexString = GetRFIDInput(rx_data);
    auto ArduinoInputReceiverActorArray = ReceiveInputList;
    for (auto actor : ArduinoInputReceiverActorArray)
    {
        if (!actor->Implements<UArduinoBLEInputInterface>())
            continue;
        IArduinoBLEInputInterface::Execute_RFIDInput(actor, HexString);
    }
}

void AArduinoBLEInputParser::InitBluetooth()
{
    auto adapter_optional = GetAdapter();
    bool found_device = false;

    if (!adapter_optional.has_value()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to find Bluetooth Adapter!"));
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Can't find the Bluetooth Adapter!"));
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

    adapter.set_callback_on_scan_start([]() { UE_LOG(LogTemp, Log, TEXT("Scan started.")); });
    adapter.set_callback_on_scan_stop([]() { UE_LOG(LogTemp, Log, TEXT("Scan stopped.")); });
    // Scan for 5 seconds and return.
    // TODO:: replace the 5000 to variable
    adapter.scan_for(5000);

    if (!found_device)
    {
        bIsConnected = false;
        UE_LOG(LogTemp, Warning, TEXT("Failed to find Bluetooth!"));
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Can't find the Arduino!"));
        return;
    }
    TargetPeripheral.connect();
    UE_LOG(LogTemp, Log, TEXT("Connected to Arduino BLE."));

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

    UE_LOG(LogTemp, Log, TEXT("Finished Initializing Bluetooth."));

    //if (bIsReceivingAccelerationInput)
    // {
    //    target_peripheral.notify(AccelerationUUID.Key, AccelerationUUID.Value, [&](SimpleBLE::ByteArray rx_data)
    //        {ProcessAccelerationInput(rx_data); });
    // }
}

void AArduinoBLEInputParser::AddToReceiveInputList(AActor* SelfPointer)
{
    if (SelfPointer)
        ReceiveInputList.Add(SelfPointer);
}

bool AArduinoBLEInputParser::RemoveFromReceiveInputList(AActor* SelfPointer)
{
    if (SelfPointer && ReceiveInputList.Contains(SelfPointer))
    {
        ReceiveInputList.Remove(SelfPointer);
        return true;
    }
    return false;
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
    AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this]() {
        this->InitBluetooth();
        });
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