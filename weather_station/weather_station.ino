// weather_station v0.0.1
// barometer+tempmeter+lcd=fun

#include <SFE_BMP180.h>
#include <Wire.h>

SFE_BMP180 pressure;

#define ALTITUDE 210.0 //Altitude via GPS - Miskolc, Diósgyőr

void setup()
{
  Serial.begin(9600);
  Serial.println("BOOT");
  if (!pressure.begin())
  {
    Serial.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }
}

void meterWrite(double temp, double pressure)
{
  //
  //lcd.print("
}

void loop() // repeat 5sec (sum of 1+1+1+1+1secs)
{
  
}
