#include "DHTesp.h"
#include "PubSubClient.h"
#include "WiFi.h"

const int DHT_PIN = 15;
const int LED_HIJAU_PIN = 27;
const int LED_KUNING_PIN = 14;
const int LED_MERAH_PIN = 12;
const int RELAY_POMPA_PIN = 2;
const int BUZZER_PIN = 13;

const char *ssid = "Wokwi-GUEST";
const char *pass = "";
const char *broker = "broker.hivemq.com";
const char *subs_pompa = "152022168/pompa";
const char *pub_suhu = "152022168/suhu";
const char *pub_humi = "152022168/humi";
const int brokerPort = 1883;

String macAddr;
String ipAddr;

DHTesp dhtSensor;
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

void WifiSetup()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  ipAddr = WiFi.localIP().toString().c_str();
  macAddr = WiFi.macAddress();

  Serial.print("Connected at: ");
  Serial.println(ipAddr);
  Serial.print("MAC: ");
  Serial.println(macAddr);
}

void MqttSetup()
{
  mqtt.setServer(broker, brokerPort);
  mqtt.setCallback(Callback);
}

void Reconnect()
{
  Serial.println("Connecting to MQTT Broker...");
  while (!mqtt.connected())
  {
    Serial.println("Reconnecting to MQTT Broker..");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str()))
    {
      Serial.print("ID: ");
      Serial.println(clientId);
      Serial.println("Connected.");
      mqtt.subscribe(subs_pompa);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void Callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Incoming topic: ");
  Serial.println(topic);

  String _messageStr;
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    _messageStr += (char)message[i];
  }
  Serial.println();

  if (String(topic) == subs_pompa)
  {
    _messageStr.toLowerCase();
    Serial.print("Pump Relay Status: ");

    if (_messageStr == "1")
    {
      Serial.println("Pompa ON");
      digitalWrite(RELAY_POMPA_PIN, HIGH);
    }
    else if (_messageStr == "0")
    {
      Serial.println("Pompa OFF");
      digitalWrite(RELAY_POMPA_PIN, LOW);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_HIJAU_PIN, OUTPUT);
  pinMode(LED_KUNING_PIN, OUTPUT);
  pinMode(LED_MERAH_PIN, OUTPUT);
  pinMode(RELAY_POMPA_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  WifiSetup();
  MqttSetup();

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
}

void loop()
{
  if (!mqtt.connected())
  {
    Reconnect();
  }

  mqtt.loop();

  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  float t = data.temperature;
  float h = data.humidity;

  if (t > 35)
  {
    digitalWrite(LED_MERAH_PIN, HIGH);
    digitalWrite(LED_KUNING_PIN, LOW);
    digitalWrite(LED_HIJAU_PIN, LOW);
    digitalWrite(BUZZER_PIN, HIGH);
  }
  else if (t >= 30 && t <= 35)
  {
    digitalWrite(LED_MERAH_PIN, LOW);
    digitalWrite(LED_KUNING_PIN, HIGH);
    digitalWrite(LED_HIJAU_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
  else
  {
    digitalWrite(LED_MERAH_PIN, LOW);
    digitalWrite(LED_KUNING_PIN, LOW);
    digitalWrite(LED_HIJAU_PIN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
  }

  char _t[8];
  dtostrf(t, 1, 2, _t);
  mqtt.publish(pub_suhu, _t);
  Serial.print("Pub Temp: ");
  Serial.println(_t);

  char _h[8];
  dtostrf(h, 1, 2, _h);
  mqtt.publish(pub_humi, _h);
  Serial.print("Pub Hum: ");
  Serial.println(_h);

  delay(1000);
}
