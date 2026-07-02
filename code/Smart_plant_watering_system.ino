// 1. YOUR BLYNK CODES
#define BLYNK_TEMPLATE_ID "TMPL3PZrU5LGa"
#define BLYNK_TEMPLATE_NAME "Smart Plant System"
#define BLYNK_AUTH_TOKEN "SYYnZGeGSZ8JHR0Nqrz-lTi9ruvKk7Gm"

// 2. PUT YOUR WI-FI DETAILS HERE
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWARD";

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <time.h> // Built-in library to track time over Wi-Fi

// --- PHYSICAL PIN DEFINITIONS ---
const int soilMoisturePin = A0; // Moisture Sensor
const int relayPin = D1;        // Relay for Water Pump
const int ledPin = D5;          // Indicator LED

// --- AUTOMATION THRESHOLD ---
const int drySoilThreshold = 700; 

// --- TIME CONFIGURATION (IST) ---
// India is UTC +5:30. In seconds: (5 * 3600) + (30 * 60) = 19800 seconds
const long timezoneOffsetSeconds = 19800; 
const int daylightSavingsOffsetSeconds = 0; // India does not use DST

BlynkTimer timer;

void setup() {
  Serial.begin(115200);
  
  pinMode(relayPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  
  digitalWrite(relayPin, LOW); 
  digitalWrite(ledPin, LOW);   

  Serial.println("Connecting to WiFi and Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Configure time synchronization using internet time servers (NTP)
  configTime(timezoneOffsetSeconds, daylightSavingsOffsetSeconds, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for internet time sync...");

  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}

// --- SEND DATA TO CLOUD & AUTOMATION ---
void sendSensorData() {
  // 1. Soil Moisture & Pump Automation Logic
  int moistureValue = analogRead(soilMoisturePin);
  Blynk.virtualWrite(V0, moistureValue);
  
  if (moistureValue > drySoilThreshold) {
    digitalWrite(relayPin, HIGH); 
    digitalWrite(ledPin, HIGH);   
    Blynk.virtualWrite(V1, 1);    
    Serial.println("Soil is DRY! Turning pump ON.");
  } else {
    digitalWrite(relayPin, LOW);  
    digitalWrite(ledPin, LOW);    
    Blynk.virtualWrite(V1, 0);    
    Serial.println("Soil is WET. Turning pump OFF.");
  }
  
  // 2. Internet Time Tracking Logic
  time_t now = time(nullptr);
  struct tm* timeInfo = localtime(&now);

  // Only check time if the chip successfully synced with the internet
  if (timeInfo->tm_year > 70) { 
    int currentHour = timeInfo->tm_hour; // Gets current hour in 24-hour format (0 to 23)
    int currentMinute = timeInfo->tm_min;

    Serial.printf("Current Internet Time: %02d:%02d\n", currentHour, currentMinute);

    // Define Daytime as 6:00 AM (6) to 6:00 PM (18)
    if (currentHour >= 6 && currentHour < 18) {
      Blynk.virtualWrite(V2, "Daytime");
      Serial.println("Status: Daytime");
    } else {
      Blynk.virtualWrite(V2, "Nighttime");
      Serial.println("Status: Nighttime");
    }
  } else {
    Serial.println("Time not synced yet...");
  }
}

// --- MANUAL OVERRIDE FROM CLOUD ---
BLYNK_WRITE(V1) {
  int buttonState = param.asInt(); 
  if (buttonState == 1) {
    digitalWrite(relayPin, HIGH); 
    digitalWrite(ledPin, HIGH);   
    Serial.println("Manual override: ON");
  } else {
    digitalWrite(relayPin, LOW);  
    digitalWrite(ledPin, LOW);    
    Serial.println("Manual override: OFF");
  }
}