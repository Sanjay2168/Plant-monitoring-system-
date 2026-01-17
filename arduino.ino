#include <WiFiS3.h>
#include <DHT.h>
#include <Arduino_JSON.h>

// Wi-Fi credentials
const char* ssid = "Three_7AD76E";
const char* password = "2vLuswu235z3256";

// Raspberry Pi Flask server details
const char* server = "192.168.0.183"; // Replace with Raspberry Pi's IP
const int port = 5000;

// ThingSpeak details
const char* thingSpeakAPI = "5PEMMWETDEH3UM7G";
const char* thingSpeakHost = "api.thingspeak.com";
const int thingSpeakPort = 80;

// Sensor pins
const int soilMoisturePin = A0; // Soil Moisture Sensor
const int lightSensorPin = A1;  // Light Sensor
const int dhtPin = 2;           // DHT Sensor
const int buzzerPin = 3;        // Buzzer

// DHT sensor type
#define DHTTYPE DHT11
DHT dht(dhtPin, DHTTYPE);

// Variables for sensor data
int soilMoisture = 0;
int lightIntensity = 0;
float temperature = 0.0;
float humidity = 0.0;

WiFiClient client;

void setup() {
  Serial.begin(9600);

  // Initialize Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");

  // Initialize DHT sensor and buzzer
  dht.begin();
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  // Read sensor data
  soilMoisture = analogRead(soilMoisturePin);
  lightIntensity = analogRead(lightSensorPin);
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // Print sensor data to Serial Monitor
  Serial.print("Soil Moisture: ");
  Serial.println(soilMoisture);
  Serial.print("Light Intensity: ");
  Serial.println(lightIntensity);
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.println(" %");

  // Buzzer logic: Activate if soil moisture is below threshold
  if (soilMoisture <= 600) { // Adjust threshold as needed
    digitalWrite(buzzerPin, HIGH); // Turn on buzzer
    Serial.println("Buzzer ON: Soil Moisture Too Low!");
  } else {
    digitalWrite(buzzerPin, LOW); // Turn off buzzer
  }

  // Send data to Raspberry Pi Flask server
  sendToRaspberryPi();

  // Send data to ThingSpeak
  sendToThingSpeak();

  delay(5000); // Wait for 5 seconds before the next cycle
}

// Function to send data to Raspberry Pi Flask server
void sendToRaspberryPi() {
  JSONVar data;
  data["soilMoisture"] = soilMoisture;
  data["lightIntensity"] = lightIntensity;
  data["temperature"] = temperature;
  data["humidity"] = humidity;

  String jsonData = JSON.stringify(data);

  if (client.connect(server, port)) {
    Serial.println("Connected to Raspberry Pi");

    client.println("POST /data HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("Content-Type: application/json");
    client.println("Content-Length: " + String(jsonData.length()));
    client.println();
    client.println(jsonData);

    while (client.available()) {
      String response = client.readString();
      Serial.println("Raspberry Pi Response: " + response);
    }
    client.stop();
  } else {
    Serial.println("Failed to connect to Raspberry Pi");
  }
}

// Function to send data to ThingSpeak
void sendToThingSpeak() {
  if (WiFi.status() == WL_CONNECTED) {
    String url = "/update?api_key=" + String(thingSpeakAPI);
    url += "&field1=" + String(temperature);
    url += "&field2=" + String(humidity);
    url += "&field3=" + String(soilMoisture);
    url += "&field4=" + String(lightIntensity);

    Serial.println("Generated URL: " + url);

    if (client.connect(thingSpeakHost, thingSpeakPort)) {
      Serial.println("Connected to ThingSpeak");

      client.println("GET " + url + " HTTP/1.1");
      client.println("Host: " + String(thingSpeakHost));
      client.println("Connection: close");
      client.println();

      String response = "";
      while (client.available()) {
        response += client.readString();
      }
      Serial.println("ThingSpeak Response: " + response);

      if (response.indexOf("200 OK") != -1) {
        Serial.println("Data successfully sent to ThingSpeak!");
      } else {
        Serial.println("");
      }
      client.stop();
    } else {
      Serial.println("Failed to connect to ThingSpeak");
    }
  } else {
    Serial.println("Wi-Fi Disconnected");
  }
}
