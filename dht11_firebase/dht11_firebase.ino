#include <WiFi.h>
#include <FirebaseESP32.h>
#include "DHT.h"

// WiFi credentials
#define WIFI_SSID "myUUM_Guest"
#define WIFI_PASSWORD ""

// Firebase database URL and secret key
#define FIREBASE_HOST "https://esp-firebase-demo-a15c2-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH ""  // Replace with your Database secret (or access token)

// DHT setup
#define DHTPIN 4          // DHT sensor connected to GPIO 4
#define DHTTYPE DHT11     // or DHT22
DHT dht(DHTPIN, DHTTYPE);

// Firebase object
FirebaseData firebaseData;

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Connect to Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Read temperature and humidity
  float temp = dht.readTemperature(); // Celsius
  float hum = dht.readHumidity();

  // Check if reading failed
  if (isnan(temp) || isnan(hum)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return;
  }

  // Print to Serial
  Serial.printf("Temperature: %.2f °C | Humidity: %.2f %%\n", temp, hum);

  // Send to Firebase
  if (Firebase.setFloat(firebaseData, "/dht/temperature", temp)) {
    Serial.println("Temperature sent to Firebase");
  } else {
    Serial.print("Error sending temperature: ");
    Serial.println(firebaseData.errorReason());
  }

  if (Firebase.setFloat(firebaseData, "/dht/humidity", hum)) {
    Serial.println("Humidity sent to Firebase");
  } else {
    Serial.print("Error sending humidity: ");
    Serial.println(firebaseData.errorReason());
  }

  // Delay before next reading
  delay(5000); // every 5 seconds
}
