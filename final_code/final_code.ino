/* 
 * SparkFun Artemis Contest Project
 * Developer - Dhairya Parikh
 * Details : This project has been developed under the MIT License 
 * Project Info - Urban Quality Monitoring System
 */

//******** Library Initializations**********

#include <SparkFunBME280.h>
#include <SparkFunCCS811.h>
#include <SFE_MicroOLED.h>
#include "Air_Quality_Sensor.h"
#include <ArduinoJson.h>                        //For Serial Data transmission

//******************************************

//****** Pin and address definations *******

#define PIN_RESET 9
#define DC_JUMPER 1
#define CCS811_ADDR 0x5B

//******************************************

//******** Global Sensor Objects ***********

CCS811 myCCS811(CCS811_ADDR);
BME280 myBME280;
MicroOLED oled(PIN_RESET, DC_JUMPER);
AirQualitySensor sensor(A13);

//JSON Document initialization
const int capacity = JSON_OBJECT_SIZE(10);
StaticJsonDocument<capacity> doc;

//******************************************

//********* Global Variables ***************

float h,t,co2,tvoc,grove;
int quality;
const int  buttonPin = 2;    // the pin that the pushbutton is attached to
const int ledPin = 19;       // the pin that the LED is attached to
const int relayPin = 9;

// Variables will change:
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button
int count = 0;

//******************************************
// Print Driver Error Function for CCS811***

void printDriverError( CCS811Core::status errorCode )
{
  switch ( errorCode )
  {
    case CCS811Core::SENSOR_SUCCESS:
      Serial.print("SUCCESS");
      break;
    case CCS811Core::SENSOR_ID_ERROR:
      Serial.print("ID_ERROR");
      break;
    case CCS811Core::SENSOR_I2C_ERROR:
      Serial.print("I2C_ERROR");
      break;
    case CCS811Core::SENSOR_INTERNAL_ERROR:
      Serial.print("INTERNAL_ERROR");
      break;
    case CCS811Core::SENSOR_GENERIC_ERROR:
      Serial.print("GENERIC_ERROR");
      break;
    default:
      Serial.print("Unspecified error.");
  }
}

//******************************************

//******BME280 Initilzation Function********

void BMEinit()
{
    myBME280.settings.commInterface = I2C_MODE;
    myBME280.settings.I2CAddress = 0x77;
    myBME280.settings.runMode = 3; //Normal mode
    myBME280.settings.tStandby = 0;
    myBME280.settings.filter = 4;
    myBME280.settings.tempOverSample = 5;
    myBME280.settings.pressOverSample = 5;
    myBME280.settings.humidOverSample = 5;
  
    //Calling .begin() causes the settings to be loaded
    delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
    byte id = myBME280.begin(); //Returns ID of 0x60 if successful
    if (id != 0x60)
    {
      Serial.println("Problem with BME280");
    }
    else
    {
      Serial.println("BME280 online");
    }
}

//******************************************************
//*********** CCS811 Initilization *********************

void CCSinit()
{
    CCS811Core::status returnCode = myCCS811.begin();
    if (returnCode != CCS811Core::SENSOR_SUCCESS)
    {
      Serial.println("Problem with CCS811");
      printDriverError(returnCode);
    }
    else
    {
      Serial.println("CCS811 online");
    }
}

//**********************************************
//************* OLED Initialization ************

void oledinit()
{
    delay(100);
    oled.begin();
    oled.clear(ALL);
    oled.display();
    delay(1000);
    oled.clear(PAGE);
    oled.display();
  
    oled.setFontType(0);
    oled.setCursor(0,0);
    oled.print("Oled Online");
    oled.display();
    delay(2000);
    oled.clear(PAGE);
    
    Serial.println(" OLED Online ");
}

void groveairinit()
{
    if (sensor.init()) 
    {
      Serial.println("Grove Air Quality Sensor Online");
    }
}
//*********************************************

float gethumidity()
{

    h = myBME280.readFloatHumidity();    
    return h;
    
}

float gettemperature()
{

    t = myBME280.readTempC();
    return t;
  
}

float gettvoc()
{

    if (myCCS811.dataAvailable())
    {
   
      myCCS811.readAlgorithmResults();
      tvoc = myCCS811.getTVOC();
    }  
    
    return tvoc;
 
}

float getco2()
{

    co2 = myCCS811.getCO2();
    return co2;
  
}

float getgrovevalue()
{

    grove = sensor.getValue();
    return grove;
  
}

int getgrovequality()
{

    quality = sensor.slope();
    return quality;
  
}

//Function to print obtained values on OLED Display *****

void printoled(int choice)
{

    switch(choice)
    {
      case 1:    //Case for print temperature and humidity values on Oled
        delay(200);
        oled.setFontType(0);
        oled.setCursor(0,0);
        oled.clear(PAGE);
        oled.print("Temperature:");
        oled.print(t,1);
        oled.print(" C");
        oled.print("\nHumidity:");
        oled.print(h,0);
        oled.print(" %");
        oled.display();
        break;

      case 2:    //Case for print co2 and tvoc values on Oled
        delay(200);
        oled.setFontType(0);
        oled.setCursor(0,0);
        oled.clear(PAGE);
        oled.print("TVOC:");
        oled.print(tvoc,1);
        oled.print(" ppb");
        oled.print("\nCO2:");
        oled.print(co2,1);
        oled.print(" ppm");
        oled.display();
        break;

      case 3:    //Case for print grove sensor value on Oled
        delay(200);
        oled.setFontType(0);
        oled.setCursor(0,0);
        oled.clear(PAGE);
        oled.print("Grove Air:");
        oled.print(grove,1);
        if (quality == AirQualitySensor::FORCE_SIGNAL) {
    oled.print("\n High pollution! Force signal active.");
  }
  else if (quality == AirQualitySensor::HIGH_POLLUTION) {
    oled.print("\n\n High pollution!");
  }
  else if (quality == AirQualitySensor::LOW_POLLUTION) {
    oled.print("\n\n Low pollution!");
  }
  else if (quality == AirQualitySensor::FRESH_AIR) {
    oled.print("\n\n Fresh air");
  }
        
        oled.display();
        break;

      default:    //Case for print temperature value on Oled
        delay(200);
        oled.setFontType(0);
        oled.setCursor(0,0);  
        oled.clear(PAGE);
        oled.print("Urban Air Quality \nMonitoringSystem");
        oled.display();
    }
    
}

