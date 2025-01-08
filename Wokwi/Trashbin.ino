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


const int trigOrganikPin = 12;   // Trig untuk sampah organik
const int echoOrganikPin = 13;   // Echo untuk sampah organik
const int trigAnorganikPin = 14; // Trig untuk sampah anorganik
const int echoAnorganikPin = 15; // Echo untuk sampah anorganik
const int trigBahayaPin = 19;    // Trig untuk sampah berbahaya
const int echoBahayaPin = 21;    // Echo untuk sampah berbahaya

bool personDetected = false;      // Status deteksi orang
bool doorOpened = false;          // Status pintu utama
bool wasteProcessed = false;      // Status pemrosesan sampah
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
  

  // Deteksi orang
  long personDistance = readUltrasonicDistance(trigPersonPin, echoPersonPin);
  if (personDistance > 0 && personDistance <= 20 && !personDetected) {
    personDetected = true;
    personDetectedTime = currentMillis;
    Serial.println("Orang terdeteksi, membuka pintu utama.");
    mainDoorServo.write(90);  // Membuka pintu utama
    doorOpened = true;
  }

  // Tutup pintu utama setelah 5 detik
  if (doorOpened && currentMillis - personDetectedTime >= 5000) {
    mainDoorServo.write(0);  // Menutup pintu utama
    doorOpened = false;
    personDetected = false; // Reset status deteksi orang
  }

  // Proses deteksi sampah berjalan terus-menerus
  detectAndSortWaste();

  // Cek apakah tong sampah penuh
  checkBinFullness();
}

// Fungsi membaca jarak dari sensor ultrasonik
long readUltrasonicDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2; // Konversi durasi ke jarak dalam cm
}

// Fungsi mendeteksi dan menyortir sampah
void detectAndSortWaste() {
  int logamDetected = digitalRead(proxyPin);
  int anorganikDetected = digitalRead(irPin);
  int organikDetected = digitalRead(rainDigitalPin);

  if (logamDetected == LOW) {
    Serial.println("Sampah logam terdeteksi");
    directionServo.write(130); // Posisi logam
    delay(2000);
    myservo.write(150);
    delay(2000);
    myservo.write(40);
  } 
  else if (organikDetected == LOW) {
    Serial.println("Sampah organik terdeteksi");
    directionServo.write(58); // Posisi organik
    delay(2000);
    myservo.write(150);
    delay(2000);
    myservo.write(40);
  } 
  else if (anorganikDetected == LOW) {
    Serial.println("Sampah anorganik terdeteksi");
    directionServo.write(0); // Posisi anorganik
    delay(2000);
    myservo.write(150);
    delay(2000);
    myservo.write(40);
  }
}

// Fungsi untuk memeriksa apakah tong sampah penuh
void checkBinFullness() {
  long organikDistance = readUltrasonicDistance(trigOrganikPin, echoOrganikPin);
  long anorganikDistance = readUltrasonicDistance(trigAnorganikPin, echoAnorganikPin);
  long bahayaDistance = readUltrasonicDistance(trigBahayaPin, echoBahayaPin);

  if (organikDistance > 0 && organikDistance <= 10) {
    Serial.println("Tong sampah organik penuh!");
  }

  if (anorganikDistance > 0 && anorganikDistance <= 10) {
    Serial.println("Tong sampah anorganik penuh!");
  }

  if (bahayaDistance > 0 && bahayaDistance <= 10) {
    Serial.println("Tong sampah berbahaya penuh!");
  }
}