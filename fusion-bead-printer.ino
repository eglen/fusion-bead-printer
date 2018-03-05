#include <AccelStepper.h>
#include <Servo.h>
#include <LiquidCrystal.h>
#include <Keypad.h>


 //Test
#define HALFSTEP 8

// Motor pin definitions
#define motorPin1  3     // IN1 on the ULN2003 driver 1
#define motorPin2  4     // IN2 on the ULN2003 driver 1
#define motorPin3  5     // IN3 on the ULN2003 driver 1
#define motorPin4  6     // IN4 on the ULN2003 driver 1

//Servo pins
#define selectorWheelPin 7
#define dispenserPin 2

//Keypad config
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the symbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}  
};

byte rowPins[ROWS] = {38, 40, 42, 44}; 
//connect to the row pinouts of the keypad
byte colPins[COLS] = {46, 48, 50, 52}; 
//connect to the column pinouts of the keypad
//initialize an instance of class NewKeypad
Keypad keypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);


// Initialize stepper with pin sequence IN1-IN3-IN2-IN4 for using the AccelStepper with 28BYJ-48
AccelStepper stepper1(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);

// initialize LCD interface pin
// with the arduino pin number it is connected to
const int rs = 13, en = 12, d4 = 11, d5 = 10, d6 = 9, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

Servo colorWheelServo;
Servo dispenserServo;

//These globals are scary, need to compartmentalise these better
int dispenserStateIndex = 0;
unsigned long dispenserStateTimer = millis(); //Avoid delay() that holds up the main loop
const int dispenserStepDelay = 200;
const int dispenserDrop = 141;
const int dispenserPickup = 120;
const int dispenserShoogleFactor = 3;
int dispenserState[6] = {
  dispenserDrop, //0 Drop location
  dispenserPickup - dispenserShoogleFactor, //1 Far pickup location
  dispenserPickup + dispenserShoogleFactor, //2 Pickup jiggle
  dispenserPickup - dispenserShoogleFactor, //3 Far pickup jiggle
  dispenserDrop + dispenserShoogleFactor, //4 Far dropoff
  dispenserDrop - dispenserShoogleFactor //5 Dropoff jiggle
};

int colorSelection = 5;
int colorWheelStep = 15; //Degrees between each position

void setup() {
  stepper1.setMaxSpeed(5000.0);
  stepper1.setAcceleration(10.0);
  stepper1.setSpeed(0);
  // stepper1.moveTo(20000);
  colorWheelServo.attach(selectorWheelPin);
  dispenserServo.attach(dispenserPin);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  
  //Keypad
  Serial.begin(9600);

}//--(end setup )---

void loop() {
  
  // stepper speed pot:
  int sensorReading = analogRead(A0);
  // stepper speed mapping
  int motorSpeed = map(sensorReading, 0, 1023, -5000, 5000);
  stepper1.setSpeed(0);
  stepper1.run();
  
  //Analog input for various uses
  int analogIn = analogRead(A1);
  int analogMapped = map(analogIn, 0, 1023, 0, 180);
 
  char key = keypad.getKey();
  if (keypad.keyStateChanged()) {
    //Dispenser servo
    if (key == 'A') {
      dispenserStateIndex = 1;
      dispenserStateTimer = millis();
    } else if (isDigit(key)) {
      colorSelection = key - '0';
    }
  }
  
  
  int colorWheelTargetPosition = updateColorServo();
  updateDispenserServo();
  updateLcd(colorWheelTargetPosition, key);
}

int updateColorServo() {
    //Color selector servo:
  int colorWheelServoPos = (colorSelection * colorWheelStep);
  //int colorWheelServoPos = 90;//90 + colorWheelOffset;
  colorWheelServo.write(colorWheelServoPos);
  return colorWheelServoPos;
}

void updateDispenserServo() {
  //dispenserServo.write(115);
  dispenserServo.write(dispenserState[dispenserStateIndex]);
  //dispenserServo.write(analogMapped);
  if (dispenserStateIndex != 0 && (millis() - dispenserStateTimer) > dispenserStepDelay) {
    dispenserStateTimer = millis();
    dispenserStateIndex = (dispenserStateIndex + 1) % 6;
  }
}

void updateLcd(int colorWheelTargetPosition, char key) {
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("C: ");
  lcd.print(colorWheelTargetPosition);
  lcd.print(" ");
  lcd.setCursor(7, 0);
  lcd.print(" D: ");
  //lcd.print(analogMapped);
  lcd.print(dispenserState[dispenserStateIndex]);
  lcd.print(" ");
  if (key) {
    lcd.setCursor(0, 1);
    lcd.print("char: ");
    lcd.print(key);
  }
  lcd.setCursor(7, 1);
  lcd.print(" I:");
  lcd.print(dispenserStateIndex);
  
  lcd.display();
}
