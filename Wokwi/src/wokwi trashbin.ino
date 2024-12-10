#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_task_wdt.h>

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

const char *ssid = "Wokwi-GUEST";
const char *password = " ";
const char *mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
const char *TOPIC_DOOR_CONTROL = "trashbin/pintu";
const char *TOPIC_BIN_DATA = "trashbin/data";
WiFiClient espClient;
PubSubClient client(espClient);

RTC_DATA_ATTR int bootCount = 0;
const int WDT_TIMEOUT = 30; // watchdog timeout in seconds

void callback(char *topic, byte *payload, unsigned int length)
{
  if (String(topic) == TOPIC_DOOR_CONTROL)
  {
    if ((char)payload[0] == '1')
    {
      mainDoorServo.write(90);
      doorOpened = true;
    }
    else if ((char)payload[0] == '0')
    {
      mainDoorServo.write(0);
      doorOpened = false;
    }
  }
}

void setup()
{
  Serial.begin(115200);
  esp_task_wdt_init(WDT_TIMEOUT, true); // enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);               // add current thread to WDT watch

  bootCount++;
  Serial.println("Boot number: " + String(bootCount));

  // Add delay for system stabilization
  delay(1000);

  // Initialize pins
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

  // Initialize servos with error checking
  if (!myservo.attach(servoPin))
  {
    Serial.println("Failed to attach main servo");
    ESP.restart();
  }
  if (!directionServo.attach(directionServoPin))
  {
    Serial.println("Failed to attach direction servo");
    ESP.restart();
  }
  if (!mainDoorServo.attach(mainDoorServoPin))
  {
    Serial.println("Failed to attach main door servo");
    ESP.restart();
  }

  // Set initial positions with delays
  myservo.write(40);
  delay(500);
  directionServo.write(90);
  delay(500);
  mainDoorServo.write(0);
  delay(500);

  WiFi.mode(WIFI_STA);   // Set WiFi to station mode
  WiFi.disconnect(true); // Disconnect from any previous WiFi
  delay(1000);

  // Connect to WiFi
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);

  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20)
  {
    delay(500);
    Serial.print(".");
    wifiAttempts++;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("\nWiFi connection failed! Restarting...");
    ESP.restart();
  }

  Serial.println("\nWiFi connected");

  // Connect to MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  connectMQTT();
}

void connectMQTT()
{
  int mqttAttempts = 0;
  while (!client.connected() && mqttAttempts < 3)
  {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("SmartBinClient"))
    {
      Serial.println("Connected to MQTT Broker");
      client.subscribe(TOPIC_DOOR_CONTROL);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(1000);
      mqttAttempts++;
    }
  }
}

void loop()
{
  esp_task_wdt_reset(); // Reset watchdog timer

  static unsigned long lastWifiCheck = 0;
  static unsigned long lastMqttCheck = 0;

  // Check connections periodically
  unsigned long currentMillis = millis();

  if (currentMillis - lastWifiCheck >= 30000)
  { // Check WiFi every 30 seconds
    lastWifiCheck = currentMillis;
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("WiFi reconnecting...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);
      delay(1000);
    }
  }

  if (currentMillis - lastMqttCheck >= 5000)
  { // Check MQTT every 5 seconds
    lastMqttCheck = currentMillis;
    if (!client.connected())
    {
      connectMQTT();
    }
  }

  client.loop();

  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi disconnected. Reconnecting...");
    WiFi.begin(ssid, password);
    delay(5000);
    return;
  }

  unsigned long currentMillis = millis();
  long personDistance = readUltrasonicDistance(trigPersonPin, echoPersonPin);

  if (personDistance > 0 && personDistance <= 20 && !personDetected)
  {
    personDetected = true;
    personDetectedTime = currentMillis;
    mainDoorServo.write(90);
    doorOpened = true;
  }

  if (doorOpened && currentMillis - personDetectedTime >= 5000)
  {
    mainDoorServo.write(0);
    doorOpened = false;
    personDetected = false;
  }

  detectAndSortWaste();
  publishBinFullness();
}

long readUltrasonicDistance(int trigPin, int echoPin)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;
}

void publishData(int kategori, int jarak)
{
  char jsonString[50];
  snprintf(jsonString, sizeof(jsonString), "{\"kategori\": %d, \"jarak\": %d}", kategori, jarak);
  client.publish(TOPIC_BIN_DATA, jsonString);
}

void detectAndSortWaste()
{
  int logamDetected = digitalRead(proxyPin);
  int anorganikDetected = digitalRead(irPin);
  int organikDetected = digitalRead(rainDigitalPin);

  long organikDistance = readUltrasonicDistance(trigOrganikPin, echoOrganikPin);