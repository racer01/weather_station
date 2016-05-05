// displaytest

#define LCDWIDTH  16
#define LCDHEIGHT  2

#include <LiquidCrystal.h>
#include <math.h>


LiquidCrystal lcd(8, 9, 4, 5, 6, 7);


const byte halfScreen = LCDWIDTH / 2;
void setup()
{
  lcd.begin(LCDWIDTH, LCDHEIGHT);
}

int paddingReq(double in)
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
  if(decimals > 0)
  {
    lcd.setCursor(halfScreen + paddingReq(in) - 1 - decimals - postfix.length(), row);
    lcd.print(in + postfix);
  }
  else
  {
    lcd.setCursor(halfScreen + paddingReq(in) - postfix.length(), row);
    lcd.print(round(in) + postfix);
  }
}

void meterWrite(double temp, double pressure, double decimals)
{
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  paddedWrite(temp, decimals, 0, (String)((char)223)+   "C ");

  lcd.setCursor(0, 1);
  lcd.print("Atm:");
  paddedWrite(pressure, decimals, 1, "hPa");
}


void loop()
{
  meterWrite(-0.5, 1600.45, 0);
  delay(1000);
  meterWrite(200, 900.654, 0);
  delay(1000);
}
