#include <Arduino.h>
#include <WiFi.h>
#include <AsyncUDP.h>
#include <HCSR04.h>
//#include <secrets.h>
//#include <Adafruit_VL53L0X.h>

const char * SSID = "***";
const char * PASSWORD = "***";
const byte triggerSonarPin = 12;
const byte echoSonarPin = 13;

AsyncUDP udp;
UltraSonicDistanceSensor sonar(triggerSonarPin, echoSonarPin);

void setup() {
  Serial.begin(9600);
  
  WiFi.begin(SSID, PASSWORD);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi Failed");
        while(1) {
            delay(1000);
        }
    }

  if(udp.listen(10000)) {
        udp.onPacket([](AsyncUDPPacket packet) {
            Serial.print("Data: ");
            Serial.write(packet.data(), packet.length());
            Serial.print("\n");
            if(strcmp((char*)packet.data(),"Hello ESP") == 0)
            {
              Serial.print("WARNING");
            }
        });
    }
    
}


void loop() {
  delay(200);
  //Send broadcast on port 10000
  //udp.broadcastTo("Anyone here?", 10000);
  float distance = sonar.measureDistanceCm();
  //Serial.println(distance);
}

