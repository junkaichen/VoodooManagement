#include <PDM.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN D10
#define RST_PIN D9
#define SOUND_LIMIT 10000
#define BUTTON_INTERVAL 150

MFRC522 rfid(SS_PIN, RST_PIN);

int button1Pin = D2;
int button2Pin = D3;
int button3Pin = D4;
int button4Pin = D5;
int button5Pin = D6;
int inputCLK = 4;
int inputDT = 5;

int rightCounter = 0;
int leftCounter = 0;

int previousCounter = 0;

int currentStateCLK;
int previousStateCLK;

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

void MyPrint(String output)
{
  if (!hasOutPutted)
  {
    hasOutPutted = true;
  }
  Serial.print(output);
}

void MyPrintln(String output)
{
    if (!hasOutPutted)
  {
    hasOutPutted = true;
  }
  Serial.println(output);
}

void setup() 
{
  Serial.begin(9600);

  // configure the data receive callback
  PDM.onReceive(onPDMdata);

  // initialize PDM with:
  // - one channel (mono mode)
  // - a 16 kHz sample rate
  if (!PDM.begin(1, 16000)) {
    Serial.println("Failed to start PDM!");
  }

  // pinMode(inputCLK, INPUT);
  // pinMode(inputDT, INPUT);
  // previousStateCLK = digitalRead(inputCLK);
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);
  pinMode(button3Pin, INPUT);
  pinMode(button4Pin, INPUT);
  pinMode(button5Pin, INPUT);

  button1LastRead = 1;
  button2LastRead = 1;
  button3LastRead = 1;
  button4LastRead = 1;
  button5LastRead = 1;

  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522

  Serial.println("OK");

}

void loop() 
{
  hasOutPutted = false;

  button1Read = digitalRead(button1Pin);
  button2Read = digitalRead(button2Pin);
  button3Read = digitalRead(button3Pin);
  button4Read = digitalRead(button4Pin);
  button5Read = digitalRead(button5Pin);

  // currentStateCLK = digitalRead(inputCLK);
  // if(currentStateCLK != previousStateCLK)
  // {
  //     if(digitalRead(inputDT) != currentStateCLK)
  //     {
  //       rightCounter = 0;
  //        if(leftCounter >= 1)
  //       {
  //         MyPrintln("RT 0");    // Left
  //         leftCounter = 0;
  //       }
  //       else
  //       {
  //          leftCounter++;
  //       }
  //     }
  //     else
  //     {
  //       leftCounter = 0;
  //       if(rightCounter <= -1)
  //       {
  //         MyPrintln("RT 1");    // Right
  //         rightCounter = 0;
  //       }
  //       else
  //       {
  //         rightCounter--;
  //       }
  //     }
  // }
  // previousStateCLK = currentStateCLK;
 
  unsigned long currentTime = millis();
  
  if(button1Read == 0 && button1LastRead == 1)
  {
    if (currentTime - buttonLastReleaseTime >= BUTTON_INTERVAL)
    {
      MyPrintln("Btn1");
    }
  }

  if(button2Read == 0 && button2LastRead == 1)
  {
    if (currentTime-buttonLastReleaseTime >= BUTTON_INTERVAL)
      MyPrintln("Btn2");
  }

  if(button3Read == 0 && button3LastRead == 1)
   {
    if (currentTime-buttonLastReleaseTime >= BUTTON_INTERVAL)
      MyPrintln("Btn3");
  }

  if(button4Read == 0 && button4LastRead == 1)
   {
    if (currentTime-buttonLastReleaseTime >= BUTTON_INTERVAL)
      MyPrintln("Btn4");
  }

  if(button5Read == 0 && button5LastRead == 1)
   {
    if (currentTime-buttonLastReleaseTime >= BUTTON_INTERVAL)
      MyPrintln("Btn5");
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

  if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    //Serial.println("New Card!");
    if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

      // print NUID in Serial Monitor in the hex format
      MyPrint("UID ");
      for (int i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
        Serial.print(rfid.uid.uidByte[i], HEX);
      }
      Serial.println();
      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }
  }

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
        MyPrintln("Snd");
        break;
      }
    }
    // clear the read count
    samplesRead = 0;
  }

  if (hasOutPutted)
  {
    Serial.println("NL");
  }
}