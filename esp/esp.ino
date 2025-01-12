#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Nops";
const char* password = "";

const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_topic = "trashbin/data";
const char* mqtt_control_topic = "trashbin/pintu"; 

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
bool wasteProcessed = false;
unsigned long personDetectedTime = 0;
unsigned long lastMsg = 0;

void setup_wifi() {
  delay(10);
  Serial.println("\nConnecting to WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  
  if (strcmp(topic, mqtt_control_topic) == 0) {
    if (strcmp(message, "1") == 0) {
      mainDoorServo.write(150);
      doorOpened = true;
    } else if (strcmp(message, "0") == 0) {
      mainDoorServo.write(45);
      doorOpened = false;
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT connected");
      client.subscribe(mqtt_control_topic); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void sendWasteData(int category, long distance) {
  if (!client.connected()) {
    reconnect();
  }
  
  char message[50];
  snprintf(message, 50, "{\"kategori\": %d, \"jarak\": %ld}", category, distance);
  client.publish(mqtt_topic, message);
  Serial.printf("Published: %s\n", message);
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
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); 
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
    mainDoorServo.write(150);
    doorOpened = true;
  }

  if (doorOpened && currentMillis - personDetectedTime >= 5000) {
    mainDoorServo.write(45);
    doorOpened = false;
    personDetected = false;
  }

  detectAndSortWaste();
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
    myservo.write(150);
    delay(2000);
    myservo.write(40);
    sendWasteData(3, readUltrasonicDistance(trigBahayaPin, echoBahayaPin));
  } 
  else if (organikDetected == LOW) {
    directionServo.write(58);
    delay(2000);
    myservo.write(150);
    delay(2000);
    myservo.write(40);
    sendWasteData(1, readUltrasonicDistance(trigOrganikPin, echoOrganikPin));
  } 
  else if (anorganikDetected == LOW) {
    directionServo.write(0);
    delay(2000);
    myservo.write(150);
    delay(2000);
    myservo.write(40);
    sendWasteData(2, readUltrasonicDistance(trigAnorganikPin, echoAnorganikPin));
  }
}
