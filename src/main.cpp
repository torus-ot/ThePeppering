// Project: Peppering (Plant Monitor)
// Board: Arduino UNO R4 WiFi
// Framework: Arduino (PlatformIO)
#define DEBUG_NOWIF


#include <Arduino.h>
#include <Arduino_LED_Matrix.h>

#ifndef DEBUG_NOWIFI
#include <WiFiS3.h>
#endif

#include "secret_wifi.h"
#include "uno_matrix.h"


// WiFi credentials
const char* ssid = SECRET_WIFI;
const char* password = SECRET_PSWD;


#ifndef DEBUG_NOWIFI
WiFiServer server(80);
#endif

// Sensor 1 configuration
const int sensor1PwrPin = 7;
const int sensor1AnlPin = A0;
const int sensor1DgtPin = 2;

// Sensor 2 configuration
const int sensor2PwrPin = 8;
const int sensor2AnlPin = A1;
const int sensor2DgtPin = 3;

const int MAX_READINGS = 10;
int iAlert1 = 0;
int iAlert2 = 0;
ArduinoLEDMatrix matrix; 
int readings1[MAX_READINGS] = {0};
int readings2[MAX_READINGS] = {0};
unsigned long timestamps[MAX_READINGS] = {0};
int dgValue1;
int dgValue2;

unsigned long lastSampleTime = 0;
#ifndef DEBUG_NOWIFI 
const unsigned long sampleInterval = 3600000; // 1 hour in milliseconds
#else
const unsigned long sampleInterval = 10000; // 10 sec. in milliseconds
#endif

String formatTimestamp(unsigned long ms) {
  unsigned long seconds = ms / 1000;
  unsigned long hours = (seconds / 3600) % 24;
  unsigned long minutes = (seconds / 60) % 60;
  unsigned long secs = seconds % 60;

  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, secs);
  return String(buffer);
}

String formatCell(int iValue) {
  String result =  "<td valign='top'>" + String(iValue) + "</td></tr>";
  return result;
}

String formatDualReadings(int arr1[], int arr2[], unsigned long tsArr[]) {
  String result = "<table border='1'><tr><th>Sensor 1</th><th>Sensor 2</th></tr>";
  for (int i = 0; i < MAX_READINGS; i++) {
    result += "<tr><td valign='top'>"  + formatTimestamp(tsArr[i]) + "</td>" ;
    if (tsArr[i] > 0 ) {
      result += formatCell(arr1[i]) + formatCell(arr2[i]) + "</td></tr>";
    }
    else {
      result += "<td></td><td></td></tr>";        // no values yet
    }
  }
  // output current digital values (Alarm?)
  result += "<tr><td valign='top'>Alarm 1 = " + String(dgValue1) + "</td><td valign='top'>Alarm 2 = " + String(dgValue2) + "</td></tr>";
  result += "</table>";
  return result;
}

void shiftReadingsUp() {
  for (int i = 0; i < MAX_READINGS - 1; i++) {
    readings1[i] = readings1[i + 1];
    readings2[i] = readings2[i + 1];
    timestamps[i] = timestamps[i + 1];
  }
}

void printMoistSensor(unsigned long timestamp, int anValue, int dgValue) {
  char buffer[80];
  String timeStr = formatTimestamp(timestamp);

  snprintf(buffer, sizeof(buffer),
           "[%s] Moisture: analog=%d, digital=%s",
           timeStr.c_str(),
           anValue,
           (dgValue == LOW ? "DRY" : "WET"));

  Serial.println(buffer);
}
/*void ShowAlert(int iAlert) {
  static const uint32_t SMILEY[8] = {
    B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10100101,
    B10011001,
    B01000010,
    B00111100
  };

  static const uint32_t EXCLAMATION[8] = {
    B00011000,
    B00011000,
    B00011000,
    B00011000,
    B00011000,
    B00000000,
    B00011000,
    B00011000
  };

// Left-side centered "!"
const uint32_t EXCLAMATION_LEFT[8] = {
  B00001100,
  B00001100,
  B00001100,
  B00001100,
  B00001100,
  B00000000,
  B00001100,
  B00001100
};

// Right-side centered "!"
const uint32_t EXCLAMATION_RIGHT[8] = {
  B00110000,
  B00110000,
  B00110000,
  B00110000,
  B00110000,
  B00000000,
  B00110000,
  B00110000
};

// Both sides "!"
const uint32_t EXCLAMATION_BOTH[8] = {
  B00111100,
  B00111100,
  B00111100,
  B00111100,
  B00111100,
  B00000000,
  B00111100,
  B00111100
};


  uint32_t buffer[8] = {0};

  if (iAlert < 0) {
    matrix.clear();
    return;
  }

  if (iAlert == 0) {
    matrix.loadFrame(SMILEY);
    return;
  }

  if (iAlert == 1 || iAlert == 3) {
    for (int row = 0; row < 8; row++) {
      buffer[row] |= EXCLAMATION[row] >> 2; // left half
    }
  }

  if (iAlert == 2 || iAlert == 3) {
    for (int row = 0; row < 8; row++) {
      buffer[row] |= EXCLAMATION[row] << 4; // right half
    }
  }

  matrix.loadFrame(buffer);
}
*/

