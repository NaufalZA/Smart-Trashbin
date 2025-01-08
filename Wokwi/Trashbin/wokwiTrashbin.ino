#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>

Servo myservo;
Servo directionServo;
Servo mainDoorServo;

const int proxyPin = 35;
const int irPin = 36;
const int rainDigitalPin = 26;
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

const char* ssid = "Wokwi-GUEST";      
const char* password = " ";   
const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
const char* TOPIC_DOOR_CONTROL = "trashbin/pintu";
const char* TOPIC_BIN_DATA = "trashbin/data";
WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  if (String(topic) == TOPIC_DOOR_CONTROL) {
    if ((char)payload[0] == '1') {
      mainDoorServo.write(90);
      doorOpened = true;
    } else if ((char)payload[0] == '0') {
      mainDoorServo.write(0);
      doorOpened = false;
    }
  }
}

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
  client.setCallback(callback);
  
  while (!client.connected()) {
    if (client.connect("SmartBinClient")) {
      Serial.println("Connected to MQTT Broker");
      client.subscribe(TOPIC_DOOR_CONTROL);
    } else {
      delay(1000);
    }
  }
}

void loop() {
  client.loop();  // Add this at the beginning of loop()
  
  unsigned long currentMillis = millis();
  long personDistance = readUltrasonicDistance(trigPersonPin, echoPersonPin);

  if (personDistance > 0 && personDistance <= 20 && !personDetected) {
    personDetected = true;
    personDetectedTime = currentMillis;
    mainDoorServo.write(90); 
    doorOpened = true;
  }

  if (doorOpened && currentMillis - personDetectedTime >= 5000) {
    mainDoorServo.write(0);
    doorOpened = false;
    personDetected = false;
  }

  detectAndSortWaste();
  publishBinFullness(); 
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

void publishData(int kategori, int jarak) {
  char jsonString[50];
  snprintf(jsonString, sizeof(jsonString), "{\"kategori\": %d, \"jarak\": %d}", kategori, jarak);
  client.publish(TOPIC_BIN_DATA, jsonString);
}

void detectAndSortWaste() {
  int logamDetected = digitalRead(proxyPin);
  int anorganikDetected = digitalRead(irPin);
  int organikDetected = digitalRead(rainDigitalPin);

  
  long organikDistance = readUltrasonicDistance(trigOrganikPin, echoOrganikPin);
  long anorganikDistance = readUltrasonicDistance(trigAnorganikPin, echoAnorganikPin);
  long bahayaDistance = readUltrasonicDistance(trigBahayaPin, echoBahayaPin);

  if (logamDetected == LOW) {
    directionServo.write(130);
    delay(2000);
    myservo.write(150);
    delay(2000);
    myservo.write(40);
    publishData(3, bahayaDistance); 
  } else if (organikDetected == LOW) {
    directionServo.write(58);
    delay(2000);
    myservo.write(150);
    delay(2000);
    myservo.write(40);
    publishData(1, organikDistance); 
  } else if (anorganikDetected == LOW) {
    directionServo.write(0);
    delay(2000);
    myservo.write(150);
    delay(2000);
    myservo.write(40);
    publishData(2, anorganikDistance); 
  }
}

void publishBinFullness() {
  
  long organikDistance = readUltrasonicDistance(trigOrganikPin, echoOrganikPin);
  long anorganikDistance = readUltrasonicDistance(trigAnorganikPin, echoAnorganikPin);
  long bahayaDistance = readUltrasonicDistance(trigBahayaPin, echoBahayaPin);

  if (organikDistance <= 10) {
    publishData(1, organikDistance);
  }
  if (anorganikDistance <= 10) {
    publishData(2, anorganikDistance);
  }
  if (bahayaDistance <= 10) {
    publishData(3, bahayaDistance);
  }
}