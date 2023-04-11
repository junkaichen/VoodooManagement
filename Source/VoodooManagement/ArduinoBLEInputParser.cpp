// Fill out your copyright notice in the Description page of Project Settings.
#include "ArduinoBLEInputParser.h"
#include "UObject/WeakInterfacePtr.h"
#include "Kismet/GameplayStatics.h"
#include "ArduinoBLEInputInterface.h"

// Sets default values
AArduinoBLEInputParser::AArduinoBLEInputParser()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    bIsConnected = false;
    bIsReceivingRFIDInput = bIsReceivingButtonSoundInput = bIsReceivingAccelerationInput = false;
    ConfigFileName = "Config.json";
}

TOptional<SimpleBLE::Safe::Adapter> AArduinoBLEInputParser::GetAdapter()
{
    if (!SimpleBLE::Safe::Adapter::bluetooth_enabled()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("Bluetooth is not enabled!"));
        return {};
    }

    auto adapterList = SimpleBLE::Safe::Adapter::get_adapters();

    // no adapter found
    if (adapterList->empty()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("No adapter was found."));
        return {};
    }
    auto adapter = adapterList->at(0);
    UE_LOG(LogTemp, Log, TEXT("Using adapter: %s [%s]"), *FString(adapter.identifier().value().c_str()), *FString(adapter.address().value().c_str()));
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
    RemoveAllInvaildPointer();
    for (auto actor : ReceiveInputObjectList)
    {
        if (!actor || !actor->Implements<UArduinoBLEInputInterface>())
            continue;
        IArduinoBLEInputInterface::Execute_AccelerationInput(actor, Type);
    }
}

void AArduinoBLEInputParser::ProcessButtonsSoundInput(const SimpleBLE::ByteArray& rx_data)
{
    uint32 BitData = GetButtonsSoundInput(rx_data);
    for (int i = 0; i < 6; i++)
    {
        if (!(BitData & 1 << i)) continue;

        void (* func)(UObject*) = nullptr;
        switch (i)
        {
        case 0:
            func = IArduinoBLEInputInterface::Execute_Button1Input;
            break;
        case 1:
            func = IArduinoBLEInputInterface::Execute_Button2Input;
            break;
        case 2:
            func = IArduinoBLEInputInterface::Execute_Button3Input;
            break;
        case 3:
            func = IArduinoBLEInputInterface::Execute_Button4Input;
            break;
        case 4:
            func = IArduinoBLEInputInterface::Execute_Button5Input;
            break;
        case 5:
            func = IArduinoBLEInputInterface::Execute_SoundInput;
            break;
        default:
            break;
        }
        if (func == nullptr) continue;
        RemoveAllInvaildPointer();
        for (auto actor : ReceiveInputObjectList)
        {
            if (!actor || !actor->Implements<UArduinoBLEInputInterface>())
                continue;
            AsyncTask(ENamedThreads::GameThread, [func, actor]() {
                func(actor);
                });
        }
    }
}

void AArduinoBLEInputParser::ProcessRFIDInput(const SimpleBLE::ByteArray& rx_data)
{
    FString HexString = GetRFIDInput(rx_data);
    RemoveAllInvaildPointer();
    for (auto actor : ReceiveInputObjectList)
    {
        if (!actor || !actor->Implements<UArduinoBLEInputInterface>())
            continue;
        if (UIDToNameMap.Contains(HexString.ToUpper()))
            AsyncTask(ENamedThreads::GameThread, [actor, HexString, this]() {
            IArduinoBLEInputInterface::Execute_RFIDInput(actor, UIDToNameMap[HexString]);
                });
    }
}

void AArduinoBLEInputParser::InitBluetooth()
{
    auto adapter_optional = GetAdapter();
    bool found_device = false;

    if (!adapter_optional.IsSet())
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to find Bluetooth Adapter!"));
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Can't find the Bluetooth Adapter!"));
        return;
    }

    if (!adapter_optional.IsSet())
        return;
    auto adapter = adapter_optional.GetValue();

    std::vector<SimpleBLE::Safe::Peripheral> peripherals;

    adapter.set_callback_on_scan_found([&](SimpleBLE::Safe::Peripheral peripheral) {
        if (peripheral.is_connectable() && peripheral.address() == TCHAR_TO_UTF8(*FNano33Mac)) {
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
    auto Services = TargetPeripheral.services().value();
    for (auto service : Services)
    {
        for (auto characteristic : service.characteristics()) 
        {
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
        //if (uuids[i].Value == TCHAR_TO_UTF8(*FAccelerationCharacteristicUUID))
        //{
        //    bIsReceivingAccelerationInput = true;
        //    AccelerationUUID = uuids[i];
        //}
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

bool AArduinoBLEInputParser::ReadJsonConfigFile()
{
    FString ConfigFilePath = FPaths::ProjectConfigDir().Append(ConfigFileName);
    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*ConfigFilePath))
        return false;

    FString RawJson;
    FFileHelper::LoadFileToString(RawJson, *ConfigFilePath);
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<TCHAR>::Create(RawJson);
    TSharedPtr<FJsonObject> JsonObject;
    FJsonSerializer::Deserialize(JsonReader, JsonObject);
    auto UIDToJsonMap = JsonObject->GetObjectField(TEXT("UID"))->Values;
    for (auto UID : UIDToJsonMap)
    {
        UIDToNameMap.Add(UID.Key.ToUpper(), UID.Value->AsString());
    }
    return true;
}

void AArduinoBLEInputParser::RemoveAllInvaildPointer()
{
    for (auto pointer : ReceiveInputObjectList)
    {
        if (!pointer)
            ReceiveInputObjectList.Remove(pointer);
    }
}

void AArduinoBLEInputParser::AddToReceiveInputObjectList(AActor* SelfPointer)
{
    if (SelfPointer && SelfPointer->Implements<UArduinoBLEInputInterface>())
        ReceiveInputObjectList.Add(SelfPointer);
}

bool AArduinoBLEInputParser::RemoveFromReceiveObjectInputList(AActor* SelfPointer)
{
    RemoveAllInvaildPointer();
    if (ReceiveInputObjectList.Contains(SelfPointer))
    {
        ReceiveInputObjectList.Remove(SelfPointer);
        return true;
    }
    return false;
}

// Called when the game starts or when spawned
void AArduinoBLEInputParser::BeginPlay()
{
	Super::BeginPlay();
    ReadJsonConfigFile();
    AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this]() {
        this->InitBluetooth();
        });
}

void AArduinoBLEInputParser::BeginDestroy()
{
    Super::BeginDestroy();
    if (bIsReceivingButtonSoundInput)
        TargetPeripheral.unsubscribe(RfidUUID.Key, RfidUUID.Value);
    if (bIsReceivingButtonSoundInput)
        TargetPeripheral.unsubscribe(ButtonSoundUUID.Key, ButtonSoundUUID.Value);
    /* if (bIsReceivingAccelerationInput)
         TargetPeripheral.unsubscribe(AccelerationUUID.Key, AccelerationUUID.Value);*/
    if (TargetPeripheral.is_connected())
        TargetPeripheral.disconnect();
}
 
// Called every frame
void AArduinoBLEInputParser::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
