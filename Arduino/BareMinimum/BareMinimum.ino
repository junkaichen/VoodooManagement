int button2Pin = 10;
int button3Pin = 11;
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
int dt = 200;

bool hasOutPutted;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(inputCLK, INPUT);
  pinMode(inputDT, INPUT);
  // pinMode(buttonPin, INPUT);
  pinMode(button2Pin, INPUT);
  pinMode(button3Pin, INPUT);
  previousStateCLK = digitalRead(inputCLK);
  button1LastRead = 1;
  button2LastRead = 1;
  button3LastRead = 1;
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

void loop() {
  // put your main code here, to run repeatedly:
  // buttonRead = digitalRead(buttonPin);
  hasOutPutted = false;

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
          MyPrintln("RT 0");    // Left
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
          MyPrintln("RT 1");    // Right
          rightCounter = 0;
        }
        else
        {
          rightCounter--;
        }
      }
  }
  previousStateCLK = currentStateCLK;
 

  // if(button1Read == 0 && button1LastRead == 1)
  // {
  //   Serial.println("btn1");
  //   delay(dt);
  // }
  if(button2Read == 0 && button2LastRead == 1)
  {
   MyPrintln("Btn2");
    //delay(dt);
  }

  if(button3Read == 0 && button3LastRead == 1)
  {
   MyPrintln("Btn3");
   //delay(dt);
  }
  button1LastRead = button1Read;
  button2LastRead = button2Read;
  button3LastRead = button3Read;

  // delay(dt);
  if (hasOutPutted)
  {
    Serial.println("NL");
  }
}
