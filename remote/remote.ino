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

  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  pinMode(SW4, INPUT_PULLUP);
  pinMode(SW5, INPUT_PULLUP);
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

void printVars() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("time: ");
  display.println(current_time);
  display.print("right_coeff: ");
  display.println(right_coeff);
  display.print("left_coeff: ");
  display.println(left_coeff);
  display.print(digitalRead(SW1));
  display.print(" ");
  display.println(digitalRead(SW3));
  display.print(" ");
  display.println(digitalRead(SW4));
  display.print(" ");
  display.println(digitalRead(SW5));
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

  printVars();

  delay(10);
}
