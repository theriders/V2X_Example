#include <Arduino.h>
#include <WiFi.h>
#include <AsyncUDP.h>
//#include <HCSR04.h>
//#include <secrets.h>
#include <Adafruit_VL53L0X.h>
#include <SPI.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>

const char * SSID = "ATT9nvMeRy";
const char * PASSWORD = "t82sxamdz2#5";
const byte triggerSonarPin = 12;
const byte echoSonarPin = 13;
const byte warningLED = 3;
const byte cautionLED = 27;

AsyncUDP udp;
//UltraSonicDistanceSensor sonar(triggerSonarPin, echoSonarPin);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

void setupSensor()
{
  // 1.) Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_4G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_8G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_16G);
  
  // 2.) Set the magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_8GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_12GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_16GAUSS);

  // 3.) Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_500DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_2000DPS);
}

String readSensor()
{
  char * data = "";

  sensors_event_t a, m, g, temp;

  lsm.getEvent(&a, &m, &g, &temp); 

  
  // Serial.print("Accel X: "); Serial.print(a.acceleration.x); Serial.print(" m/s^2");
  data = "Accel X: ";// + String(a.acceleration.x).c_str()).c_str();


  // Serial.print("\tY: "); Serial.print(a.acceleration.y);     Serial.print(" m/s^2 ");
  // Serial.print("\tZ: "); Serial.print(a.acceleration.z);     Serial.println(" m/s^2 ");

  // Serial.print("Mag X: "); Serial.print(m.magnetic.x);   Serial.print(" uT");
  // Serial.print("\tY: "); Serial.print(m.magnetic.y);     Serial.print(" uT");
  // Serial.print("\tZ: "); Serial.print(m.magnetic.z);     Serial.println(" uT");

  // Serial.print("Gyro X: "); Serial.print(g.gyro.x);   Serial.print(" rad/s");
  // Serial.print("\tY: "); Serial.print(g.gyro.y);      Serial.print(" rad/s");
  // Serial.print("\tZ: "); Serial.print(g.gyro.z);      Serial.println(" rad/s");
  

 

  return data;
}


void setup() {
  Serial.begin(115200);

  Serial.println("VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }

  Serial.println("LSM9DS1 test");
  
  // Try to initialise and warn if we couldn't detect the chip
  if (!lsm.begin())
  {
    Serial.println("Oops ... unable to initialize the LSM9DS1. Check your wiring!");
    while (1);
  }

  setupSensor();

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

