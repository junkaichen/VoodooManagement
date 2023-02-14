// int buttonPin = 12;
bool Btn1P=false;
int button2Pin = 11;
int button3Pin = 10;
int inputCLK = 4;
int inputDT = 5;

int rightCounter = 0;
int leftCounter = 0;

int previousCounter = 0;

int currentStateCLK;
int previousStateCLK;
String dir = "";

int buttonRead;
int button2Read;
int button3Read;
int dt = 200;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(inputCLK, INPUT);
  pinMode(inputDT, INPUT);
  // pinMode(buttonPin, INPUT);
  pinMode(button2Pin, INPUT);
  pinMode(button3Pin, INPUT);
  previousStateCLK = digitalRead(inputCLK);
}

void loop() {
  // put your main code here, to run repeatedly:
  // buttonRead = digitalRead(buttonPin);
  button2Read = digitalRead(button2Pin);
  button3Read = digitalRead(button3Pin);
  currentStateCLK = digitalRead(inputCLK);

  if(currentStateCLK != previousStateCLK)
  {
      if(digitalRead(inputDT) != currentStateCLK)
      {
        rightCounter = 0;
         if(leftCounter >= 1)
        {
          Serial.print("Left");
          leftCounter = 0;
        }
        else
        {
           leftCounter++;
        }


      }
      else
      {
        leftCounter = 0;
        if(rightCounter <= -1)
        {
          Serial.print("Right");
          rightCounter = 0;
        }
        else
        {
          rightCounter--;
        }
        // Serial.println("Right");
        
      }
  }
  previousStateCLK = currentStateCLK;
 

  // if(buttonRead == 0)
  // {
  //   Serial.println("button1");
  //   delay(dt);
  // }
  if(button2Read == 0)
  {
    Serial.println("button2");
    delay(dt);
  }

  if(button3Read == 0)
  {
   Serial.println("button3");
   delay(dt);
  }
  // delay(dt);


}
