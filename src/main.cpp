#include <Arduino.h>
/*
	Name: weather_station

	@author Székely-Tóth László
	@version 1.1.0 22-05-16
*/

#include <RBD_Timer.h>
#include <Sodaq_BMP085.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <math.h>

// MODEs: 0 - running avg, 1 - last CACHESIZE avg, 2 - raw data
#define DEFMODE 1
#define ALTITUDE 210 // Altitude via GPS - Miskolc, Diósgyőr

// device constants
#define LCDWIDTH  16
#define LCDHEIGHT  2
#define CACHESIZE 120 // 6 seconds of data @ 125
#define DLY 125
#define JSON 4
#define ROWCOUNT 2

// program constants
const String prefix[ROWCOUNT] = {"Temp:", "Atmo:"};
const String postfix[ROWCOUNT] = {(String)((char)223) + "C ", "hPa"};
const byte sides[ROWCOUNT][2] = {{5, 13}, {5, 13}}; // [ROW][COLUMN] = {{r0, c0}, {r1, c1}}

// declarations
RBD::Timer timer;
RBD::Timer timerJSON;
Sodaq_BMP085 sensor;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
byte mode = DEFMODE;

// averages
float temp_avg;
unsigned long atm_avg;

// data arrays
float temp_data[CACHESIZE];
unsigned long atm_data[CACHESIZE];
byte data_pointer = 0;

/*
	Reads data from sensor, always takes the specified time.

	@param temp pointer of the place to store temperature.
	@param atm pointer of the place to store atmospheric pressue.
*/
void SensorRead(float* const temp, unsigned long* const atm) // this must be 80 milliseconds
{
	unsigned long previousMillis = millis();
	*temp = sensor.readTemperature();
	*atm = sensor.readPressure(ALTITUDE);
	unsigned long currentMillis = millis();
	while (currentMillis - previousMillis < DLY - 45)
	{
		delay(1);
		currentMillis = millis();
	}
}

/// <summary>
/// MATH
///</summary>

/*
	Calculates average from any type.

	@param in array to calculate average from.
	@param length length of given array.
	@return the average of given data.
*/
template <typename Type>
Type Average(Type* in, byte length)
{
	Type t = 0;
	for (byte i = 0; i < length; i++)
	{
		t += in[i];
	}
	t /= length;
	return t;
}

/// <summary>
/// LCD WRITE
/// </summary>

/*
	Calculates padding.

	@param in number to calculate padding for.
	@param row lcd screen tow where it will be written to mind constrains.
	@return padding count.
*/
byte Padding(double in, byte row)
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

/*
	Loading/Boot screen.
*/
void BootWrite()
{
	lcd.print("weather_station");
	lcd.setCursor(0, 1);
	lcd.print("v1.1.0    by rcr");
}

/*
	Writing non-variable characters.
*/
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

/*
	Writing numbers with padding.

	@param in number to be written.
	@param decimals decimal places.
	@param row to write the number.
*/
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
	lcd.setCursor(Padding(in, row) - dm - decimals, row);
	lcd.print((round(in * pow(10, decimals)) / pow(10, decimals)), decimals);
}

/*
	Writing data.

	@param temp temperature to write.
	@param pressure atmospheric pressure to write.
	@param decimals decimal places.
*/
void MeterWrite(float temp, double pressure, double decimals)
{
	PaddedWrite(temp, decimals, 0);
	PaddedWrite(pressure, decimals, 1);
}

/// <summary>
/// STEP
/// </summary>

/*
	Initializing simple mode.
*/
void SimpleInit()
{
	SensorRead(&temp_avg, &atm_avg);
}

/*
	Updating data
*/
void SimpleStep()
{
	float temp = 0;
	unsigned long atm = 0;
	SensorRead(&temp, &atm);
	temp_avg = (temp_avg + temp) / 2;
	atm_avg = (atm_avg + atm) / 2;
}

/*
	Initializing advanced mode.
*/
void AdvancedInit()
{
	for (data_pointer = 0; data_pointer < CACHESIZE; data_pointer++)
		SensorRead(&temp_data[data_pointer], &atm_data[data_pointer]);
	data_pointer = 0;
}

/*
	Updating data
*/
void AdvancedStep()
{
	SensorRead(&temp_data[data_pointer], &atm_data[data_pointer]);

	if (data_pointer < CACHESIZE)
		data_pointer++;
	else
		data_pointer = 0;
}

/*
	Initializing raw data mode.
*/
void RawInit()
{
	SensorRead(&temp_avg, &atm_avg);
}

/*
	Updating data
*/
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


// MAIN
void switcher()
{
	switch (mode)
	{
		case 0: SimpleStep();
			break;
		case 1: AdvancedStep();
				temp_avg = Average(temp_data, CACHESIZE);
				atm_avg = Average(atm_data, CACHESIZE);
			break;
		case 2: RawStep();
			break;
	}
}

void setup()
{
	lcd.begin(LCDWIDTH, LCDHEIGHT);
	lcd.clear();
	BootWrite();
	Serial.begin(9600);
	while (!Serial);
	sensor.begin();
	initialize(mode);
	FirstWrite();
	timer.setTimeout(DLY);
	timerJSON.setTimeout(JSON * DLY);
	timer.restart();
	timerJSON.restart();
}

void loop()
{
	if (timer.onRestart())
	{
		switcher();
		MeterWrite(temp_avg, atm_avg / 100.0, 1);
	}
	if (timerJSON.onRestart())
	{
		// Sending data in JSON via serial port
		Serial.print("{\"temp\":");
		Serial.print(temp_avg);
		Serial.print(", \"atm\":");
		Serial.print(atm_avg);
		Serial.println("}");
	}
}
