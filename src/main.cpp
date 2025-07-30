// Project: Peppering (Plant Monitor)
// Board: Arduino UNO R4 WiFi
// Framework: Arduino (PlatformIO)
#define DEBUG_NOWIF


#include <Arduino.h>
#include <Arduino_LED_Matrix.h>

#ifndef DEBUG_NOWIFI
#include <WiFiS3.h>
#endif

#include "sensors.h"
#include "uno_matrix.h"
#include "secret_wifi.h"
//
// FIle "secret_wifi.h" should have these two lines:
// #define SECRET_WIFI "Name_Of_Your_WiFi"
// #define SECRET_PSWD "Password_For_Your_WiFI"
// Place the file in your 'include' folder
//

// WiFi credentials
const char* ssid = SECRET_WIFI;
const char* password = SECRET_PSWD;

#define MAXLINE 200

#ifndef DEBUG_NOWIFI
WiFiServer server(80);
WiFiClient client;
#endif

// Sensor 1 configuration
// const int sensor1PwrPin = 7;
// const int sensor1AnlPin = A0;
// const int sensor1DgtPin = 2;

// Sensor 2 configuration
// const int sensor2PwrPin = 8;
// const int sensor2AnlPin = A1;
// const int sensor2DgtPin = 3;

//extern const int MAX_READINGS;

// arrays vor reading
/*extern int sensorReadings[MAX_SENSORS][MAX_READINGS];     // Analog values
extern int sensorDigital[MAX_SENSORS];                    // Latest digital states
extern unsigned long aTimestamps[MAX_READINGS]; */     

MoistureSensor sensors[] = {
  {7, A0, 2, 0, "Pot A"},           // Resistive sensor, alarmValue is ignored
  {8, A1, A1, 500, "Pot B"},        // Capacitive sensor - immitating digital pin with alarm value
  // {9, A2, A2, 400, "Test"}, 
  // Add more...
};
const int iNumSensors = sizeof(sensors) / sizeof(sensors[0]);

ArduinoLEDMatrix matrix; 
//int readings1[MAX_READINGS] = {0};
// int readings2[MAX_READINGS] = {0};
// unsigned long timestamps[MAX_READINGS] = {0};
int dgValue1 = 0;
int dgValue2 = 0;
int iAlert;


unsigned long lastSampleTime = 0;
#ifndef DEBUG_NOWIFI 
unsigned long sampleInterval = 60000;    // 1min //3600000; // 1 hour in milliseconds
#else
const unsigned long sampleInterval = 300000; // 15 min. in milliseconds
#endif


char* TimestampOut(char* sTime, int iMaxL, unsigned long ms) {
  unsigned long seconds = ms / 1000;
  unsigned long hours = (seconds / 3600) % 24;
  unsigned long minutes = (seconds / 60) % 60;
  unsigned long secs = seconds % 60;

  snprintf(sTime, iMaxL, "%02lu:%02lu:%02lu", hours, minutes, secs);
  return sTime;
}


char* formatRow(char* buf, size_t maxLen, int iVal1, int iVal2, unsigned long ms) {
  char sTime[20];
  TimestampOut(sTime, sizeof(sTime), ms);
  snprintf(buf, maxLen, "<tr align='right'><td>%s</td><td>%d</td><td>%d</td></tr>", sTime, iVal1, iVal2);
  return buf;
}


char* formatRowEx(char* buf, size_t maxLen, int iIndx, unsigned long ms) 
{ // build row for sensors
  #define MAX_CELL 20
  char sCell[MAX_CELL];
  char sTime[MAX_CELL];
  String  sRow = "<tr align='right'>";
  TimestampOut(sTime, sizeof(sTime), ms);
  snprintf(sCell, MAX_CELL-1, "<td>%s</td>", sTime);
  sRow += sCell;
  for (int n = 0; n < MAX_SENSORS; n++) {   // add sensors reading into cells
    snprintf(sCell, MAX_CELL-1, "<td>%d</td>", sensorReadings[n][iIndx]);
    sRow += sCell;
  }
  sRow += "</tr>";        // close the table row
  sRow.toCharArray(buf, maxLen);
  buf[maxLen-1] = '\0';     // safety null termination
  return buf;
}


