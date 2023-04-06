#include <Arduino_BMI270_BMM150.h>
#include <ArduinoBLE.h>
#include <PDM.h>
#include <SPI.h>
#include <MFRC522.h>


// Pin Settings
#define SS_PIN D10
#define RST_PIN D9
#define BUTTON1_PIN D2
#define BUTTON2_PIN D3
#define BUTTON3_PIN D4
#define BUTTON4_PIN D5
#define BUTTON5_PIN D6
#define BUTTON1_INPUT 1
#define BUTTON2_INPUT 1<<1
#define BUTTON3_INPUT 1<<2
#define BUTTON4_INPUT 1<<3
#define BUTTON5_INPUT 1<<4
#define SOUND_INPUT 1<<5

MFRC522 rfid(SS_PIN, RST_PIN);
void(* resetFunc) (void) = 0;

// General Settings
#define SOUND_LIMIT 8000
#define BUTTON_INTERVAL 150
// int rightCounter = 0;
// int leftCounter = 0;

// int previousCounter = 0;

// int currentStateCLK;
// int previousStateCLK;

int button1Read, button1LastRead;
int button2Read, button2LastRead;
int button3Read, button3LastRead;
int button4Read, button4LastRead;
int button5Read, button5LastRead;
unsigned long buttonLastReleaseTime = 0;
bool hasOutPutted;


// buffer to read samples into, each sample is 16-bits
short sampleBuffer[256];

// number of samples read
volatile int samplesRead;

