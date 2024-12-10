#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>

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
unsigned long personDetectedTime = 0;

const char* ssid = "YourSSID";
const char* password = "YourPassword";
const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

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
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    if (client.connect("SmartBinClient")) {
      Serial.println("Connected to MQTT Broker");
    } else {
      delay(1000);
    }
  }
}

void loop() {
  unsigned long currentMillis = millis();
  long personDistance = readUltrasonicDistance(trigPersonPin, echoPersonPin);

  if (personDistance > 0 && personDistance <= 20 && !personDetected) {
    personDetected = true;
    personDetectedTime = currentMillis;
    mainDoorServo.write(90);
    doorOpened = true;
    client.publish("smartbin/personDetected", "1");
  }

  if (doorOpened && currentMillis - personDetectedTime >= 5000) {
    mainDoorServo.write(0);
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
    client.publish("smartbin/wasteDetected", "Logam");
    directionServo.write(130);
    delay(2000);
    myservo.write(150);
    delay(2000);
    myservo.write(40);
  } else if (organikDetected == LOW) {
    client.publish("smartbin/wasteDetected", "Organik");
    directionServo.write(58);
    delay(2000);
    myservo.write(150);
    delay(2000);
    myservo.write(40);
  } else if (anorganikDetected == LOW) {
    client.publish("smartbin/wasteDetected", "Anorganik");
    directionServo.write(0);
    delay(2000);
    myservo.write(150);
    delay(2000);
    myservo.write(40);
  }
}

void checkBinFullness() {
  long organikDistance = readUltrasonicDistance(trigOrganikPin, echoOrganikPin);
  long anorganikDistance = readUltrasonicDistance(trigAnorganikPin, echoAnorganikPin);
  long bahayaDistance = readUltrasonicDistance(trigBahayaPin, echoBahayaPin);

  if (organikDistance > 0 && organikDistance <= 10) {
    client.publish("smartbin/organikFull", "1");
  }
  if (anorganikDistance > 0 && anorganikDistance <= 10) {
    client.publish("smartbin/anorganikFull", "1");
  }
  if (bahayaDistance > 0 && bahayaDistance <= 10) {
    client.publish("smartbin/logamFull", "1");
  }
}