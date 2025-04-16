/************************************************************
 * Project: ESP32 + DHT11 + Firebase (Realtime DB)
 * Features:
 *   - Connects to WiFi (myUUM_Guest)
 *   - Reads Temperature & Humidity from DHT11
 *   - Sends data to Firebase Realtime Database
 *
 * Compatible with:
 *   - Firebase ESP Client v4.4.6 (by Mobizt)
 *   - DHT sensor library v1.4.4 (by Adafruit)
 *   - Adafruit Unified Sensor v1.1.9
 *   - ESP32 Board package v2.0.9 or newer
 ************************************************************/

#include <WiFi.h>                    // Built-in ESP32 WiFi
#include <Firebase_ESP_Client.h>     // Firebase ESP Client v4.4.6
#include "DHT.h"                     // DHT sensor library v1.4.4

// WiFi credentials
#define WIFI_SSID "myUUM_Guest"
#define WIFI_PASSWORD ""

// Firebase credentials
#define API_KEY "YOUR_FIREBASE_API_KEY"  // from Firebase Console > Project Settings
#define DATABASE_URL "https://esp-firebase-demo-a15c2-default-rtdb.firebaseio.com/"  // without "https://" at end

// DHT sensor setup
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Firebase helper files (included with the library)
#include "addons/TokenHelper.h"   // For token generation
#include "addons/RTDBHelper.h"    // For handling RTDB response

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // No email/password used (anonymous login)
  auth.user.email = "";
  auth.user.password = "";

  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Wait for Firebase to authenticate
  Serial.println("Authenticating with Firebase...");
  while (auth.token.uid == "") {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nFirebase Ready!");
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (!isnan(temperature) && !isnan(humidity)) {
    Serial.printf("Temperature: %.2f°C | Humidity: %.2f%%\n", temperature, humidity);

    // Send temperature to Firebase
    if (Firebase.RTDB.setFloat(&fbdo, "/dht/temperature", temperature)) {
      Serial.println("✔️ Temperature sent to Firebase");
    } else {
      Serial.print("❌ Temp Error: ");
      Serial.println(fbdo.errorReason());
    }

    // Send humidity to Firebase
    if (Firebase.RTDB.setFloat(&fbdo, "/dht/humidity", humidity)) {
      Serial.println("✔️ Humidity sent to Firebase");
    } else {
      Serial.print("❌ Humidity Error: ");
      Serial.println(fbdo.errorReason());
    }
  } else {
    Serial.println("⚠️ Failed to read from DHT sensor");
  }

  delay(5000); // Wait 5 seconds
}
