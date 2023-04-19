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
const byte warningLED = 3;
const byte cautionLED = 27;

AsyncUDP udp;
UltraSonicDistanceSensor sonar(triggerSonarPin, echoSonarPin);

void setup() {
  Serial.begin(9600);
  
  pinMode(warningLED, OUTPUT);
  pinMode(cautionLED, OUTPUT);

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
            if(strcmp((char*)packet.data(),"CAUTION") == 0)
            {
              Serial.print("CAUTION");
              digitalWrite(cautionLED, HIGH);
            }
        });
    }
    
}


void loop() {
  delay(200);
  //Send broadcast on port 10000
  //udp.broadcastTo("Anyone here?", 10000);
  // breaking then send message
  // if have caution light and see car braking turn on WARNING light
  float distance = sonar.measureDistanceCm();
  if (0 < distance < 30)
  {
    udp.broadcastTo("CAUTION", 10000);
  }
  Serial.println(distance);
}

