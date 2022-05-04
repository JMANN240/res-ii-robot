#define JX A8
#define JY A9

#define SW1 53
#define SW2 65
#define SW3 64
#define SW4 61
#define SW5 60
#define SW6 59
#define SW7 58

void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400, SERIAL_8E2);
  Serial1.begin(38400, SERIAL_8E2);

  pinMode(JX, INPUT);
  pinMode(JY, INPUT);

  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(SW3, INPUT);
  pinMode(SW4, INPUT);
  pinMode(SW5, INPUT);
  pinMode(SW6, INPUT);
  pinMode(SW7, INPUT);
}

void loop() {
  int forward = analogRead(JY);
  int leftward = analogRead(JX);

  if (forward > (1024 - 256)) {
    Serial1.write(0b1111);
    Serial.println("forward");
    last_pos = "forward";
  } else if (forward < 256) {
    Serial1.write(0b0101);
    Serial.println("backward");
    last_pos = "backward";
  } else if (leftward > (1024 - 256)) {
    Serial1.write(0b1101);
    Serial.println("leftward");
    last_pos = "leftward";
  } else if (leftward < 256) {
    Serial1.write(0b0111);
    Serial.println("rightward");
    last_pos = "rightward";
  } else if (forward >= 256 && forward <= (1024 - 256) && leftward >= 256 && leftward <= (1024 - 256)) {
    Serial1.write(0b0000);
    Serial.println("stop");
    last_pos = "stop";
  }

  delay(200);
}
