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

#define MAXLINE 150

#ifndef DEBUG_NOWIFI
WiFiServer server(80);
WiFiClient client;
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


ArduinoLEDMatrix matrix; 
int readings1[MAX_READINGS] = {0};
int readings2[MAX_READINGS] = {0};
unsigned long timestamps[MAX_READINGS] = {0};
int dgValue1;
int dgValue2;

unsigned long lastSampleTime = 0;
#ifndef DEBUG_NOWIFI 
unsigned long sampleInterval = 60000;    // 5min //3600000; // 1 hour in milliseconds
#else
const unsigned long sampleInterval = 10000; // 10 sec. in milliseconds
#endif


char* TimestampOut(char* sTime, int iMaxL, unsigned long ms) {
  unsigned long seconds = ms / 1000;
  unsigned long hours = (seconds / 3600) % 24;
  unsigned long minutes = (seconds / 60) % 60;
  unsigned long secs = seconds % 60;

  snprintf(sTime, iMaxL, "%02lu:%02lu:%02lu", hours, minutes, secs);
  return sTime;
}


char* formatRaw(char* buf, size_t maxLen, int iVal1, int iVal2, unsigned long ms) {
  char sTime[20];
  TimestampOut(sTime, sizeof(sTime), ms);
  snprintf(buf, maxLen, "<tr align='right'><td>%s</td><td>%d</td><td>%d</td></tr>", sTime, iVal1, iVal2);
  return buf;
}


void OuputTable(int arr1[], int arr2[], unsigned long tsArr[]) 
{
  char* sAlert1;
  char* sAlert2;
  char* sOK =  "&#128994;";     // "OK";    //
  char* sBad = "&#10060;";      // "!!!";   // 
  char sLine[MAXLINE];
  int iMaxLine = MAXLINE;

  // WiFiClient client = server.available();   // called in the WebOutput()
  if (client) {
    client.println("<p><table border='1' align='left'>");                                    // table
    client.println("<tr><th>Time elapsed</th><th>Sensor 1</th><th>Sensor 2</th></tr>");   // header
    for (int i = 0; i < MAX_READINGS; i++) {                                              // raws
      formatRaw(sLine, iMaxLine, arr1[i], arr2[i], tsArr[i]);
      client.println(sLine);
    }
    // bottom raw - digital
    sAlert1 = (dgValue1 == 1) ? sOK : sBad;
    sAlert2 = (dgValue2 == 1) ? sOK : sBad;
    snprintf(sLine, iMaxLine, "<tr align='right'><th>Alarm</th><td valign='top'>%s</td><td valign='top'>%s</td></tr>", sAlert1,sAlert2);
    client.println(sLine); 
    client.println("</table></p>\r\n"); 
  }

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
  char sTime[20];
  TimestampOut(sTime, sizeof(sTime), timestamp);

  snprintf(buffer, sizeof(buffer),
           "[%s] Moisture: analog=%d, digital=%s",
           sTime,
           anValue,
           (dgValue == LOW ? "DRY" : "WET"));

  Serial.println(buffer);
}


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

  IPAddress local_ip(192, 168, 1, 193);    // your desired static IP
  IPAddress gateway(192, 168, 1, 1);       // usually your router
  IPAddress subnet(255, 255, 255, 0);      // standard home subnet
  WiFi.config(local_ip, gateway, subnet);

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
    Serial.print("Count was: "); Serial.println(String(iCount));
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
  char sRequest[MAXLINE] = {0};
  int iMaxLine = MAXLINE;

  /*WiFiClient*/ client = server.available();  
  if (client) {
    // ✅ Step 1: Read the HTTP request line
    client.readBytesUntil('\r', sRequest, sizeof(sRequest) - 1);
    client.read();  // Consume the '\n'

    // ✅ Step 2: Parse ?interval=value from the request
    char* paramPtr = strstr(sRequest, "interval=");
    if (paramPtr != NULL) {
      paramPtr += 9;    // sizeof "interval="
      int intervalValue = atoi(paramPtr);

      if (intervalValue >= 10000 && intervalValue <= 43200000) {  // Safety bounds: 10s to 12h
        sampleInterval = intervalValue;
      }
    }

    // ✅ Step 3: Respond with updated HTML page
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();

    client.println("<!DOCTYPE html><html><head><meta charset='utf-8'><title>Peppering Monitor</title></head><body>");
    client.println("<h2>Soil Moisture Readings</h2>");
    
    client.println("<form action='/' method='GET'><p> New Interval (ms): <input type='number' name='interval' value='3600000'><br> <input type='submit' value='Set'></p></form>");

    OuputTable(readings1, readings2, timestamps);

    client.println("</body></html>");
    
    client.flush();
    delay(100);
    client.stop();
  }
 
}


void loop() {
  unsigned long now = millis();
  int iAlert;

  if (iAlert  != !dgValue1 + !dgValue2 ) {      // output matrix
    iAlert = !dgValue1 + !dgValue2;
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
    printMoistSensor(timestamps[MAX_READINGS - 1], readings1[MAX_READINGS - 1], dgValue1);
    printMoistSensor(timestamps[MAX_READINGS - 1], readings2[MAX_READINGS - 1], dgValue2);
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
