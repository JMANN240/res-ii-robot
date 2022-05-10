#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// Motor pins

#define PIN_MOTOR_PWM_LEFT    9
#define PIN_MOTOR_IN0_LEFT    7
#define PIN_MOTOR_IN1_LEFT    8
#define PIN_MOTOR_PWM_RIGHT    4
#define PIN_MOTOR_IN0_RIGHT    6
#define PIN_MOTOR_IN1_RIGHT    5


// Rotary encoder pins

#define PIN_WHEEL_ENCODER_CLK_LEFT    2
#define PIN_WHEEL_ENCODER_DT_LEFT    A3
#define PIN_WHEEL_ENCODER_CLK_RIGHT    3
#define PIN_WHEEL_ENCODER_DT_RIGHT    A1


// Device initialization

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_MPU6050 mpu;


// Time

int current_millis = 0, last_millis = 0;
float delta_millis = 0;

float rate = 100; //Hz


// MPU

sensors_event_t a, g, temp;
float baseline_gyro_z;
float rot = 0;


// Odometer/wheels

int left_count = 0;
int right_count = 0;

int last_left_count = 0;
int last_right_count = 0;

int delta_left_count = 0;
int delta_right_count = 0;

float left_coeff = 1;
float right_coeff = 1;

double robot_x = 32;
double robot_y = 16;

double scale = 0.01;


//Commands

char currentChar;
String command, attribute, data;
bool newCommand;


void setup() {
  // Begin the serial
  Serial.begin(38400, SERIAL_8E2);

  // Set the motor pins as outputs
  pinMode(PIN_MOTOR_PWM_LEFT, OUTPUT);
  pinMode(PIN_MOTOR_IN0_LEFT, OUTPUT);
  pinMode(PIN_MOTOR_IN1_LEFT, OUTPUT);
  pinMode(PIN_MOTOR_PWM_RIGHT, OUTPUT);
  pinMode(PIN_MOTOR_IN0_RIGHT, OUTPUT);
  pinMode(PIN_MOTOR_IN1_RIGHT, OUTPUT);

  // Set the encoder pins as inputs
  pinMode(PIN_WHEEL_ENCODER_DT_LEFT, INPUT_PULLUP);
  pinMode(PIN_WHEEL_ENCODER_CLK_LEFT, INPUT_PULLUP);
  pinMode(PIN_WHEEL_ENCODER_DT_RIGHT, INPUT_PULLUP);
  pinMode(PIN_WHEEL_ENCODER_CLK_RIGHT, INPUT_PULLUP);

  // Attach the counter interrupts to the encoder clock pins
  attachInterrupt(digitalPinToInterrupt(PIN_WHEEL_ENCODER_CLK_RIGHT), updateEncoderRight, CHANGE);  
  attachInterrupt(digitalPinToInterrupt(PIN_WHEEL_ENCODER_CLK_LEFT), updateEncoderLeft, CHANGE);  

  // Initialize the display and clear it
  /*if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);*/

  // Initialize the MPU
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  mpu.getEvent(&a, &g, &temp);
  baseline_gyro_z = g.gyro.z;

  delay(1000);
}


