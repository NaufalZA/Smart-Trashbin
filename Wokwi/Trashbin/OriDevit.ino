#include <ESP32Servo.h>

Servo myservo;
Servo directionServo;
Servo mainDoorServo;

const int proxyPin = 35;
const int irPin = 32;
const int rainDigitalPin = 25;
const int servoPin = 16;
const int directionServoPin = 17;
const int mainDoorServoPin = 18;

const int trigPersonPin = 5;
const int echoPersonPin = 4;

const int trigOrganikPin = 12;
const int echoOrganikPin = 13;
const int trigAnorganikPin = 14;
const int echoAnorganikPin = 15;
const int trigBahayaPin = 19;
const int echoBahayaPin = 21;

bool personDetected = false;
bool doorOpened = false;
bool wasteProcessed = false;
unsigned long personDetectedTime = 0;

void setup() {
  pinMode(proxyPin, INPUT);
  pinMode(irPin, INPUT);
  pinMode(rainDigitalPin, INPUT);

  pinMode(trigPersonPin, OUTPUT);
  pinMode(echoPersonPin, INPUT);

  pinMode(trigOrganikPin, OUTPUT);
  pinMode(echoOrganikPin, INPUT);
  pinMode(trigAnorganikPin, OUTPUT);
  pinMode(echoAnorganikPin, INPUT);
  pinMode(trigBahayaPin, OUTPUT);
  pinMode(echoBahayaPin, INPUT);

  myservo.attach(servoPin);
  directionServo.attach(directionServoPin);
  mainDoorServo.attach(mainDoorServoPin);

  Serial.begin(115200);
  Serial.println("Sistem tong sampah pintar siap!");
}

void loop() {
  unsigned long currentMillis = millis();
  
  long personDistance = readUltrasonicDistance(trigPersonPin, echoPersonPin);
  if (personDistance > 0 && personDistance <= 5 && !personDetected) {
    personDetected = true;
    personDetectedTime = currentMillis;
    Serial.println("Orang terdeteksi, membuka pintu utama.");
    mainDoorServo.write(150);
    doorOpened = true;
  }

  if (doorOpened && currentMillis - personDetectedTime >= 3000) {
    mainDoorServo.write(45);
    doorOpened = false;
    personDetected = false;
  }

  detectAndSortWaste();
  checkBinFullness();
}

long readUltrasonicDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;
}

void detectAndSortWaste() {
  int logamDetected = digitalRead(proxyPin);
  int anorganikDetected = digitalRead(irPin);
  int organikDetected = digitalRead(rainDigitalPin);

  if (logamDetected == LOW) {
    Serial.println("Sampah logam terdeteksi");
    directionServo.write(130);
    delay(2000);
    myservo.write(140);
    delay(2000);
    myservo.write(40);
  } 
  else if (organikDetected == LOW) {
    Serial.println("Sampah organik terdeteksi");
    directionServo.write(58);
    delay(2000);
    myservo.write(140);
    delay(2000);
    myservo.write(40);
  } 
  else if (anorganikDetected == LOW) {
    Serial.println("Sampah anorganik terdeteksi");
    directionServo.write(0);
    delay(2000);
    myservo.write(140);
    delay(2000);
    myservo.write(40);
  }
}

void checkBinFullness() {
  long organikDistance = readUltrasonicDistance(trigOrganikPin, echoOrganikPin);
  long anorganikDistance = readUltrasonicDistance(trigAnorganikPin, echoAnorganikPin);
  long bahayaDistance = readUltrasonicDistance(trigBahayaPin, echoBahayaPin);

  if (organikDistance > 0 && organikDistance <= 5) {
    Serial.println("Tong sampah organik penuh!");
  } else if (organikDistance > 5 && organikDistance <= 10) {
    Serial.println("Tong sampah organik hampir penuh!");
  }

  if (anorganikDistance > 0 && anorganikDistance <= 5) {
    Serial.println("Tong sampah anorganik penuh!");
  } else if (anorganikDistance > 5 && anorganikDistance <= 10) {
    Serial.println("Tong sampah anorganik hampir penuh!");
  }

  if (bahayaDistance > 0 && bahayaDistance <= 5) {
    Serial.println("Tong sampah berbahaya penuh!");
  } else if (bahayaDistance > 5 && bahayaDistance <= 10) {
    Serial.println("Tong sampah berbahaya hampir penuh!");
  }
}