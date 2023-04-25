#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <AsyncUDP.h>
#include <Adafruit_VL53L0X.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <secrets.h>

// set to false if accelerometer is not present
#define hasAccel true

#ifdef hasAccel
#include <Adafruit_LSM9DS1.h>
#endif

//comment out if ultrasonic is not present (are we even using the ultasonic?)
#define hasUltra true

#ifdef hasUltra
#include <HCSR04.h>
#endif

//moved SSID + PASSWORD to secrets.h
/* 
const char * SSID = "ATT9nvMeRy";
const char * PASSWORD = "t82sxamdz2#5";
*/
const byte triggerSonarPin = 12;
const byte echoSonarPin = 13;
const byte warningLED = 26;
const byte cautionLED = 27;
const byte receivingLED = 2;
String myID;

//tuple for local array of network cars
struct communicatingCar {
  unsigned long time;
  String carID;
};

//packet contancts
struct packetInfo {
  unsigned int carID;
  unsigned int tofData;
  unsigned int accelData;
};

//we only have 2 devices right now so 3 max should be fine
struct communicatingCar cars[3];

//timers and booleans
volatile unsigned long currentTime;
unsigned long sendTimer;
unsigned long warningTimer;
volatile unsigned long cautionTimer;
//yes I did a bad by switching to snake_case
volatile bool warning_led_enabled = false;
volatile bool caution_led_enabled = false;

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

void readSensor(int * data)
{

  sensors_event_t a, m, g, temp;

  lsm.getEvent(&a, &m, &g, &temp); 

  data[0] = a.acceleration.x;
  data[1] = a.acceleration.y;
  data[2] = a.acceleration.z;
  
  // Serial.print("Accel X: "); Serial.print(a.acceleration.x); Serial.print(" m/s^2");
  // Serial.print("\tY: "); Serial.print(a.acceleration.y);     Serial.print(" m/s^2 ");
  // Serial.print("\tZ: "); Serial.print(a.acceleration.z);     Serial.println(" m/s^2 ");

  // Serial.print("Mag X: "); Serial.print(m.magnetic.x);   Serial.print(" uT");
  // Serial.print("\tY: "); Serial.print(m.magnetic.y);     Serial.print(" uT");
  // Serial.print("\tZ: "); Serial.print(m.magnetic.z);     Serial.println(" uT");

  // Serial.print("Gyro X: "); Serial.print(g.gyro.x);   Serial.print(" rad/s");
  // Serial.print("\tY: "); Serial.print(g.gyro.y);      Serial.print(" rad/s");
  // Serial.print("\tZ: "); Serial.print(g.gyro.z);      Serial.println(" rad/s");
  return;
}


void setup() {
  Serial.begin(115200);

  //need to call millis once to initialize for some reason
  sendTimer = millis();

  Serial.println("VL53L0X Online");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }

  if (hasAccel == true)
  {
    Serial.println("LSM9DS1 Online");
    if (!lsm.begin())
    {
      Serial.println("Failed to boot LSM9DS1.");
      while (1);
    }

     setupSensor();
  }
  
  // power 
  
  pinMode(warningLED, OUTPUT);
  pinMode(cautionLED, OUTPUT);
  pinMode(receivingLED, OUTPUT);

  WiFi.begin(SSID, PASSWORD);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi Failed");
        while(1) {
            delay(1000);
        }
    }

  myID = WiFi.macAddress().substring(15);

  if(udp.listen(10000)) {
        udp.onPacket([](AsyncUDPPacket packet) {
            // turn on the LED once and leave on
            // this is the onboard led (next to power)
            digitalWrite(receivingLED, HIGH);

            Serial.print("Data: ");
            Serial.write(packet.data(), packet.length());
            Serial.print("\n");
            
            //status msg logic
            //ADD HERE (collect car ID and time received in an array)

            //CAUTION msg
            if(strcmp((char*)packet.data(),"CAUTION") == 0)
            {
              //Serial.print("CAUTION");
              digitalWrite(cautionLED, HIGH);
              cautionTimer = millis();
              caution_led_enabled = true;
            }

        });
    }
    
}


void loop() {

  //IF millis() works we can remove this delay
  delay(200);

  currentTime = millis();
  //Send broadcast on port 10000
  //udp.broadcastTo("Anyone here?", 10000);

  //int distance = sonar.measureDistanceCm();
  VL53L0X_RangingMeasurementData_t distance;
  lox.rangingTest(&distance, false);

  //logic to turn off caution LED
  if(caution_led_enabled)
  {
    currentTime = millis();
    if (currentTime > (cautionTimer + ((unsigned long)5000)))
    {
      digitalWrite(cautionLED,LOW);
      caution_led_enabled = false;
    }
  }

  if (hasAccel == true)
  {
    int data[3];
    readSensor(data);
  }
  
  //status msg (send all the data)
  if(currentTime > (sendTimer + ((unsigned long) 3000)))
  {
    udp.broadcastTo("Status Msg",10000);
    sendTimer = millis();
  }

  //send caution if accel changes drastically (or negative acceleration) (maybe don't allow more than a couple messages per event (w/ timers+booleans))

  // if caution light on (caution_light_enabled) and ToF sees something (change from average values or within range) turn on warning light (can also turn on warning light if accel changes)
  /*
  // if distance is less than 300 and acceleration suddenly decreases, turn on the WARNING LIGHT
  if (0 < (int)distance.RangeMilliMeter and (int)distance.RangeMilliMeter < 300)
  {
    udp.broadcastTo(String(distance.RangeMilliMeter).c_str(), 10000);
  }
  */
}

