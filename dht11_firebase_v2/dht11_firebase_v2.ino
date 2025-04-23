/************************************************************
 * Project: ESP32 + DHT11 + Firebase (Realtime DB) + Relay
 * Features:
 *   - Connects to WiFi (myUUM_Guest)
 *   - Reads Temp & Humidity from DHT11
 *   - Sends data to Firebase
 *   - Reads relay control value (0/1) from Firebase
 *   - Turns relay ON/OFF accordingly
 *   - Avoids re-triggering relay if status hasn't changed
 *   - Includes epoch timestamp with DHT data
 *
 * Required Libraries:
 *   - Firebase ESP Client v4.4.6 by Mobizt
 *   - DHT sensor library v1.4.4 by Adafruit
 *   - Adafruit Unified Sensor v1.1.9
 *   - ESP32 Board Package v2.0.9 or later
 ************************************************************/

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "DHT.h"
#include <time.h>  // For NTP time

// WiFi credentials
#define WIFI_SSID "myUUM_Guest"
#define WIFI_PASSWORD ""

// Firebase credentials
#define API_KEY "AIzaSyDLhNBs1q-ICr4o4LnGxxTX--bUjj9NxV8"
#define DATABASE_URL "https://esp-firebase-demo-a15c2-default-rtdb.firebaseio.com/"

// DHT sensor setup
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Relay setup
#define RELAY_PIN 2
int previousRelayStatus = -1;  // Initial unknown state

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Firebase helper files
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // OFF initially (for active LOW relay)

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Initialize NTP
  configTime(28800, 0, "pool.ntp.org", "time.nist.gov");  // Malaysia UTC+8
  Serial.println("Waiting for NTP time sync...");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("\nTime synchronized");

  // Firebase setup
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = "slumberjer@gmail.com";
  auth.user.password = "H3QUyMHigZDmPH3";

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Authenticating with Firebase...");
  while (auth.token.uid == "") {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nFirebase Ready!");
}

void loop() {
  // Refresh token if needed
  if (!Firebase.ready() && Firebase.isTokenExpired()) {
    Serial.println("Token expired. Restarting Firebase...");
    Firebase.begin(&config, &auth);
    Serial.println("Token refreshed successfully.");
    return;
  }

  // Skip loop if token is not ready
  if (!Firebase.ready()) {
    Serial.println("Skipping loop: Firebase token not ready");
    // Firebase.refreshToken() is void in latest library, no return value to check
    delay(1000);
    return;
  }
  static unsigned long lastDhtUpdate = 0;
  unsigned long currentMillis = millis();

  // Update DHT data every 10 seconds
  if (currentMillis - lastDhtUpdate >= 10000) {
    lastDhtUpdate = currentMillis;

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    time_t now = time(nullptr);  // Get current epoch time

    if (!isnan(temperature) && !isnan(humidity)) {
      Serial.printf("Temp: %.2f°C | Humidity: %.2f%%", temperature, humidity);

      Firebase.RTDB.setFloat(&fbdo, "/103/dht/temperature", temperature);
      Firebase.RTDB.setFloat(&fbdo, "/103/dht/humidity", humidity);
      Firebase.RTDB.setInt(&fbdo, "/103/dht/timestamp", now);
    } else {
      Serial.println("⚠️ DHT sensor read failed.");
      Firebase.RTDB.setString(&fbdo, "/103/errors/dht", "Failed to read from DHT sensor");
    }
  }

  // Get relay control value from Firebase (expecting 0 or 1)
  if (Firebase.RTDB.getInt(&fbdo, "/103/relay/status")) {
    int relayStatus = fbdo.intData();
    Serial.print("Relay command from Firebase: ");
    Serial.println(relayStatus);

    if (relayStatus != previousRelayStatus) {
      if (relayStatus == 1) {
        digitalWrite(RELAY_PIN, LOW);
        Serial.println("Relay turned ON");
      } else {
        digitalWrite(RELAY_PIN, HIGH);
        Serial.println("Relay turned OFF");
      }

      previousRelayStatus = relayStatus;
      //Firebase.RTDB.setInt(&fbdo, "/103/relay/status", relayStatus);
    } else {
      Serial.println("Relay status unchanged, no action taken.");
    }
  } else {
    String errorMsg = fbdo.errorReason();
    Serial.print("Failed to read relay status: ");
    Serial.println(errorMsg);
    Firebase.RTDB.setString(&fbdo, "/103/errors/relay", errorMsg);
  }
}
