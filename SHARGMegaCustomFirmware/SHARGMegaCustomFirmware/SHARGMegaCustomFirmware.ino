/****************************************************************************************
**  This is example LINX firmware for use with the Arduino Uno with the serial
**  interface enabled.
**
**  For more information see:           www.labviewmakerhub.com/linx
**  For support visit the forums at:    www.labviewmakerhub.com/forums/linx
**
**  Written By Sam Kristoff
**
**  BSD2 License.
****************************************************************************************/
//Include All Peripheral Libraries Used By LINX
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Servo.h>
#include <LiquidCrystal.h>
#include <DHT.h>
#include <Unistep2.h>
#include <Keypad.h>
//Include Device Specific Header From Sketch>>Import Library (In This Case LinxChipkitMax32.h)
//Also Include Desired LINX Listener From Sketch>>Import Library (In This Case LinxSerialListener.h)
//#include <LinxArduinoUno.h>
#include <LinxArduinoMega2560.h>
#include <LinxSerialListener.h>
#define DHT11_PIN 3
#define DHTTYPE DHT11
// initialize the library with the numbers of the interface pins
//LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
DHT dht(DHT11_PIN, DHTTYPE);
Unistep2 steppermotor(51, 47, 49, 53, 2048, 50);  //Full Step

const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

//byte scorePin[8] = {4, 5, 6, 7, 52, 46, 48, 50}; //Pins for score 7seg
byte scorePin[8] = {52, 46, 48, 50, 4, 5, 6, 7}; //Pins for score 7seg

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

//const int dataPin = 8;
//const int latchPin = 9;
//const int clockPin = 10;
const int dataPin2 = 12;
const int latchPin2 = 11;
const int clockPin2 = 9;
const int dataPin = 22;
const int latchPin = 24;
const int clockPin =26;

//Create A Pointer To The LINX Device Object We Instantiate In Setup()
LinxArduinoMega2560* LinxDevice;
// Custom Functions for LabVIEW
int setLCDCursor();
int numLCD();
int textLCD();
int clearLCD();
int dht1();
int stepper_motor();
int sevenSeg();
int scoreSevenSeg();
int timeSevenSeg();
int comboSevenSeg();
int allSevenSeg();


//Initialize LINX Device And Listener
void setup()
{
//  Serial.begin(9600);
  //Instantiate The LINX Device
  LinxDevice = new LinxArduinoMega2560();
  dht.begin();
//  lcd.begin(16, 2);
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin2, OUTPUT);
  pinMode(latchPin2, OUTPUT);
  pinMode(clockPin2, OUTPUT);
  //setting all the displays off
  digitalWrite(latchPin, HIGH);
  for (int i = 0; i < 8; i++)
  {
    pinMode(scorePin[i], OUTPUT);
  }
  //The LINXT Listener Is Pre Instantiated, Call Start And Pass A Pointer To The LINX Device And The UART Channel To Listen On
  LinxSerialConnection.Start(LinxDevice, 0);
  //Setting listener to Custom functions
  LinxSerialConnection.AttachCustomCommand(0, setLCDCursor);
  LinxSerialConnection.AttachCustomCommand(1, numLCD);
  LinxSerialConnection.AttachCustomCommand(2, dht1);
  LinxSerialConnection.AttachCustomCommand(3, textLCD);
  LinxSerialConnection.AttachCustomCommand(4, clearLCD);
  LinxSerialConnection.AttachCustomCommand(5, stepper_motor);
  LinxSerialConnection.AttachCustomCommand(6, sevenSeg);
  LinxSerialConnection.AttachCustomCommand(7, scoreSevenSeg);
  LinxSerialConnection.AttachCustomCommand(8, timeSevenSeg);
  LinxSerialConnection.AttachCustomCommand(9, comboSevenSeg);
  LinxSerialConnection.AttachCustomCommand(11, allSevenSeg);
  // Print a message to the LCD.
