#include <WiFi.h>
#include <IOXhop_FirebaseESP32.h>

// Set these to run example.
#define FIREBASE_HOST "Fire Base Host"
#define FIREBASE_AUTH "Data Base key"
#define WIFI_SSID "Wifi SSID"
#define WIFI_PASSWORD "Wifi password"

#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
MAX30105 particleSensor;


const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;


void setup() 
{
 Serial.begin(115200);
 Serial.println("Initializing...");
 // Initialize sensor
 if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
 {
   Serial.println("MAX30105 was not found. Please check wiring/power. ");
   while (1);
 }
 Serial.println("Place your index finger on the sensor with steady pressure.");
 particleSensor.setup(); //Configure sensor with default settings
 particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
 particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
  particleSensor.enableDIETEMPRDY(); //Enable the temp ready interrupt. This is required.
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop(){

float temperature = particleSensor.readTemperature();

 float temperatureF = particleSensor.readTemperatureF();

 
 long irValue = particleSensor.getIR();
 if (checkForBeat(irValue) == true) // checkForBeat is a function of heartRate.h Library
 {
   //We sensed a beat!
   long delta = millis() - lastBeat;
   lastBeat = millis();
   beatsPerMinute = 60 / (delta / 1000.0);
   if (beatsPerMinute < 255 && beatsPerMinute > 20)
   {
     rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
     rateSpot %= RATE_SIZE; //Wrap variable
     //Take average of readings
     beatAvg = 0;
     for (byte x = 0 ; x < RATE_SIZE ; x++)
       beatAvg += rates[x];
     beatAvg /= RATE_SIZE;
   }
 }

 if (irValue < 50000){
   Serial.print(" No finger?");
   Serial.println();
    Firebase.setFloat("Temperature", 0);
    
 Firebase.setInt("BPM", 0);
 Firebase.setInt("Heart_Rate",0);
   }
 else{
    Serial.print("IR=");
 Serial.print(irValue);
 Serial.print(", BPM=");
 Serial.print(beatsPerMinute);
 
 Serial.print(", Avg BPM=");
 Serial.print(beatAvg);
 Serial.print(" temperatureF=");
 Serial.print(temperatureF, 4);
 Serial.print(" temperatureC=");
 Serial.print(temperature);
 Serial.println();

 Firebase.setInt("BPM", beatsPerMinute);
 Firebase.setInt("Heart_Rate",beatAvg);
 Firebase.setFloat("Temperature", temperature);
 }
}
