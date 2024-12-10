#include "DHTesp.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "PubSubClient.h"

const int DHT_PIN = 15;
const int LED_HIJAU_PIN = 27;
const int LED_KUNING_PIN = 14;
const int LED_MERAH_PIN = 12;
const int RELAY_POMPA_PIN = 2;
const int BUZZER_PIN = 13;

#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET 25200
#define UTC_OFFSET_DST 25200

String baseUrl = "https://e89d-180-244-132-182.ngrok-free.app";

const char *ssid = "Wokwi-GUEST";
const char *pass = "";
const char *broker = "broker.hivemq.com";
const char *subs_pompa = "152022168/pompa";
const char *pub_suhu = "152022168/suhu";
const char *pub_humi = "152022168/humi";
const int brokerPort = 1883;

String ipAddr;

DHTesp dhtSensor;
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

void setup()
{
  Serial.begin(115200);

  pinMode(LED_HIJAU_PIN, OUTPUT);
  pinMode(LED_KUNING_PIN, OUTPUT);
  pinMode(LED_MERAH_PIN, OUTPUT);
  pinMode(RELAY_POMPA_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  ipAddr = WiFi.localIP().toString().c_str();

  Serial.print("Connected at: ");
  Serial.println(ipAddr);

  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);

  MqttSetup();

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
}

void printLocalTime(float _suhu, float _humid)
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Connection Err");
    return;
  }

  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  char _jam[3];
  char _menit[3];
  char _detik[3];

  strftime(_jam, 3, "%H", &timeinfo);
  strftime(_menit, 3, "%M", &timeinfo);
  strftime(_detik, 3, "%S", &timeinfo);

  if (strcmp(_jam, "14") == 0 && strcmp(_detik, "00") == 0)
  {
    for (int i = 57; i <= 59; i++)
    {
      char menit[3];
      sprintf(menit, "%02d", i);
      if (strcmp(_menit, menit) == 0)
      {
        SendData(_suhu, _humid);
        break;
      }
    }
  }
}

void SendData(float temp, float hum)
{
  HTTPClient http;

  String url = baseUrl + "/API/insertdata.php?suhu=" + temp + "&humid=" + hum;
  Serial.println("url: " + String(url));

  http.begin(url.c_str());

  int httpResponseCode = http.GET();

  String payload = http.getString();
  Serial.print("paylod: ");
  Serial.println(payload);

  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);
  }
  else
  {
    Serial.print("Error Code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
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

  printLocalTime(t, h);

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
