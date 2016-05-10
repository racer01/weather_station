// weather_station v0.0.1
// barometer+tempmeter+lcd=fun

#include <SFE_BMP180.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <math.h>

#define LCDWIDTH  16
#define LCDHEIGHT  2

#define ALTITUDE 210.0 //Altitude via GPS - Miskolc, Diósgyőr

SFE_BMP180 sensor;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
const byte halfScreen = LCDWIDTH / 2;

int paddingReq(double in, int side)
{
  if(in > 999 || in < -99)
    return side - 4;
  else if(in > 99 || in < -9)
    return side - 3;
  else if(in > 9 || in < 0)
    return side - 2;
  else // 0 < in < 10
    return side - 1;
}

int paddingReq2(double in)
{
  if(in > 999 || in < -99)
    return halfScreen - 4;
  else if(in > 99 || in < -9)
    return halfScreen - 3;
  else if(in > 9 || in < 0)
    return halfScreen - 2;
  else // 0 < in < 10
    return halfScreen - 1;
}

void paddedWrite(double in, int decimals, int row, String postfix)
{
  byte side = LCDWIDTH - postfix.length();
  // decimal mark
  byte dm = 0;  if(decimals > 0) dm = 1;
  
  lcd.setCursor(paddingReq(in, side) - dm - decimals, row);    
  lcd.print((round(in * pow(10, decimals)) / pow(10, decimals)), decimals);
  lcd.setCursor(side, row);
  //lcd.setCursor(paddingReq(in) - decimals, row);
  lcd.print(postfix);
}

void paddedWrite2(double in, int decimals, int row, String postfix)
{
  if(decimals > 0)
  {
    lcd.setCursor(halfScreen + paddingReq2(in) - 1 - decimals - postfix.length(), row);
    lcd.print((round(in * pow(10, decimals)) / pow(10, decimals)) + postfix);
  }
  else
  {
    lcd.setCursor(halfScreen + paddingReq2(in) - postfix.length(), row);
    lcd.print(round(in) + postfix);
  }
}

void meterWrite(double temp, double pressure, double decimals)
{
  //lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  paddedWrite(temp, decimals, 0, (String)((char)223) + "C ");

  lcd.setCursor(0, 1);
  lcd.print("Atmo:");
  paddedWrite(pressure, decimals, 1, "hPa");
}


void setup()
{
  lcd.begin(LCDWIDTH, LCDHEIGHT);
  //Serial.begin(9600);
  //Serial.println("BOOT");
  lcd.clear();
  lcd.print("BOOT");
  if (!sensor.begin())
  {
    //Serial.println("BMP180 init fail\n\n");
    lcd.print("BMP180 init fail");
    while(1); // Pause forever.
  }
}

void loop() // repeat 5sec (sum of 1+1+1+1+1secs)
{
  /*meterWrite(-0.5, 1600.45, 0);
  delay(1000);
  meterWrite(200, 900.654, 0);
  delay(1000);*/

  char status;
  double T, P, p0, a;
  status = sensor.startTemperature();
  if(status != 0)
  {
    delay(status);
    status = sensor.getTemperature(T);
    if(status != 0)
    {
      status = sensor.startPressure(3);
      if(status != 0)
      {
        delay(status);
        status = sensor.getPressure(P, T);
        if(status != 0)
        {
          p0 = sensor.sealevel(P, ALTITUDE);
          meterWrite(T, p0, 2);
        }
      }
    }
  }
  delay(100);
}
