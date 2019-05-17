void setup() {
  // put your setup code here, to run once:
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
}

void green() {
  analogWrite(5, 127);
  analogWrite(3, 0);
}

void red() {
  analogWrite(5, 127);
  analogWrite(3, 255);
}

void off() {
  analogWrite(5, 0);
  analogWrite(3, 0);  
}

void loop() {
  red();
  delay(1000);
  green();
  delay(1000);
  off();
  delay(1000);
}
