#include <Arduino.h>
// weather_station v1.0.1
// barometer+tempmeter+lcd=fun

#include <Sodaq_BMP085.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <math.h>

// MODEs: 0 - running avg, 1 - last CACHESIZE avg, 2 - raw data
#define DEFMODE 1

#define LCDWIDTH  16
#define LCDHEIGHT  2
#define CACHESIZE 50
#define DLY 100
#define ROWCOUNT 2

#define ALTITUDE 210 // Altitude via GPS - Miskolc, Diósgyőr

const String prefix[ROWCOUNT] = {"Temp:", "Atmo:"};
const String postfix[ROWCOUNT] = {(String)((char)223) + "C ", "hPa"};

const byte sides[ROWCOUNT][2] = {{5, 13}, {5, 13}}; // [ROW][COLUMN] = {{r0, c0}, {r1, c1}}


Sodaq_BMP085 sensor;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
byte mode = DEFMODE;


float temp_avg;
unsigned long atm_avg;

float temp_data[CACHESIZE];
unsigned long atm_data[CACHESIZE];
byte data_pointer = 0;


void Average(float *temp, unsigned long *atm)
{
	*temp = 0;
	*atm = 0;
	for (byte i = 0; i < CACHESIZE; i++)
	{
		*temp += temp_data[i];
		*atm += atm_data[i];
	}
	*temp /= CACHESIZE;
	*atm /= CACHESIZE;
}

void SensorRead(float *temp, unsigned long *atm) // this must be 100 millisecond
{
	unsigned long previousMillis = millis();
	*temp = sensor.readTemperature();
	*atm = sensor.readPressure(ALTITUDE);
	unsigned long currentMillis = millis();
	while (currentMillis - previousMillis <= DLY)
	{
		delay(1);
		currentMillis = millis();
	}
}


void FirstWrite()
{
	lcd.clear();
	for (byte i = 0; i < ROWCOUNT; i++)
	{
		lcd.setCursor(0, i);
		lcd.print(prefix[i]);
		lcd.setCursor(LCDWIDTH - postfix[i].length(), i);
		lcd.print(postfix[i]);
	}
}

int PaddingReq(double in, byte row)
{
	if(in > 999 || in < -99)
		return sides[row][1] - 4;
	else if(in > 99 || in < -9)
		return sides[row][1] - 3;
	else if(in > 9 || in < 0)
		return sides[row][1] - 2;
	else // 0 < in < 10
		return sides[row][1] - 1;
}

void PaddedWrite(double in, byte decimals, byte row)
{
	// decimal mark
	byte dm = 0;  if(decimals > 0) dm = 1;

	// clear row
	for (byte i = 0; i < (sides[row][1] - sides[row][0]); i++)
	{
		lcd.setCursor(sides[row][0] + i, row);
		lcd.write(' ');
	}
	lcd.setCursor(PaddingReq(in, row) - dm - decimals, row);
	lcd.print((round(in * pow(10, decimals)) / pow(10, decimals)), decimals);
}

void MeterWrite(float temp, double pressure, double decimals)
{
	PaddedWrite(temp, decimals, 0);
	PaddedWrite(pressure, decimals, 1);
}

void SimpleInit()
{
	SensorRead(&temp_avg, &atm_avg);
}

void SimpleStep()
{
	float temp = 0;
	unsigned long atm = 0;
	SensorRead(&temp, &atm);
	temp_avg = (temp_avg + temp) / 2;
	atm_avg = (atm_avg + atm) / 2;
}

void AdvancedInit()
{
	for (data_pointer = 0; data_pointer < CACHESIZE; data_pointer++)
		SensorRead(&temp_data[data_pointer], &atm_data[data_pointer]);
	data_pointer = 0;
}

void AdvancedStep()
{
	SensorRead(&temp_data[data_pointer], &atm_data[data_pointer]);

	if (data_pointer < CACHESIZE)
		data_pointer++;
	else
		data_pointer = 0;
}

void RawInit()
{
	SensorRead(&temp_avg, &atm_avg);
}

void RawStep()
{
	SensorRead(&temp_avg, &atm_avg);
}

void initialize(byte m)
{
	switch (m)
	{
		case 0: SimpleInit();
			break;
		case 1: AdvancedInit();
			break;
		case 2: RawInit();
			break;
	}
}



void setup()
{
	lcd.begin(LCDWIDTH, LCDHEIGHT);
	lcd.clear();
	lcd.print("BOOT");
	sensor.begin();
	initialize(mode);
	FirstWrite();
}

void loop()
{
	switch (mode) {
		case 0: SimpleStep();
			break;
		case 1: AdvancedStep();
				Average(&temp_avg, &atm_avg);
			break;
		case 2: RawStep();
			break;
	}

	MeterWrite(temp_avg, atm_avg / 100.0, 1);
}
