
#include "..\include\ILI9341Display.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

ILI9341Display display;



void setup(){

    bool status;

    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    status = bme.begin(0x76);  
    if (!status) {
       Serial.println("Could not find a valid BME280 sensor, check wiring!");
       while (1);
    }
    Serial.begin(115200);
    display.init();
    //show_maindisplay();
    //display.wifiScreen();
    //show_wifidisplay();
    // lv_example_style_3();
    // lv_scr_load(scr1);
    display.lv_example_style_3_class();
    
    

}

void loop(){
    display.updateData(bme.readTemperature(), bme.readHumidity(), bme.readPressure(), bme.readAltitude(SEALEVELPRESSURE_HPA));
    lv_timer_handler();
    delay( 5 );

}