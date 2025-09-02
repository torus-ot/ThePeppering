#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

const int MAX_READINGS = 10;
const int MAX_SENSORS = 6;     // up to 4 realistically 
//const char cRes =  'R';
//const char cCap = 'C';

struct MoistureSensor {
  int powerPin;      // Digital pin to control power
  int analogPin;     // Analog pin for analog read
  int digitalPin;    // Digital pin (or analog reused)
  int alarmValue;    // Threshold for capacitive sensors
  const char* name;  // Sensor label
};

// arrays vor reading
int sensorReadings[MAX_SENSORS][MAX_READINGS] = {0};     // Analog values
int sensorDigital[MAX_SENSORS] = {0};                    // Latest digital states
unsigned long aTimestamps[MAX_READINGS] = {0};           // array of Timestamp s


// Try it inline
inline int digitalRead(int dgPin, int anPin, int alarmValue) {
     if ( dgPin == anPin ) {         // dgPin is not real! imitate reading
        return (analogRead(anPin) > alarmValue) ? HIGH : LOW;
    } else {
        return digitalRead(dgPin);
    }
}

#endif // SENSORS_H