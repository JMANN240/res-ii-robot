#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h> //Display

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

// SoftwareSerial mySerial(2, 3); // RX, TX

int getBit(byte n, int b) {
  return (n & (1 << b)) >> b;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400, SERIAL_8E2);
  // mySerial.begin(38400, SERIAL_8E2);
  
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

int count  = 0;

int left_motor_power;
int left_motor_direction;
int right_motor_power;
int right_motor_direction;

int left_count = 0;
int right_count = 0;

double left_coeff = 1;
double right_coeff = 1;

void calibrate() {
  left_count = 0;
  right_count = 0;
  
  analogWrite(PIN_MOTOR_PWM_LEFT, 128 * left_coeff);
  digitalWrite(PIN_MOTOR_IN0_LEFT, HIGH);
  digitalWrite(PIN_MOTOR_IN1_LEFT , LOW);
  analogWrite(PIN_MOTOR_PWM_RIGHT, 128 * right_coeff);
  digitalWrite(PIN_MOTOR_IN0_RIGHT, LOW);
  digitalWrite(PIN_MOTOR_IN1_RIGHT , HIGH);

  delay(1000);
  
  analogWrite(PIN_MOTOR_PWM_LEFT, 0);
  digitalWrite(PIN_MOTOR_IN0_LEFT, LOW);
  digitalWrite(PIN_MOTOR_IN1_LEFT , LOW);
  analogWrite(PIN_MOTOR_PWM_RIGHT, 0);
  digitalWrite(PIN_MOTOR_IN0_RIGHT, LOW);
  digitalWrite(PIN_MOTOR_IN1_RIGHT , LOW);

  delay(1000);

  //left_coeff = left_coeff * (right_count / left_count);
  //right_coeff = right_coeff * (left_count / right_count);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(left_count);
  display.println(right_count);
  display.println(left_coeff);
  display.println(right_coeff);
  display.display();
  
  delay(1000);
}

bool calibrated = false;

void loop() {
  if (!calibrated && millis() > 10000) {
    calibrate();
    calibrate();
    calibrate();
    calibrate();
    calibrate();
    calibrated = true;
  }
  // put your main code here, to run repeatedly:
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(millis());
  if (Serial.available() > 0) {
    i = Serial.read();
    last = millis();
    
    left_motor_power = getBit(i, 0);
    left_motor_direction = getBit(i, 1);
    right_motor_power = getBit(i, 2);
    right_motor_direction = getBit(i, 3);

    if (left_motor_power == 1) {
      analogWrite(PIN_MOTOR_PWM_LEFT, 128 * left_coeff);
      if (left_motor_direction == 0) {
        display.print("lf ");
        digitalWrite(PIN_MOTOR_IN0_LEFT, HIGH);
        digitalWrite(PIN_MOTOR_IN1_LEFT , LOW);
      } else if (left_motor_direction == 1) {
        display.print("lb ");
        digitalWrite(PIN_MOTOR_IN0_LEFT, LOW);
        digitalWrite(PIN_MOTOR_IN1_LEFT , HIGH);
      }
    } else {
      display.print("ls ");
      analogWrite(PIN_MOTOR_PWM_LEFT, 0);
      digitalWrite(PIN_MOTOR_IN0_LEFT, LOW);
      digitalWrite(PIN_MOTOR_IN1_LEFT , LOW);
    }

    if (right_motor_power == 1) {
      analogWrite(PIN_MOTOR_PWM_RIGHT, 128 * right_coeff);
      if (right_motor_direction == 0) {
        display.println("rf");
        digitalWrite(PIN_MOTOR_IN0_RIGHT, LOW);
        digitalWrite(PIN_MOTOR_IN1_RIGHT , HIGH);
      } else if (right_motor_direction == 1) {
        display.println("rb");
        digitalWrite(PIN_MOTOR_IN0_RIGHT, HIGH);
        digitalWrite(PIN_MOTOR_IN1_RIGHT , LOW);
      }
    } else {
      display.println("rs");
      analogWrite(PIN_MOTOR_PWM_RIGHT, 0);
      digitalWrite(PIN_MOTOR_IN0_RIGHT, LOW);
      digitalWrite(PIN_MOTOR_IN1_RIGHT , LOW);
    }
  }

  display.println(i);

  display.println(left_count);
  display.println(right_count);

  display.print(digitalRead(PIN_WHEEL_ENCODER_CLK_LEFT));
  display.print(digitalRead(PIN_WHEEL_ENCODER_DT_LEFT));
  display.print(digitalRead(PIN_WHEEL_ENCODER_CLK_RIGHT));
  display.println(digitalRead(PIN_WHEEL_ENCODER_DT_RIGHT));
  
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
