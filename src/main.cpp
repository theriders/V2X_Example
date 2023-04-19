#include <Arduino.h>
#include <WiFi.h>
#include <AsyncUDP.h>
//#include <HCSR04.h>
//#include <secrets.h>
#include <Adafruit_VL53L0X.h>

const char * SSID = "ATT9nvMeRy";
const char * PASSWORD = "t82sxamdz2#5";
const byte triggerSonarPin = 12;
const byte echoSonarPin = 13;
const byte warningLED = 3;
const byte cautionLED = 27;

AsyncUDP udp;
//UltraSonicDistanceSensor sonar(triggerSonarPin, echoSonarPin);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  Serial.begin(115200);

  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }

  // power 
  
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
              //Serial.print("CAUTION");
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
  //int distance = sonar.measureDistanceCm();
  VL53L0X_RangingMeasurementData_t distance;
  lox.rangingTest(&distance, false);

  //Serial.print(distance.RangeMilliMeter);

  if (0 < distance.RangeMilliMeter < 800)
  {
    udp.broadcastTo(String(distance.RangeMilliMeter).c_str(), 10000);
  }
}