void OuputTable(int arr1[], int arr2[], unsigned long tsArr[]) 
{
  /*char* sAlert1;
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
      formatRow(sLine, iMaxLine, arr1[i], arr2[i], tsArr[i]);
      client.println(sLine);
    }
    // bottom raw - digital
    sAlert1 = (dgValue1 == 1) ? sBad : sOK;
    sAlert2 = (dgValue2 == 1) ? sBad : sOK;
    snprintf(sLine, iMaxLine, "<tr align='right'><th>Alarm</th><td valign='top'>%s</td><td valign='top'>%s</td></tr>", sAlert1,sAlert2);
    client.println(sLine); 
    client.println("</table>\r\n</p>\r\n"); 
    client.println("\r\n<p style='clear:both'>Low Value is wet, Higher Value is dry. <\p>\r\n"); 
  }
*/
}


void TableHeader() 
{ // output table header
  char buf[64];
  if (client) {
    client.print("<tr><th>Time elapsed</th>");
    for (int i = 0; i < MAX_SENSORS; i++) {
        snprintf(buf, sizeof(buf), "<th>%s</th>", sensors[i].name);
        client.print(buf);
    }
    client.println("</tr>");
  }
}


void digitalRow() 
{ // Output Alarms row 
  char* sAlert;
  char* sOK =  "&#128994;";     // "OK";    //
  char* sBad = "&#10060;";      // "!!!";   // 
  char buf[64];
 
    if (client) {
    client.print("<tr align='right'><th>Alarm</th>");
    for (int i = 0; i < MAX_SENSORS; i++) {
      sAlert = (sensorDigital[i] == 1) ? sBad : sOK;
      snprintf(buf, sizeof(buf), "<th>%s</th>", sAlert);
      client.print(buf);
    }
    client.println("</tr>");
  }

}


void OuputTableEx() 
{
  char sLine[MAXLINE];
  int iMaxLine = MAXLINE;

  // WiFiClient client = server.available();   // called in the WebOutput()
  if (client) {                                                 // Output:
    client.println("<p><table border='1' align='left'>");       // table
    TableHeader();                                              // table header
    for (int i = 0; i < MAX_READINGS; i++) {                    // raws
      formatRowEx(sLine, iMaxLine, i, aTimestamps[i]);
      client.println(sLine);
    }
    // bottom raw - digital
    digitalRow();
    client.println("</table>\r\n</p>\r\n"); 
    client.println("\r\n<p style='clear:both'>Low Value is wet, Higher Value is dry. <\p>\r\n"); 
  }

}


void OutputTime() {
  // ouput local time - best if added just before </body> tag
  if (client) {
    client.println("<div>Page was refreshed on: <span id='clock'></span></div>");
    client.println("<script>const tD = document.getElementById('clock');");
    client.println("function updateTime() {const now = new Date(); const fT = now.toLocaleTimeString(); tD.textContent = fT;}");
    client.println("updateTime(); </script>");
    client.println("\r\n");
  }
}


void shiftReadingsUp() {
  /*for (int i = 0; i < MAX_READINGS - 1; i++) {
    readings1[i] = readings1[i + 1];
    readings2[i] = readings2[i + 1];
    timestamps[i] = timestamps[i + 1];
  }*/
}


