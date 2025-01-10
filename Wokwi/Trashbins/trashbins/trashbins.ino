#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "King";
const char* password = "devit123";
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_topic_status = "trashbin/data";
const char* mqtt_topic_people = "trashbin/people";
const char* mqtt_topic_control = "trashbin/control";

WiFiClient espClient;
PubSubClient client(espClient);

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
int peopleCount = 0;

void setupWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      client.subscribe(mqtt_topic_control);
    } else {
      delay(1000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  if (String(topic) == mqtt_topic_control) {
    if (message == "open") {
      mainDoorServo.write(150); // Membuka pintu
    } else if (message == "close") {
      mainDoorServo.write(45); // Menutup pintu
    }
  }
}

void setup() {
  Serial.begin(115200);
  setupWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

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
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();
  long personDistance = readUltrasonicDistance(trigPersonPin, echoPersonPin);

  if (personDistance > 0 && personDistance <= 5 && !personDetected) {
    personDetected = true;
    personDetectedTime = currentMillis;
    peopleCount++;
    mainDoorServo.write(150);
    doorOpened = true;
    String peopleMessage = String(peopleCount);
    client.publish(mqtt_topic_people, peopleMessage.c_str());
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
    directionServo.write(130);
    delay(2000);
    myservo.write(140);
    delay(2000);
    myservo.write(40);
  } else if (organikDetected == LOW) {
    directionServo.write(58);
    delay(2000);
    myservo.write(140);
    delay(2000);
    myservo.write(40);
  } else if (anorganikDetected == LOW) {
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
    client.publish(mqtt_topic_status, "Tong sampah organik penuh!");
  } else if (organikDistance > 5 && organikDistance <= 10) {
    client.publish(mqtt_topic_status, "Tong sampah organik hampir penuh!");
  }

  if (anorganikDistance > 0 && anorganikDistance <= 5) {
    client.publish(mqtt_topic_status, "Tong sampah anorganik penuh!");
  } else if (anorganikDistance > 5 && anorganikDistance <= 10) {
    client.publish(mqtt_topic_status, "Tong sampah anorganik hampir penuh!");
  }

  if (bahayaDistance > 0 && bahayaDistance <= 5) {
    client.publish(mqtt_topic_status, "Tong sampah berbahaya penuh!");
  } else if (bahayaDistance > 5 && bahayaDistance <= 10) {
    client.publish(mqtt_topic_status, "Tong sampah berbahaya hampir penuh!");
  }
}