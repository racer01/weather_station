#include <Arduino.h>
// weather_station v0.0.1
// barometer+tempmeter+lcd=fun

#include <Sodaq_BMP085.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <math.h>

#define LCDWIDTH  16
#define LCDHEIGHT  2

#define ALTITUDE 210 // Altitude via GPS - Miskolc, Diósgyőr

Sodaq_BMP085 sensor;
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

void paddedWrite(double in, int decimals, int row, String postfix)
{
  byte side = LCDWIDTH - postfix.length();
  // decimal mark
  byte dm = 0;  if(decimals > 0) dm = 1;

  lcd.setCursor(paddingReq(in, side) - dm - decimals, row);
  lcd.print((round(in * pow(10, decimals)) / pow(10, decimals)), decimals);
  lcd.setCursor(side, row);
  lcd.print(postfix);
}

void meterWrite(float temp, double pressure, double decimals)
{
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
  lcd.clear();
  lcd.print("BOOT");
  sensor.begin();
}

void loop() // repeat 5sec (sum of 1+1+1+1+1secs)
{
	double T, p0;
	T = sensor.readTemperature();
	p0 = sensor.readPressure(ALTITUDE);
	meterWrite(T, p0 / 100, 1);
	delay(100);
}