void onPDMdata() 
{
  // query the number of bytes available
  int bytesAvailable = PDM.available();

  // read into the sample buffer
  PDM.read(sampleBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
}

//----------------------------------------------------------------------------------------------------------------------
// BLE
//----------------------------------------------------------------------------------------------------------------------

#define BLE_UUID_TEST_SERVICE               "9A48ECBA-2E92-082F-C079-9E75AAE428B1"
#define BLE_UUID_ACCELERATION               "ba118772-c36d-494a-a8e0-c0cc9f569b89"
#define BLE_UUID_RFID                       "9d6e6653-fe77-449d-a1c9-58061a811483"
#define BLE_UUID_BUTTON                     "8cb974de-1f87-4f2f-9942-ac1d421fa34d"
#define BLE_UUID_RESET                      "5fbd9f9b-bcd3-4d02-a2f7-0acdab89e8f0"
#define NUMBER_OF_SENSORS 3

union multi_sensor_data
{
  struct __attribute__( ( packed ) )
  {
    float values[NUMBER_OF_SENSORS];
  };
  uint8_t bytes[ NUMBER_OF_SENSORS * sizeof( float ) ];
};

union multi_sensor_data sensor_data;

BLEService testService( BLE_UUID_TEST_SERVICE );
// BLECharacteristic accelerationCharacteristic( BLE_UUID_ACCELERATION, BLERead | BLENotify, sizeof sensor_data.bytes);
BLECharacteristic rfidCharacteristic(BLE_UUID_RFID, BLERead| BLENotify, 4);
BLEUnsignedIntCharacteristic buttonCharacteristic( BLE_UUID_BUTTON, BLERead | BLENotify );
BLEBoolCharacteristic resetCharacteristic(BLE_UUID_RESET, BLEWrite);
void setup()
{
  Serial.begin( 9600 );

  // configure the data receive callback
  PDM.onReceive(onPDMdata);

  // initialize PDM with:
  // - one channel (mono mode)
  // - a 16 kHz sample rate
  if (!PDM.begin(1, 16000)) {
    Serial.println("Failed to start PDM!");
  }

  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522

  if ( !IMU.begin() )
  {
    Serial.println( "Failed to initialize IMU!" );
    while ( 1 );
  }
  if (!setupBleMode())
  {
    Serial.println("Failed to initialize BLE!");
  }

  // pinMode(inputCLK, INPUT);
  // pinMode(inputDT, INPUT);
  // previousStateCLK = digitalRead(inputCLK);
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
  pinMode(BUTTON3_PIN, INPUT);
  pinMode(BUTTON4_PIN, INPUT);
  pinMode(BUTTON5_PIN, INPUT);

  button1LastRead = 1;
  button2LastRead = 1;
  button3LastRead = 1;
  button4LastRead = 1;
  button5LastRead = 1;

  Serial.println("OK");
}


void loop()
{
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  if ( central )
  {
    Serial.print( "Connected to central: " );
    Serial.println( central.address() );

    while ( central.connected() )
    {
      if (resetCharacteristic.written())      // reset Arduino
      {
        if (resetCharacteristic.value())
          resetFunc();
      }


      static long previousMillis = 0;
      unsigned int buttonSoundInput = 0;

      long interval = 20;
      unsigned long currentMillis = millis();
      if( currentMillis - previousMillis > interval )
      {
        previousMillis = currentMillis;

        if( central.rssi() != 0 )
        {
          button1Read = digitalRead(BUTTON1_PIN);
          button2Read = digitalRead(BUTTON2_PIN);
          button3Read = digitalRead(BUTTON3_PIN);
          button4Read = digitalRead(BUTTON4_PIN);
          button5Read = digitalRead(BUTTON5_PIN);

          if(button1Read == 0 && button1LastRead == 1)
          {
            if (currentMillis - buttonLastReleaseTime >= BUTTON_INTERVAL)
            {
              buttonSoundInput |= BUTTON1_INPUT;
            }
          }

          if(button2Read == 0 && button2LastRead == 1)
          {
            if (currentMillis-buttonLastReleaseTime >= BUTTON_INTERVAL)
              buttonSoundInput |= BUTTON2_INPUT;
          }

          if(button3Read == 0 && button3LastRead == 1)
          {
            if (currentMillis-buttonLastReleaseTime >= BUTTON_INTERVAL)
              buttonSoundInput |= BUTTON3_INPUT;
          }

          if(button4Read == 0 && button4LastRead == 1)
          {
            if (currentMillis-buttonLastReleaseTime >= BUTTON_INTERVAL)
              buttonSoundInput |= BUTTON4_INPUT;
          }

          if(button5Read == 0 && button5LastRead == 1)
          {
            if (currentMillis-buttonLastReleaseTime >= BUTTON_INTERVAL)
              buttonSoundInput |= BUTTON5_INPUT;
          }

          if ((button1Read == 1 && button1LastRead == 0)||(button2Read == 1 && button2LastRead == 0) ||(button3Read == 1 && button3LastRead == 0) ||(button4Read == 1 && button4LastRead == 0)||(button5Read == 1 && button5LastRead == 0))
          {
            buttonLastReleaseTime = millis();
          }

          button1LastRead = button1Read;
          button2LastRead = button2Read;
          button3LastRead = button3Read;
          button4LastRead = button4Read;
          button5LastRead = button5Read;

          // Sound Sensor
          // wait for samples to be read
          if (samplesRead) 
          {

            // print samples to the serial monitor or plotter

            for (int i = 0; i < samplesRead; i++) 
            {
              //Serial.println(sampleBuffer[i]);
              // check if the sound value is higher than 500
              if (sampleBuffer[i]>=SOUND_LIMIT)
              {
                buttonSoundInput |= SOUND_INPUT;
                break;
              }
            }
            // clear the read count
            samplesRead = 0;
          }

          if (buttonSoundInput != 0)
            buttonCharacteristic.writeValue(buttonSoundInput);

          if (rfid.PICC_IsNewCardPresent()) 
          { // new tag is available
            //Serial.println("New Card!");
            if (rfid.PICC_ReadCardSerial()) 
            { // NUID has been readed
              MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
              unsigned long rfidInput = 0;
              // make NUID in the hex format
              for (int i = 0; i < rfid.uid.size; i++) 
              {
                rfidInput = rfidInput<<8 | (unsigned long) rfid.uid.uidByte[i];
              }
              // Serial.print(rfidInput, HEX);
              rfidCharacteristic.writeValue(rfidInput);

              // Serial.println();
              rfid.PICC_HaltA(); // halt PICC
              //rfid.PCD_StopCrypto1(); // stop encryption on PCD
              
            }
          }

          // Acceleration input
          // float accelerationX, accelerationY, accelerationZ;
          // if (IMU.accelerationAvailable())
          // {
          //   IMU.readAcceleration( sensor_data.values[0], sensor_data.values[1], sensor_data.values[2] );
          //   accelerationCharacteristic.writeValue( sensor_data.bytes, sizeof sensor_data.bytes);
          // }
        }
      } // intervall
    } // while connected

    Serial.print( F( "Disconnected from central: " ) );
    Serial.println( central.address() );
  } // if central
} // loop



bool setupBleMode()
{
  if ( !BLE.begin() )
  {
    return false;
  }

  // set advertised local name and service UUID:
  BLE.setDeviceName( "Nano33" );
  BLE.setLocalName( "Nano33" );
  BLE.setAdvertisedService( testService );

  // BLE add characteristics
  //testService.addCharacteristic( accelerationCharacteristic );
  testService.addCharacteristic( buttonCharacteristic );
  testService.addCharacteristic(rfidCharacteristic);
  testService.addCharacteristic(resetCharacteristic);

  // add service
  BLE.addService( testService );

  // set the initial value for the characeristic:
  for (int i=0;i<NUMBER_OF_SENSORS;++i)
  {
    sensor_data.values[i] = 0;
  }
  //accelerationCharacteristic.writeValue( sensor_data.bytes, sizeof sensor_data.bytes );
  rfidCharacteristic.writeValue((byte)0x0);
  buttonCharacteristic.writeValue( 0 );
  resetCharacteristic.writeValue(false);
  // start advertising
  BLE.advertise();

  return true;
}