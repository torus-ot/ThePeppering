#include <Arduino.h>

// Project: Peppering (Plant Monitor)
// Board: Arduino UNO R4 WiFi
// Framework: Arduino (PlatformIO)

#include <WiFiS3.h>

#include "uno_matrix.h"



// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";



WiFiServer server(80);

const int sensor1Pin = A0;
const int sensor2Pin = A1;

const int MAX_READINGS = 10;
int iAlert1 = 0;
int iAlert2 = 0;

int readings1[MAX_READINGS] = {0};
int readings2[MAX_READINGS] = {0};
unsigned long timestamps[MAX_READINGS] = {0};

unsigned long lastSampleTime = 0;
const unsigned long sampleInterval = 3600000; // 1 hour in milliseconds

String formatTimestamp(unsigned long ms) {
  unsigned long seconds = ms / 1000;
  unsigned long hours = (seconds / 3600) % 24;
  unsigned long minutes = (seconds / 60) % 60;
  unsigned long secs = seconds % 60;

  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, secs);
  return String(buffer);
}

String formatReadings(int arr[], unsigned long tsArr[]) {
  String result = "<table border='1'><tr><th>Time</th><th>Value</th></tr>";
  for (int i = 0; i < MAX_READINGS; i++) {
    if (tsArr[i] == 0) continue; // skip uninitialized rows
    result += "<tr><td>" + formatTimestamp(tsArr[i]) + "</td><td>" + String(arr[i]) + "</td></tr>";
  }
  result += "</table>";
  return result;
}

String formatDualReadings(int arr1[], int arr2[], unsigned long tsArr[]) {
  String result = "<table border='1'><tr><th>Sensor 1</th><th>Sensor 2</th></tr>";
  result += "<tr><td valign='top'>" + formatReadings(arr1, tsArr) + "</td><td valign='top'>" + formatReadings(arr2, tsArr) + "</td></tr>";
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

void setup() {
  Serial.begin(9600);           // connect to terminal

  matrix.begin();

  pinMode(sensor1Pin, INPUT);
  pinMode(sensor2Pin, INPUT);

  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
    delay(2000);
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  
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

    shiftReadingsUp();
    readings1[MAX_READINGS - 1] = analogRead(sensor1Pin);
    readings2[MAX_READINGS - 1] = analogRead(sensor2Pin);
    timestamps[MAX_READINGS - 1] = now;

    // Debug outputt to the terminal
    Serial.print("Sensor1: "); Serial.print(readings1[MAX_READINGS - 1]);
    Serial.print(" | Sensor2: "); Serial.println(readings2[MAX_READINGS - 1]);
  }

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

  delay(100); // brief pause to avoid tight loop
}
