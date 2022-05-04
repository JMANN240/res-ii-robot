#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define PIN_MOTOR_PWM_LEFT    9
#define PIN_MOTOR_IN0_LEFT    7
#define PIN_MOTOR_IN1_LEFT    8

#define PIN_MOTOR_PWM_RIGHT    4
#define PIN_MOTOR_IN0_RIGHT    6
#define PIN_MOTOR_IN1_RIGHT    5

#define PIN_WHEEL_ENCODER_CLK_LEFT    2
#define PIN_WHEEL_ENCODER_DT_LEFT    A3

#define PIN_WHEEL_ENCODER_CLK_RIGHT    3
#define PIN_WHEEL_ENCODER_DT_RIGHT    A1

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  Serial.begin(38400, SERIAL_8E2);
  
  pinMode(PIN_MOTOR_PWM_LEFT, OUTPUT);
  pinMode(PIN_MOTOR_IN0_LEFT, OUTPUT);
  pinMode(PIN_MOTOR_IN1_LEFT, OUTPUT);
  
  pinMode(PIN_MOTOR_PWM_RIGHT, OUTPUT);
  pinMode(PIN_MOTOR_IN0_RIGHT, OUTPUT);
  pinMode(PIN_MOTOR_IN1_RIGHT, OUTPUT);
  
  pinMode(PIN_WHEEL_ENCODER_DT_LEFT, INPUT_PULLUP);
  pinMode(PIN_WHEEL_ENCODER_CLK_LEFT, INPUT_PULLUP);
  
  pinMode(PIN_WHEEL_ENCODER_DT_RIGHT, INPUT_PULLUP);
  pinMode(PIN_WHEEL_ENCODER_CLK_RIGHT, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(PIN_WHEEL_ENCODER_CLK_RIGHT), updateEncoderRight, CHANGE);  
  attachInterrupt(digitalPinToInterrupt(PIN_WHEEL_ENCODER_CLK_LEFT), updateEncoderLeft, CHANGE);  

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);

  delay(1000);
}

byte i;

long int last;

int left_motor_power;
int left_motor_direction;
int right_motor_power;
int right_motor_direction;

int left_count = 0;
int right_count = 0;

double left_coeff = 1;
double right_coeff = 1;

char currentChar;
String command, attribute, data;
bool newCommand;

void loop() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(millis());
  
  if (Serial.available() > 0) {
    currentChar = (char)Serial.read();
    if (currentChar == 2) {
      command = "";
      attribute = "";
      data = "";
      do {
        currentChar = (char)Serial.read();
        if (!isspace(currentChar) && currentChar != -1) {
          command += currentChar;
        }
      } while (!isspace(currentChar));
      do {
        currentChar = Serial.read();
        if (!isspace(currentChar) && currentChar != -1 && currentChar != 3) {
          attribute += currentChar;
        }
      } while (!isspace(currentChar) && currentChar != 3);
      if (command == "S") {
        do {
          currentChar = Serial.read();
          if (!isspace(currentChar) && currentChar != -1 && currentChar != 3) {
            data += currentChar;
          }
        } while (!isspace(currentChar) && currentChar != 3);
      }
      do {
        currentChar = Serial.read();
      } while (currentChar != 3);
      newCommand = true;
    }
  }

  if (newCommand) {
    display.print(command);
    display.print(" ");
    display.print(attribute);
    display.print(" ");
    display.println(data);
    if (command == "S") {
      if (attribute == "GO") {
        if (data == "F") {
          analogWrite(PIN_MOTOR_PWM_LEFT, 128 * left_coeff);
          analogWrite(PIN_MOTOR_PWM_RIGHT, 128 * right_coeff);
          digitalWrite(PIN_MOTOR_IN0_LEFT, LOW);
          digitalWrite(PIN_MOTOR_IN1_LEFT , HIGH);
          digitalWrite(PIN_MOTOR_IN0_RIGHT, HIGH);
          digitalWrite(PIN_MOTOR_IN1_RIGHT , LOW);
        } else if (data == "B") {
          analogWrite(PIN_MOTOR_PWM_LEFT, 128 * left_coeff);
          analogWrite(PIN_MOTOR_PWM_RIGHT, 128 * right_coeff);
          digitalWrite(PIN_MOTOR_IN0_LEFT, HIGH);
          digitalWrite(PIN_MOTOR_IN1_LEFT , LOW);
          digitalWrite(PIN_MOTOR_IN0_RIGHT, LOW);
          digitalWrite(PIN_MOTOR_IN1_RIGHT , HIGH);
        } else if (data == "L") {
          analogWrite(PIN_MOTOR_PWM_LEFT, 128 * left_coeff);
          analogWrite(PIN_MOTOR_PWM_RIGHT, 128 * right_coeff);
          digitalWrite(PIN_MOTOR_IN0_LEFT, HIGH);
          digitalWrite(PIN_MOTOR_IN1_LEFT , LOW);
          digitalWrite(PIN_MOTOR_IN0_RIGHT, HIGH);
          digitalWrite(PIN_MOTOR_IN1_RIGHT , LOW);
        } else if (data == "R") {
          analogWrite(PIN_MOTOR_PWM_LEFT, 128 * left_coeff);
          analogWrite(PIN_MOTOR_PWM_RIGHT, 128 * right_coeff);
          digitalWrite(PIN_MOTOR_IN0_LEFT, LOW);
          digitalWrite(PIN_MOTOR_IN1_LEFT , HIGH);
          digitalWrite(PIN_MOTOR_IN0_RIGHT, LOW);
          digitalWrite(PIN_MOTOR_IN1_RIGHT , HIGH);
        } else if (data == "S") {
          analogWrite(PIN_MOTOR_PWM_LEFT, 0);
          analogWrite(PIN_MOTOR_PWM_RIGHT, 0);
          digitalWrite(PIN_MOTOR_IN0_LEFT, LOW);
          digitalWrite(PIN_MOTOR_IN1_LEFT , LOW);
          digitalWrite(PIN_MOTOR_IN0_RIGHT, LOW);
          digitalWrite(PIN_MOTOR_IN1_RIGHT , LOW);
        }
      } else if (attribute == "LC") {
        left_coeff = data.toDouble();
      } else if (attribute == "RC") {
        right_coeff = data.toDouble();
      }
    }
  }

  display.print(left_coeff);
  display.print("    ");
  display.println(right_coeff);

  display.print(left_count);
  display.print("    ");
  display.println(right_count);
  
  display.display();
}

void updateEncoderLeft(){
  int encoderPin_X = digitalRead(PIN_WHEEL_ENCODER_CLK_LEFT);
  int encoderPin_Y = digitalRead(PIN_WHEEL_ENCODER_DT_LEFT);

  if (encoderPin_X == 0){   
    if (encoderPin_Y == 0){
      left_count++;
    }else{
      left_count++;
    }
  }else{
    if (encoderPin_Y == 0){
      left_count++;
    }else{
      left_count++;
    }    
  }
}

void updateEncoderRight(){
  int encoderPin_X = digitalRead(PIN_WHEEL_ENCODER_CLK_RIGHT);
  int encoderPin_Y = digitalRead(PIN_WHEEL_ENCODER_DT_RIGHT);

  if (encoderPin_X == 0){   
    if (encoderPin_Y == 0){
      right_count++;
    }else{
      right_count--;
    }
  }else{
    if (encoderPin_Y == 0){
      right_count--;
    }else{
      right_count++;
    }    
  }
}