//  lcd.print("Welcome to");
//  lcd.setCursor(0, 2);
//  lcd.print("ECE 144!");
//  delay(1000);
//  lcd.clear();
//  lcd.setCursor(0, 1);
}
void loop()
{
  //Listen For New Packets From LabVIEW
  LinxSerialConnection.CheckForCommands();
  //Your Code Here, But It will Slow Down The Connection With LabVIEW
}

int setLCDCursor(unsigned char numInputBytes, unsigned char* input, unsigned char* numResponseBytes, unsigned char* response)
{
//  lcd.setCursor(input[0], input[1]);  // col, row for writing onto new line
//  return 0;
}
int numLCD(unsigned char numInputBytes, unsigned char* input, unsigned char* numResponseBytes, unsigned char* response)
{
//  // Print only numbers to LCD
//  for (int i = 0; i < numInputBytes; i++)
//  {
//    response[i] = input[i];
//    lcd.print(response[i]);
//  }
//  *numResponseBytes = numInputBytes;
//  return 0;
}
int dht1(unsigned char numInputBytes, unsigned char* input, unsigned char* numResponseBytes, unsigned char* response)
{
//  response[0] = (unsigned char)dht.readHumidity();       // Humidity
//  response[1] = (unsigned char)dht.readTemperature();    // Temperature in Celsius
//  response[2] = (unsigned char)dht.readTemperature(true); // Temperature in Fahrenheit
//  *numResponseBytes = 3;
//  delay(500);//For stability of sensor
//  return 0;
}
int textLCD(unsigned char numInputBytes, unsigned char* input, unsigned char* numResponseBytes, unsigned char* response)
{
//  // Print only text to LCD and on LabVIEW(need to convert unit8_t to string)return string
//  for (int i = 0; i < numInputBytes; i++)
//  {
//    response[i] = char(input[i]);
//    lcd.print(char(response[i]));
//  }
//  *numResponseBytes = numInputBytes;
//  return 0;
}
int clearLCD(unsigned char numInputBytes, unsigned char* input, unsigned char* numResponseBytes, unsigned char* response)
{
//  lcd.clear();
//  return 0;
}
int stepper_motor(unsigned char numInputBytes, unsigned char* input, unsigned char* numResponseBytes, unsigned char* response)
{
  int step_value = 0;
  int step_response = 0;
  // Input from LabVIEW = [Direction, Number of Steps];
  for (int i = 1, j = numInputBytes - 1; i < numInputBytes; i++, j--) {
    step_value += input[i] * pow(10, j);
  }
  // Direction T = CW, F = CCW
  if (input[0] == 0) {
    steppermotor.move(-step_value);
  }
  else {
    steppermotor.move(step_value);
  }
  steppermotor.run();
  *numResponseBytes = numInputBytes;
  for (int i = 0; i < numInputBytes; i++) {
    response[i] = input[i];
  }
  return 0;
}


int sevenSeg(unsigned char numInputBytes, unsigned char* input, unsigned char* numResponseBytes, unsigned char* response)
{
  //Serial.println(*input);
  for (int i = 0; i < numInputBytes; i++)
  {
    response[i] = input[i];
    //response[i] = 4;
    byte num = myfnNumToBits(int(response[i]));
    digitalWrite(latchPin, LOW);  // prepare shift register for data
    shiftOut(dataPin, clockPin, LSBFIRST, num); // send data
    digitalWrite(latchPin, HIGH); // update display
  }

  //delay(5);
  *numResponseBytes = numInputBytes;
  return 0;

}

int scoreSevenSeg(unsigned char numInputBytes, unsigned char* input, unsigned char* numResponseBytes, unsigned char* response)
{
  //Serial.println(*input);
  for (int i = 0; i < numInputBytes; i++)
  {
    response[i] = input[i];
    //response[i] = 4;
    byte num = myfnNumToBits(int(response[i]));
    digitalWrite(latchPin, LOW);  // prepare shift register for data
    shiftOut(dataPin, clockPin, LSBFIRST, num); // send data
    digitalWrite(latchPin, HIGH); // update display
    digitalWrite(scorePin[i], LOW); // Send number to display
    delayMicroseconds(550);
    digitalWrite(scorePin[i], HIGH); // Move to next number
  }
  *numResponseBytes = numInputBytes;
  return 0;
}