//***********************************************************

//******** Function to send sensor values to photon serially ******

void sendSerial(float tem, float humid, float tvoc, float co2, float ga, int gq)
{

 //----------------Adding The Data in JSON Document-------------------------------------
    doc["t"] = tem;
    doc["h"] = humid;
    doc["tv"] = tvoc;
    doc["c"] = co2;
    doc["ga"] = ga;
    doc["gq"] = gq;
 //-------------------------------------------------------------------------------------

    JsonObject root = doc.as<JsonObject>(); // get the root object
    
    serializeJson(doc, Serial1);
    Serial1.print("#");
}

void printSerial(float tem, float humid, float ccstvoc, float ccsco2, float ga, int gq)
{

    Serial.println("----------------------------------------------");
    Serial.print("Temperature : ");
    Serial.print(tem);
    Serial.println(" °C");
    

    Serial.print("Humidity : ");
    Serial.print(humid);
    Serial.println(" %");
   

    Serial.print("CCS TVOC : ");
    Serial.print(tvoc);
    Serial.println(" ppb");
   

    Serial.print("CCS eCO2 : ");
    Serial.print(co2);
    Serial.println(" ppm");
    delay(500);

    Serial.print("Grove Air Value : ");
    Serial.println(ga);

    
    Serial.print("Grove Quality Value : ");
    Serial.println(gq);

    if (gq == AirQualitySensor::FORCE_SIGNAL) {
    Serial.println("High pollution! Force signal active.");
  }
  else if (gq == AirQualitySensor::HIGH_POLLUTION) {
    Serial.println("High pollution!");
  }
  else if (gq == AirQualitySensor::LOW_POLLUTION) {
    Serial.println("Low pollution!");
  }
  else if (gq == AirQualitySensor::FRESH_AIR) {
    Serial.println("Fresh air.");
  }
    Serial.println("----------------------------------------------");
    
    delay(1000);
}

//Function for logic

void Logic(float co2)
{
  if(co2 > 1000.00)
  {
    for(int fadeValue = 0; fadeValue <= 255; fadeValue +=5) {
        // sets the value (range from 0 to 255):
        analogWrite(ledPin, fadeValue);
        // wait for 30 milliseconds to see the dimming effect
        delay(30);
    }

    // fade out from max to min in increments of 5 points:
    for(int fadeValue = 255; fadeValue >= 0; fadeValue -=5) {
        // sets the value (range from 0 to 255):
        analogWrite(ledPin, fadeValue);
        // wait for 30 milliseconds to see the dimming effect
        delay(30);
    }
    digitalWrite(relayPin, LOW);
  }
  else
  {
    analogWrite(ledPin, 255);
    digitalWrite(relayPin, HIGH);
  }
}

// Push Button Counter Function **********************

void setup() 
{

    // initialize the button pin as a input:
    pinMode(buttonPin, INPUT);
    // initialize the LED and relay as an output:
    pinMode(ledPin, OUTPUT);
    pinMode(relayPin, OUTPUT);
    
    Serial.begin(115200);
    Serial1.begin(9600);
    Wire.begin();    //initialize I2C bus
    Serial.print(" Initializaing BME280 Sensor : ");
    BMEinit();
    Serial.print(" Initializing CCS811 Sensor : ");
    CCSinit();
    Serial.print(" Initializing OLed Module : ");
    oledinit();
    Serial.print(" Initializing Grove Air Quality Module : ");
    groveairinit();
    Serial.println();
    
}

void loop() 
{
    //get the sensor values
    float temperature = gettemperature();
    float humidity = gethumidity();
    float CCStvoc = gettvoc();
    float CCSco2 = getco2();
    float grove_air_value = getgrovevalue();
    int grove_air_quality = getgrovequality(); 
    

    // Print this data on Serial
    //printSerial(temperature, humidity, CCStvoc, CCSco2, grove_air_value, grove_air_quality);

   
    
    //Send this data serially to photon
    sendSerial(temperature, humidity, CCStvoc, CCSco2, grove_air_value, grove_air_quality);

    Logic(CCSco2);    
    
    buttonState = digitalRead(buttonPin);
    // compare the buttonState to its previous state
    if (buttonState != lastButtonState) 
    {
      // if the state has changed, increment the counter
      if (buttonState == HIGH) 
      {
        // if the current state is HIGH then the button went from off to on:
        buttonPushCounter++;
        Serial.println(buttonPushCounter);
        
      }

      // Delay a little bit to avoid bouncing
      delay(50);
    }
    // save the current state as the last state, for next time through the loop
    lastButtonState = buttonState; 

    if ((buttonPushCounter == 1) || (buttonPushCounter == 2) || (buttonPushCounter == 3))
    {
       printoled(buttonPushCounter);
    }
    else
    {
       buttonPushCounter = 0;    //reset count;
       printoled(buttonPushCounter);
    }
    
    
}
