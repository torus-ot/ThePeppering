# First stage of The Peppering project is completed.
## THE CODE
  ### What the code (and Arduino) can do now:  
    1. Read two Soil Moisture Sensors with provided time interval   
    2. Output the table of last 10 analog reads from sensors to the web page.  
    3. Output the digital reads from sensors -Alarms   
    4. Allow user to change time interval from the web page.  
    5. Show Alarms status on the LED matrix buid in with Arduino UNO R4 WiFi  
    6. ouput text information to the Serial  
  Hopefuly I will have time and patience to write extended description to the code.

## THE HARDWARE:
  * Arduino UNO R4 WiFI  - https://www.amazon.com/dp/B0C8V88Z9D  
  * HiLetgo Soil Moisture Detect Sensors - https://www.amazon.com/dp/B01DKISKLO  
  (no referal in amazon's links - I don't need no #jeffbezos money :grin:)  

## KNOWN PROBLEMS
  1. For some reasons my WiFi router refuses to give my Arduino the dynamic IP address.  
  All I am getting is 0.0.0.0 (???)  I am still looking for the answer. Is it in my code?  
  For now I am using static IP address.
  2. I am not happy with resistive moisture sensors accuracy.Will play with capacitive sensors.


# Were are we going?
  - [x] The sensors are working with Arduino, Web page is generated, LED matrix is showing Alarms.
  - [ ] Connect pamp to the project. Write code to work with pump.
  - [ ] Fork code to work with Capacitive sensors and ESP32 controller.
  - [ ] Think how to collect data from multiple controllers(Blutooth, MQTT, may b ESP-NOW???).
  - [ ] From breadboard to real live - No coding, but still challenging.