void loop() {
  // Set the current and last time
  last_millis = current_millis;
  current_millis = millis();
  delta_millis = current_millis - last_millis;

  // Set the last and delta counts
  delta_left_count = left_count - last_left_count;
  delta_right_count = right_count - last_right_count;
  last_left_count = left_count;
  last_right_count = right_count;

  // Read from the MPU and set the rot
  mpu.getEvent(&a, &g, &temp);
  rot += ((g.gyro.z - baseline_gyro_z) / (1000 / delta_millis));

  //Calculate the change in X and Y
  robot_x += (scale * cos(rot) * (delta_left_count + delta_right_count) / 2);
  robot_y += (scale * sin(rot) * (delta_left_count + delta_right_count) / 2);
  
  // Reset the display and print milliseconds
  /*display.clearDisplay();
  display.setCursor(0, 0);
  display.println(millis());*/

  // Read serial command if it is available  
  if (Serial.available() > 0) {

    // Read the first character
    currentChar = (char)Serial.read();

    // If it isn't a 2, ignore it
    if (currentChar == 2) {

      // Empty the command, attribute, and data variables
      command = "";
      attribute = "";
      data = "";

      // Create the command
      do {
        // Read a character and add it to the command if it is valid
        currentChar = (char)Serial.read();
        if (!isspace(currentChar) && currentChar != -1) {
          command += currentChar;
        }

        // Read again if we haven't reached a space
      } while (!isspace(currentChar));

      //Create the attribute
      do {
        // Read a character and add it to the attribute if it is valid
        currentChar = Serial.read();
        if (!isspace(currentChar) && currentChar != -1 && currentChar != 3) {
          attribute += currentChar;
        }

        // Read again if we haven't reached a space
      } while (!isspace(currentChar) && currentChar != 3);

      // If the command was to set the attribute
      if (command == "S") {

        // Create the data
        do {
          // Read a character and add it to the data if it is valid
          currentChar = Serial.read();
          if (!isspace(currentChar) && currentChar != -1 && currentChar != 3) {
            data += currentChar;
          }

        // Read again if we haven't reached a space
        } while (!isspace(currentChar) && currentChar != 3);
      }

      // Clear the serial buffer if we have some leftovers
      do {
        currentChar = Serial.read();
      } while (currentChar != 3);

      // Let the rest of the program know we have a new command to process
      newCommand = true;
    }
  }
  
  // If there is a new command to process
  if (newCommand) {

    // Print it
    /*Serial.print(command);
    Serial.print(" ");
    Serial.print(attribute);
    Serial.print(" ");
    Serial.println(data);*/

    // If it is a set command
    if (command == "S") {
      if (attribute == "GO") { // Motor control
        if (data == "F") { // Forward
          analogWrite(PIN_MOTOR_PWM_LEFT, 128 * left_coeff);
          analogWrite(PIN_MOTOR_PWM_RIGHT, 128 * right_coeff);
          digitalWrite(PIN_MOTOR_IN0_LEFT, LOW);
          digitalWrite(PIN_MOTOR_IN1_LEFT , HIGH);
          digitalWrite(PIN_MOTOR_IN0_RIGHT, HIGH);
          digitalWrite(PIN_MOTOR_IN1_RIGHT , LOW);
        } else if (data == "B") { // Backward
          analogWrite(PIN_MOTOR_PWM_LEFT, 128 * left_coeff);
          analogWrite(PIN_MOTOR_PWM_RIGHT, 128 * right_coeff);
          digitalWrite(PIN_MOTOR_IN0_LEFT, HIGH);
          digitalWrite(PIN_MOTOR_IN1_LEFT , LOW);
          digitalWrite(PIN_MOTOR_IN0_RIGHT, LOW);
          digitalWrite(PIN_MOTOR_IN1_RIGHT , HIGH);
        } else if (data == "L") { // Leftward
          analogWrite(PIN_MOTOR_PWM_LEFT, 128 * left_coeff);
          analogWrite(PIN_MOTOR_PWM_RIGHT, 128 * right_coeff);
          digitalWrite(PIN_MOTOR_IN0_LEFT, HIGH);
          digitalWrite(PIN_MOTOR_IN1_LEFT , LOW);
          digitalWrite(PIN_MOTOR_IN0_RIGHT, HIGH);
          digitalWrite(PIN_MOTOR_IN1_RIGHT , LOW);
        } else if (data == "R") { // Rightward
          analogWrite(PIN_MOTOR_PWM_LEFT, 128 * left_coeff);
          analogWrite(PIN_MOTOR_PWM_RIGHT, 128 * right_coeff);
          digitalWrite(PIN_MOTOR_IN0_LEFT, LOW);
          digitalWrite(PIN_MOTOR_IN1_LEFT , HIGH);
          digitalWrite(PIN_MOTOR_IN0_RIGHT, LOW);
          digitalWrite(PIN_MOTOR_IN1_RIGHT , HIGH);
        } else if (data == "S") { // Stop
          analogWrite(PIN_MOTOR_PWM_LEFT, 0);
          analogWrite(PIN_MOTOR_PWM_RIGHT, 0);
          digitalWrite(PIN_MOTOR_IN0_LEFT, LOW);
          digitalWrite(PIN_MOTOR_IN1_LEFT , LOW);
          digitalWrite(PIN_MOTOR_IN0_RIGHT, LOW);
          digitalWrite(PIN_MOTOR_IN1_RIGHT , LOW);
        }
      } else if (attribute == "LC") { // Left coefficient
        left_coeff = data.toDouble();
      } else if (attribute == "RC") { // Right coefficient
        right_coeff = data.toDouble();
      }
    }
  }

  Serial.write(2);
  Serial.print("S R ");
  Serial.print(rot);
  Serial.write(3);

  Serial.write(2);
  Serial.print("S X ");
  Serial.print(robot_x);
  Serial.write(3);

  Serial.write(2);
  Serial.print("S Y ");
  Serial.print(robot_y);
  Serial.write(3);

  delay(1000/rate);
}

void updateEncoderLeft(){
  int encoderPin_X = digitalRead(PIN_WHEEL_ENCODER_CLK_LEFT);
  int encoderPin_Y = digitalRead(PIN_WHEEL_ENCODER_DT_LEFT);

  if (encoderPin_X == 0){   
    if (encoderPin_Y == 0){
      left_count--;
    }else{
      left_count++;
    }
  }else{
    if (encoderPin_Y == 0){
      left_count++;
    }else{
      left_count--;
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
