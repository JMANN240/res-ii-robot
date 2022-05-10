#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define JX A8
#define JY A9

#define SW1 53
#define SW2 65
#define SW3 64
#define SW4 61
#define SW5 60
#define SW6 59
#define SW7 58

Adafruit_SSD1306 display(128, 64, &Wire, -1);

int bt_msg_stride = 100;

void setup() {
  Serial.begin(38400, SERIAL_8E2);
  Serial1.begin(38400, SERIAL_8E2);

  pinMode(JX, INPUT);
  pinMode(JY, INPUT);

  pinMode(SW1, INPUT_PULLUP); // Left wheel adjust
  pinMode(SW2, INPUT_PULLUP); // Toggle menu
  pinMode(SW3, INPUT_PULLUP); // Right wheel adjust
  pinMode(SW4, INPUT_PULLUP); // Wheel adjust up
  pinMode(SW5, INPUT_PULLUP); // Wheel adjust down
  pinMode(SW6, INPUT_PULLUP);
  pinMode(SW7, INPUT_PULLUP);

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

double right_coeff = 1;
double left_coeff = 1.8;

long int current_time;
long int last_time;

int forward, leftward;

bool menu_button_state = false;
bool menu_button_state_last = false;

bool show_path = false;

char currentChar;
String command, attribute, data;
bool newCommand;

bool path[64][32];

int robot_x = 32;
int robot_y = 16;
double robot_r = 0;

void printVars() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(current_time);
  display.print(left_coeff);
  display.print(" ");
  display.println(right_coeff);
  display.print("(");
  display.print(robot_x);
  display.print(", ");
  display.print(robot_y);
  display.print("), ");
  display.println(robot_r);
  display.display();
}

void drawPath() {
  display.clearDisplay();
  display.setCursor(0, 0);
  for (int x = 0; x < 64; x++) {
    for (int y = 0; y < 32; y++) {
      if (path[x][y]) {
        display.drawPixel(x*2, 63-y*2, SSD1306_WHITE);
        display.drawPixel(x*2+1, 63-y*2, SSD1306_WHITE);
        display.drawPixel(x*2+1, 62-y*2, SSD1306_WHITE);
        display.drawPixel(x*2, 62-y*2, SSD1306_WHITE);
      }
    }
  }
  display.display();
}

void loop() {
  current_time = millis();
  
  forward = analogRead(JY);
  leftward = analogRead(JX);

  if (digitalRead(SW4) == 0) {
    if (digitalRead(SW1) == 0) {
      left_coeff += 0.01;
    }
    if (digitalRead(SW3) == 0) {
      right_coeff += 0.01;
    }
  }

  if (digitalRead(SW5) == 0) {
    if (digitalRead(SW1) == 0) {
      left_coeff -= 0.01;
    }
    if (digitalRead(SW3) == 0) {
      right_coeff -= 0.01;
    }
  }

  menu_button_state = digitalRead(SW2);

  if (menu_button_state == 0 && menu_button_state_last == 1) {
    show_path = !show_path;
  }

  menu_button_state_last = menu_button_state;

  if (current_time >= last_time + bt_msg_stride) {
    if (forward > (1024 - 256)) {
      Serial1.write(2);
      Serial1.print("S GO F");
      Serial1.write(3);
      Serial.println("forward");
    } else if (forward < 256) {
      Serial1.write(2);
      Serial1.print("S GO B");
      Serial1.write(3);
      Serial.println("backward");
    } else if (leftward > (1024 - 256)) {
      Serial1.write(2);
      Serial1.print("S GO L");
      Serial1.write(3);
      Serial.println("leftward");
    } else if (leftward < 256) {
      Serial1.write(2);
      Serial1.print("S GO R");
      Serial1.write(3);
      Serial.println("rightward");
    } else if (forward >= 256 && forward <= (1024 - 256) && leftward >= 256 && leftward <= (1024 - 256)) {
      Serial1.write(2);
      Serial1.print("S GO S");
      Serial1.write(3);
      Serial.println("stop");
    }

    Serial1.write(2);
    Serial1.print("S LC ");
    Serial1.print(left_coeff);
    Serial1.write(3);

    Serial1.write(2);
    Serial1.print("S RC ");
    Serial1.print(right_coeff);
    Serial1.write(3);

    last_time = current_time;
  }



  if (Serial1.available() > 0) {
    currentChar = (char)Serial1.read();
    if (currentChar == 2) {
      command = "";
      attribute = "";
      data = "";
      do {
        currentChar = (char)Serial1.read();
        if (!isspace(currentChar) && currentChar != -1) {
          command += currentChar;
        }
      } while (!isspace(currentChar));
      do {
        currentChar = Serial1.read();
        if (!isspace(currentChar) && currentChar != -1 && currentChar != 3) {
          attribute += currentChar;
        }
      } while (!isspace(currentChar) && currentChar != 3);
      if (command == "S") {
        do {
          currentChar = Serial1.read();
          if (!isspace(currentChar) && currentChar != -1 && currentChar != 3) {
            data += currentChar;
          }
        } while (!isspace(currentChar) && currentChar != 3);
      }
      do {
        currentChar = Serial1.read();
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
      if (attribute == "R") {
        robot_r = data.toDouble();
      } else if (attribute == "X") {
        robot_x = data.toInt();
        path[robot_x][robot_y] = true;
      } else if (attribute == "Y") {
        robot_y = data.toInt();
        path[robot_x][robot_y] = true;
      }
    }
  }

  

  if (show_path) {
    drawPath();
  } else {
    printVars();
  }

  delay(10);
}