int timeSevenSeg(unsigned char numInputBytes, unsigned char* input, unsigned char* numResponseBytes, unsigned char* response)
{
  //Serial.println(*input);
  for (int i = 0; i < 2; i++)
  {
//    response[i] = input[i];
//    //response[i] = 4;
//    byte num = myfnNumToBits(int(response[i]));
//    digitalWrite(latchPin, LOW);  // prepare shift register for data
//    shiftOut(dataPin, clockPin, LSBFIRST, num); // send data
//    digitalWrite(latchPin, HIGH); // update display
//    digitalWrite(timePin[i], LOW); // Send number to display
//    delayMicroseconds(550);
//    digitalWrite(timePin[i], HIGH); // Move to next number
  }
  *numResponseBytes = numInputBytes;
  return 0;
}

int comboSevenSeg(unsigned char numInputBytes, unsigned char* input, unsigned char* numResponseBytes, unsigned char* response)
{
  //Serial.println(*input);
//  response[0] = input[0];
//  //response[i] = 4;
//  byte num = myfnNumToBits(int(response[0]));
//  digitalWrite(latchPin, LOW);  // prepare shift register for data
//  shiftOut(dataPin, clockPin, LSBFIRST, num); // send data
//  digitalWrite(latchPin, HIGH); // update display
//  digitalWrite(comboPin, LOW); // Send number to display
//  delayMicroseconds(550);
//  digitalWrite(comboPin, HIGH); // Move to next number
  *numResponseBytes = numInputBytes;
  return 0;
}

int allSevenSeg(unsigned char numInputBytes, unsigned char* input, unsigned char* numResponseBytes, unsigned char* response)
{
  int bright = 90;
  for (int i = 0; i < 4; i++)
  {
//    response[i] = input[i];
    response[i + 4] = input[i + 4];
    //response[i] = 4;
    byte num = myfnNumToBits(int(response[i]));
    byte num2 = myfnNumToBits(int(response[i + 4]));
//    Serial.print(num2);
//    digitalWrite(latchPin, LOW);  // prepare shift register for data
    digitalWrite(latchPin2, LOW);  // prepare shift register for data2
//    shiftOut(dataPin, clockPin, LSBFIRST, num); // send data
    shiftOut(dataPin2, clockPin2, LSBFIRST, num2); // send data2
//    digitalWrite(latchPin, HIGH); // update display
    digitalWrite(latchPin2, HIGH); // update display2
//    digitalWrite(scorePin[i], LOW); // Send number to display
    digitalWrite(scorePin[i+4], LOW); // Send number to display2
//    delayMicroseconds(bright);
//    delay(1);
    delayMicroseconds(1638*((100-bright)/10));         // largest value 16383
//    digitalWrite(scorePin[i], HIGH); // Move to next number
    digitalWrite(scorePin[i+4], HIGH); // Move to next number
  }
//  Serial.println();
  *numResponseBytes = numInputBytes;
  return 0;
}

byte myfnNumToBits(int someNumber) {
  switch (someNumber) {
    case 0:
      return B11111100;//done
      break;
    case 1:
      return B01100000;//done
      break;
    case 2:
      return B11011010;//done
      break;
    case 3:
      return B11110010;//done
      break;
    case 4:
      return B01100110;//done
      break;
    case 5:
      return B10110110;//done
      break;
    case 6:
      return B10111110;//done
      break;
    case 7:
      return B11100000;//done
      break;
    case 8:
      return B11111110;//done
      break;
    case 9:
      return B11100110;//done
      break;
    case 10:
      return B00000001;//done
      break;

  }
}
