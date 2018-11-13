/**
   @copyright (C) 2017 Melexis N.V.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

#include <Wire.h>
#include <Arduino.h>
#include "dht12.h"

#define sda_pin (4)
#define scl_pin (5)

uint8_t data[5];
int Temprature, Humi;
void dht12_init(void)
{
    Wire.begin(sda_pin,scl_pin);
    //Wire.setClock(400000); //Increase I2C clock speed to 400kHz
}

//Read a number of words from startAddress. Store into Data array.
//Returns 0 if successful, -1 if error
int dht12_read(void)
{
  uint8_t i;
  uint8_t res;
    Wire.beginTransmission(0x5c);
    Wire.write(0); 
    res = Wire.endTransmission();
    //Serial.printf("res:%d",res);
    
    if (res == 0) 
    {

       Wire.beginTransmission(0x5c);
       
       Wire.requestFrom(0x5c, 5);
      if (Wire.available())
      {
        //Store data into array
        for(i = 0;i<5;i++)
        data[i] = Wire.read(); //LSB
        Wire.endTransmission();
        parse_dht12_data();
      }
      else
      {
         Serial.println("No ack read");
      }
    }
    else
    {
      Serial.println("sensor read");
    }
   return (0); //Success

}
void parse_dht12_data(void)
{
  uint16_t i;
  uint8_t Humi_H,Humi_L,Temp_H,Temp_L,Temp_CAL,temp;
  Humi_H = data[0];
  Humi_L = data[1];
  Temp_H = data[2];
  Temp_L = data[3];
  Temp_CAL = data[4];
  temp = (uint8_t)(Humi_H+Humi_L+Temp_H+Temp_L);
  if(Temp_CAL == temp)
  {
    Humi=Humi_H*10+Humi_L; 

    if(Temp_L&0X80) 
    {
      Temprature =0-(Temp_H*10+((Temp_L&0x7F)));
    }
    else  
    {
      Temprature=Temp_H*10+Temp_L;
    }
  
    if(Humi>950)
    {
      Humi=950;
    }
    if(Humi<200)
    {
      Humi =200;
    }
    if(Temprature>600)
    {
      Temprature=600;
    }
    if(Temprature<-200)
    {
      Temprature = -200;
    }
    Temprature=Temprature/10;
    Humi=Humi/10;
    Serial.printf("\r\nTemperature:%d\r\n",Temprature);
    Serial.printf("Humi:%d\r\n",Humi);
    
  }
  else
  {
    Serial.println("CRC error");
  }
}
void gpio_test(void)
{
  digitalWrite(4, LOW);   // Turn the LED on (Note that LOW is the voltage level
  digitalWrite(5, LOW);   // Turn the LED on (Note that LOW is the voltage level
  digitalWrite(0, LOW);   // Turn the LED on (Note that LOW is the voltage level
  digitalWrite(2, LOW);   // Turn the LED on (Note that LOW is the voltage level
  // but actually the LED is on; this is because
  // it is active low on the ESP-01)
  delay(100);                      // Wait for a second
  digitalWrite(4, HIGH);  // Turn the LED off by making the voltage HIGH
  digitalWrite(5, HIGH);  // Turn the LED off by making the voltage HIGH
  digitalWrite(0, HIGH);  // Turn the LED off by making the voltage HIGH
  digitalWrite(2, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(100);
}