void shiftReadingsUpEx(unsigned long ms) {
  // first - shift all values up one line
  for (int i = 0; i < MAX_READINGS - 1; i++) {
    for (int n = 0; n < MAX_SENSORS; n++) {
      sensorReadings[n][i] = sensorReadings[n][i + 1];
    }
    // sensorDigital[i] = sensorDigital[i + 1];
    aTimestamps[i] = aTimestamps[i + 1];
  }
  // second - read all values into the last line
  for (int n = 0; n < MAX_SENSORS; n++) {
      int aPin = sensors[n].analogPin;
      int dPin = sensors[n].digitalPin;
      sensorReadings[n][MAX_READINGS - 1] = analogRead(sensors[n].analogPin);                                 // read analog
      sensorDigital[n] = digitalReadEx(sensors[n].digitalPin, sensors[n].analogPin, sensors[n].alarmValue);   // read digital
  }
  aTimestamps[MAX_READINGS-1] = ms;                                                                           // save timestamp
}


void printMoistSensor(unsigned long timestamp, int anValue, int dgValue) {
  char buffer[80];
  char sTime[20];
  TimestampOut(sTime, sizeof(sTime), timestamp);

  snprintf(buffer, sizeof(buffer),
           "[%s] Moisture: analog=%d, digital=%s",
           sTime,
           anValue,
           (dgValue == LOW ? "WET" : "DRY"));

  Serial.println(buffer);
}


void printMoistSensorEx(int nSensor, int indx) {
  char buffer[MAXLINE];
  char sTime[20];
  int dgValue = sensorDigital[nSensor];
  TimestampOut(sTime, sizeof(sTime), aTimestamps[indx]);

  snprintf(buffer, sizeof(buffer),
           "Sensor %s: [%s] Moisture: analog=%d, digital=%s", sensors[nSensor].name, 
           sTime, sensorReadings[nSensor][indx], (dgValue == LOW ? "WET" : "DRY"));

  Serial.println(buffer);
}


int initMoistSensor(int pwrPin, int anlPin, int dgPin) {
  /*pinMode(pwrPin, OUTPUT);
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
  }*/
}


int initMoistSensorEx() 
{
  int pwrPin, anlPin, dgPin;
  char buffer[100];
  int retCode = 0;
  for (int i = 0; i < MAX_SENSORS; i++) {
    pwrPin = sensors[i].powerPin;
    anlPin = sensors[i].analogPin;
    dgPin = sensors[i].digitalPin;
    pinMode(pwrPin, OUTPUT);
    pinMode(anlPin, INPUT);
    pinMode(dgPin, INPUT);
    digitalWrite(pwrPin, HIGH);         // power on       
    delay(200);                         // allow sensor to stabilize// power on
    // try to read sensor
    int analogVal = analogRead(anlPin);
    int digitalVal = digitalReadEx(dgPin, anlPin, sensors[i].alarmValue);
    // check for stupid
    if ( analogVal > 10 && analogVal < 1013 ) {
      snprintf(buffer, sizeof(buffer), "✅ Sensor %s (%d, %d, %d) is on: %d", 
              sensors[i].name, pwrPin, anlPin - A0, digitalVal, analogVal);
      Serial.println(buffer);
      retCode += 1;
    } else {
      snprintf(buffer, sizeof(buffer), "⚠️ Sensor %s (%d, %d, %d) has failed: %d", 
              sensors[i].name, pwrPin, anlPin - A0, digitalVal, analogVal);
      Serial.println(buffer);
    }
  }
  if (retCode < MAX_SENSORS) {
    return 0;
  } else {
    return 1;
  } 

}

