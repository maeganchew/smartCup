#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>  // not used in this demo but required!
#include <CurieBLE.h>
#include <CurieTime.h>
#include <string.h>

// i2c
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

#define LSM9DS1_SCK A5
#define LSM9DS1_MISO 12
#define LSM9DS1_MOSI A4
#define LSM9DS1_XGCS 6
#define LSM9DS1_MCS 5
const int button = 2;
const int LED1 = 7;
const int LED2 = 6;
const int LED3 = 5;
int numPress = 1;
// You can also use software SPI
//Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(LSM9DS1_SCK, LSM9DS1_MISO, LSM9DS1_MOSI, LSM9DS1_XGCS, LSM9DS1_MCS);
// Or hardware SPI! In this case, only CS pins are passed in
//Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(LSM9DS1_XGCS, LSM9DS1_MCS);


BLEPeripheral blePeripheral;
BLEService imuService("917649A0-D98E-11E5-9EEC-0002A5D5C51B"); // Custom UUID
BLECharacteristic numDrinksCharacteristic("917649A2-D98E-11E5-9EEC-0002A5D5C51B", BLERead | BLENotify, 12 );
BLECharacteristic timeCharacteristic("917649A2-D98E-11E5-9EEC-0002A5D5C51B", BLERead | BLENotify, 12 );
BLEDescriptor numDrinksDescriptor("2902", "block");
BLEDescriptor timeDescriptor("2902", "block");

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


void setup() 
{
  Serial.begin(115200);

  while (!Serial) {
    delay(1); // will pause Zero, Leonardo, etc until serial console opens
  }
  
  Serial.println("LSM9DS1 data read demo");
  
  // Try to initialise and warn if we couldn't detect the chip
  if (!lsm.begin())
  {
    Serial.println("Oops ... unable to initialize the LSM9DS1. Check your wiring!");
    while (1);
  }
  Serial.println("Found LSM9DS1 9DOF");

  setTime(5, 54, 30, 7, 4, 2019);
  
  blePeripheral.setLocalName("AlcCup");
  Serial.println("Set Name");
  blePeripheral.setAdvertisedServiceUuid(imuService.uuid());  // add the service UUID
  blePeripheral.addAttribute(imuService);
  blePeripheral.addAttribute(numDrinksCharacteristic);
  blePeripheral.addAttribute(numDrinksDescriptor);
  blePeripheral.addAttribute(timeCharacteristic);
  blePeripheral.addAttribute(timeDescriptor);

  blePeripheral.begin();
  
  // helper to just set the default scaling we want, see above!
  setupSensor();
  pinMode(button, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  digitalWrite(LED1, 1);
}

void loop() 
{
  BLECentral central = blePeripheral.central();

  if(central){
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    while(central.connected()){
      lsm.read();  /* ask it to read in the data */ 

      /* Get a new sensor event */ 
      sensors_event_t a, m, g, temp;

      lsm.getEvent(&a, &m, &g, &temp); 
      if(digitalRead(button)){
        while(digitalRead(button)){};
        numPress = (numPress + 1) % 4;
        if(numPress == 0){
          digitalWrite(LED1, 0);
          digitalWrite(LED2, 0);
          digitalWrite(LED3, 0);
        }
        else if(numPress == 1){
          digitalWrite(LED1, 1);
        }
        else if(numPress == 2){
          digitalWrite(LED2, 1);
        }
        else if(numPress == 3){
          digitalWrite(LED3, 1);
        }
      }
      if((m.magnetic.y) > .44){
        while((m.magnetic.y) > .44){
          lsm.getEvent(&a, &m, &g, &temp); 
        }
        Serial.print(numPress);
        Serial.println(" shot(s) taken");
        const unsigned char numDrinksCharArray[2] = { 0, (char)numPress};
        const unsigned char timeCharArray[5] = { (char) hour()/10, (char) hour()%10, ':', (char) minute()/10, (char) minute()%10};
        numDrinksCharacteristic.setValue(numDrinksCharArray, 12);
        timeCharacteristic.setValue(timeCharArray, 12);
        numPress = 1;
        digitalWrite(LED2, 0);
        digitalWrite(LED3, 0);
        digitalWrite(LED1, 1);
      }
    }
  }
  

}