int initMoistSensor(int pwrPin, int anlPin, int dgPin) {
  pinMode(pwrPin, OUTPUT);
  pinMode(anlPin, INPUT);
  pinMode(dgPin, INPUT);

  digitalWrite(pwrPin, HIGH);
  delay(200);  // allow sensor to stabilize

  int analogVal = analogRead(anlPin);
  int digitalVal = digitalRead(dgPin);

  bool isAnalogOK = (analogVal > 10 && analogVal < 1013);

  char buffer[100];

  if (isAnalogOK) {
    snprintf(buffer, sizeof(buffer), "✅ Sensor on A%d: analog=%d, digital=%d",
             anlPin - A0, analogVal, digitalVal);
    Serial.println(buffer);
    return 1;
  } else {
    snprintf(buffer, sizeof(buffer),
             "⚠️ Sensor on A%d failed to initialize. Analog=%d",
             anlPin - A0, analogVal);
    Serial.println(buffer);
    return 0;
  }
}


void setup() {
  Serial.begin(9600);           // connect to terminal

  matrix.begin();               // init matrix on the board

  // init both misture sensors
  int ok1 = initMoistSensor(sensor1PwrPin, sensor1AnlPin, sensor1DgtPin);
  int ok2 = initMoistSensor(sensor2PwrPin, sensor2AnlPin, sensor2DgtPin);


  // Try to connect - 10 times 
  #ifndef DEBUG_NOWIFI
  int iCount = 0;
   while (WiFi.begin(ssid, password) != WL_CONNECTED && iCount < 10 ) {
    Serial.println("Connecting to WiFi...");
    iCount++;
    delay(3000);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    server.begin();
  }
  else {
    Serial.println("Cannot connect to WiFi - trying without it");
  }
#endif

}

// ouput page
void webOutput() {
    WiFiClient client = server.available();
    if (client) {
      String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Peppering Monitor</title></head><body>";
      html += "<h2>Soil Moisture Readings (last 10 hours)</h2>";
      html += formatDualReadings(readings1, readings2, timestamps);
      html += "</body></html>";

      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();
      client.println(html);

      delay(10);
      client.stop();
    }
 
}

void loop() {
  unsigned long now = millis();
  int iAlert;

  if (iAlert  != iAlert1 + iAlert2 ) {      // output matrix
    iAlert = iAlert1 + iAlert2;
    ShowIconById(static_cast<IconId>(iAlert));
  }
 
  if (now - lastSampleTime >= sampleInterval || lastSampleTime == 0) {
    lastSampleTime = now;

    // powering sensors up
    digitalWrite(sensor1PwrPin, HIGH);
    digitalWrite(sensor2PwrPin, HIGH);
    delay(200);       // allow sensors to stabilize before reading
 
    shiftReadingsUp();
    readings1[MAX_READINGS - 1] = analogRead(sensor1AnlPin);
    readings2[MAX_READINGS - 1] = analogRead(sensor2AnlPin);
    timestamps[MAX_READINGS - 1] = now;
    dgValue1 = digitalRead(sensor1DgtPin);
    dgValue2 = digitalRead(sensor2DgtPin);

    // powering sensors off
    digitalWrite(sensor1PwrPin, LOW);
    digitalWrite(sensor2PwrPin, LOW);
 
    // Debug outputt to the terminal
    Serial.print("Sensor1: "); Serial.print(readings1[MAX_READINGS - 1]);
    Serial.print(" | Sensor2: "); Serial.println(readings2[MAX_READINGS - 1]);
  }

  if (WiFi.status() == WL_CONNECTED) {
    webOutput();
  }
  else {      // No WiFi
    printMoistSensor(timestamps[MAX_READINGS - 1], readings1[MAX_READINGS - 1], dgValue1);
    printMoistSensor(timestamps[MAX_READINGS - 1], readings2[MAX_READINGS - 1], dgValue2);

  }
  
  delay(100); // brief pause to avoid tight loop
}