void setup() {
  Serial.begin(9600);           // connect to terminal

  iAlert = 999;

  matrix.begin();               // init matrix on the board

  IPAddress local_ip(192, 168, 1, 193);    // your desired static IP
  IPAddress gateway(192, 168, 1, 1);       // usually your router
  IPAddress subnet(255, 255, 255, 0);      // standard home subnet
  WiFi.config(local_ip, gateway, subnet);

  // init all moisture sensors
  //int ok1 = initMoistSensor(sensor1PwrPin, sensor1AnlPin, sensor1DgtPin);
  //int ok2 = initMoistSensor(sensor2PwrPin, sensor2AnlPin, sensor2DgtPin);
  int ok = initMoistSensorEx();


  // Try to connect - 10 times 
  #ifndef DEBUG_NOWIFI
  int iCount = 0;
   while (WiFi.begin(ssid, password) != WL_CONNECTED && iCount < 10 ) {
    Serial.println("Connecting to WiFi...");
    iCount++;
    delay(500);
  }
  delay(1000);          // if we want DHCP give as IP address
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

    client.println("<!DOCTYPE html><html><head><meta charset='utf-8'>");
    client.println("<meta http-equiv='refresh' content='1800'>");       // refreshh in 30 min
    client.println("<title>Peppering Monitor</title></head><body>");
    client.println("<h2>Soil Moisture Readings</h2>");
    
    // Form for the form HTML part
    snprintf(sRequest, sizeof(sRequest),
        "<form action='/' method='GET'><p> New Interval (ms): "
        "<input type='number' name='interval' size='10' value='%lu'> "
        "<input type='submit' value='Set'></p></form>",
        sampleInterval);
    client.println(sRequest);

    // OuputTable(readings1, readings2, timestamps);           // HTML print data table
    // OuputTable(sensorReadings[0], sensorReadings[1], aTimestamps);
    OuputTableEx();                                         // print new table
    OutputTime();                                           // HTML print the table

    client.println("</body></html>");
    
    client.flush();
    delay(100);
    client.stop();
  }
 
}


void loop() {
  unsigned long now = millis();
  int iNewAlert;

  // Do this for now
  dgValue1 = sensorDigital[0];   
  dgValue2 = sensorDigital[1];

  iNewAlert = dgValue1 + 2*dgValue2;
  if (iAlert  != iNewAlert ) {      // output matrix
    iAlert = iNewAlert;
    ShowIconById(static_cast<IconId>(iAlert));
    // debug 
    Serial.print("iAlert=");
    Serial.println(iAlert); Serial.println(dgValue1); Serial.println(dgValue2);
    Serial.println("\r\n");
  }
 
  if (now - lastSampleTime >= sampleInterval || lastSampleTime == 0) {
    lastSampleTime = now;

    // powering sensors up
    //digitalWrite(sensor1PwrPin, HIGH);
    // digitalWrite(sensor2PwrPin, HIGH);
    // delay(200);       // allow sensors to stabilize before reading
 
    for (int i = 0; i < MAX_SENSORS; i++) {       // power up all sensors
      digitalWrite(sensors[i].powerPin, HIGH); 
    }
    delay(200); 

    shiftReadingsUp();
    // readings1[MAX_READINGS - 1] = analogRead(sensor1AnlPin);
    // readings2[MAX_READINGS - 1] = analogRead(sensor2AnlPin);
    // timestamps[MAX_READINGS - 1] = now;
    // old digital read
    // dgValue1 = digitalRead(sensor1DgtPin);
    // dgValue2 = digitalRead(sensor2DgtPin);
    // dgValue2 = (dgValue2>750) ? HIGH : LOW ;
   
    // new new analog and digital read
    shiftReadingsUpEx(now);           // shift values up, read new values and save them to the last row

    for (int n = 0; n < MAX_SENSORS; n++) {
      digitalWrite(sensors[n].powerPin, LOW);                                                               // power off the sensor
      printMoistSensorEx(n, MAX_READINGS-1);                                                                // output to terminal                                                
    }
 
    // powering sensors off
    //digitalWrite(sensor1PwrPin, LOW);
    // digitalWrite(sensor2PwrPin, LOW);
 
    // Debug outputt to the terminal
    //printMoistSensor(timestamps[MAX_READINGS - 1], readings1[MAX_READINGS - 1], dgValue1);
    //printMoistSensor(timestamps[MAX_READINGS - 1], readings2[MAX_READINGS - 1], dgValue2);
  }

  if (WiFi.status() == WL_CONNECTED) {
    
    webOutput();
  }
  else {      // No WiFi
    for (int i = 0; i < MAX_SENSORS; i++) {
      printMoistSensorEx(i, MAX_READINGS-1); 
    }
  }
  
  delay(100); // brief pause to avoid tight loop
}
