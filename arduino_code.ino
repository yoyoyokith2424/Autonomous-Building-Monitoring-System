#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "*******";  // Your WiFi SSID
const char* password = "***********";  // Your WiFi password
const char* serverUrl = "http://192.168.77.79:5000/status";  // Flask server URL
const char* updatePinStatusUrl = "http://192.168.77.79:5000/update_pin_status";  // Pin status endpoint

int ledPin1 = 26;
int ledPin2 = 27;
int ledPin3 = 12;
int ledPin4 = 14;
int inputPin = 21;
int abcd = 34;  // Manual switch pin (GPIO 34)
int flameSensorPin = A0;  // Flame sensor pin

void setup() {
  Serial.begin(115200);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);
  pinMode(inputPin, INPUT);

  // Configure the manual switch pin as INPUT_PULLUP
  pinMode(abcd, INPUT_PULLUP);  // Enable internal pull-up resistor on GPIO 34

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Get the LED status from Flask server
    http.begin(serverUrl);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server Response: " + response);

      // Parse JSON response
      DynamicJsonDocument doc(512);
      deserializeJson(doc, response);

      // Read the manual switch (abcd pin status)
      int manualSwitchStatus = digitalRead(abcd);
     
      // Check manual switch first, if LOW (pressed), turn on LED1
      if (manualSwitchStatus == LOW) {
        turnOnLedPin1();
      } else {
        // If manual switch is HIGH (not pressed), follow the dashboard status
        if (String(doc["led1"]) == "ON") {
          turnOnLedPin1();
        } else {
          turnOffLedPin1();
        }
      }

      // Update other LEDs based on the server status
      digitalWrite(ledPin2, doc["led2"] == "ON" ? HIGH : LOW);
      digitalWrite(ledPin3, doc["led3"] == "ON" ? HIGH : LOW);
      digitalWrite(ledPin4, doc["led4"] == "ON" ? HIGH : LOW);

      // Send input pin status and flame sensor data to the server
      int pinStatus = digitalRead(inputPin);
      int flameSensorValue = readFlameSensor();
      sendSensorData(pinStatus, flameSensorValue);
    } else {
      Serial.println("Error on HTTP request");
    }
    http.end();
  }
  delay(1000);
}

// Separate function to turn on ledPin1
void turnOnLedPin1() {
  digitalWrite(ledPin1, HIGH);
  Serial.println("LED1 is ON");
}

// Separate function to turn off ledPin1
void turnOffLedPin1() {
  digitalWrite(ledPin1, LOW);
  Serial.println("LED1 is OFF");
}

// Function to read flame sensor data
int readFlameSensor() {
  unsigned int sensorValue = analogRead(flameSensorPin);
  unsigned int outputValue = map(sensorValue, 0, 1023, 0, 255); // map the data to 8-bit data
  return outputValue;
}

// Function to send both pin status and flame sensor value
void sendSensorData(int pinStatus, int flameValue) {
  HTTPClient http;
  http.begin(updatePinStatusUrl);
  http.addHeader("Content-Type", "application/json");

  // Create JSON payload
  DynamicJsonDocument doc(128);
  doc["pinStatus"] = pinStatus ? "ON" : "OFF";
  doc["flameSensor"] = flameValue;
  String requestBody;
  serializeJson(doc, requestBody);

  int httpResponseCode = http.POST(requestBody);
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Data sent. Response: " + response);
  } else {
    Serial.println("Failed to send data");
  }
  http.end();
}