#include <Wire.h>
#include "RTClib.h"
#include <LiquidTWI.h> 
#include <Adafruit_ADS1015.h>
#include <math.h>
#include <SD.h>
#include <SPI.h>

Adafruit_ADS1115 ads;
RTC_DS1307 rtc;
LiquidTWI lcd(0);
const int chipSelect = 53;

unsigned long _changeState = 5250;
unsigned long _refreshState = 250;
unsigned long previusMillisChS = 0;
unsigned long previusMillisRS = 0;
unsigned long _Sampling = 30000;
unsigned long previusMillisSpg = 0;
unsigned long _Datalogger = 60000;
unsigned long previusMillisDlG = 0;

int16_t adc0;
int16_t adc1;
int cont = 0;

float DC;
float Level;
float Velocity;
float sav;
float sav0;
float AT; // Area total del trapecio 

int index = 0;
const int N = 20;
float Buffer[N];

int index0 = 0;
const int N0 = 20;
float Buffer0[N0];


#define STATESTART 1
#define STATEDATE 2
#define STATESENSORLEVEL 3
#define STATESENSORVELOCITY 4
#define STATEAREA 5
#define STATEDISCHARGE 6

int stateDisplay = STATESTART;

void setup()
{
	ads = 0x48;
	lcd.begin(20, 4);
	ads.begin();
	rtc.begin();
	SD.begin();

	ads.setGain(GAIN_TWO);
}

void loop()
{
	unsigned long currentMillis = millis();

	if ((currentMillis - previusMillisRS) >= _refreshState)
	{
		previusMillisRS = currentMillis;
		refreshState();
	}

	if ((currentMillis - previusMillisChS) >= _changeState)
	{
		previusMillisChS = currentMillis;
		changeState();
	}
	if ((currentMillis - previusMillisDlG) >= _Datalogger)
	{
		previusMillisDlG = currentMillis;
		SAVE();
	}
	if (stateDisplay == STATESTART || (currentMillis - previusMillisSpg) >= _Sampling)
	{
		previusMillisSpg = currentMillis;

	}
}

void refreshState()
{
	switch (stateDisplay)
	{
	case STATESTART:
		lcd.clear();
		START();
		break;

	case STATEDATE:
		lcd.clear();
		DATE();
		conta();
		Sampling();
		break;

	case STATESENSORLEVEL:
		lcd.clear();
		SENSORLEVEL();
		break;

	case STATESENSORVELOCITY:
		lcd.clear();
		SENSORVELOCITY();
		break;

	case STATEAREA:
		lcd.clear();
		AREA();
		break;

	case STATEDISCHARGE:
		lcd.clear();
		DISCHARGE();
		break;
	}
}

void changeState()
{
	switch (stateDisplay)
	{
	case STATESTART:
		stateDisplay = STATEDATE;
		break;

	case STATEDATE:
		stateDisplay = STATESENSORLEVEL;
		break;

	case STATESENSORLEVEL:
		stateDisplay = STATESENSORVELOCITY;
		break;

	case STATESENSORVELOCITY:
		stateDisplay = STATEAREA;
		break;

	case STATEAREA:
		stateDisplay = STATEDISCHARGE;
		break;

	case STATEDISCHARGE:
		stateDisplay = STATEDATE;
		break;
	}
}

void START()
{
	lcd.setCursor(2, 0);
	lcd.print("DRRY TECNOLOGIA");
	lcd.setCursor(6, 1);
	lcd.print("VERSION");
	lcd.setCursor(8, 2);
	lcd.print("1.2");
}

void DATE()
{
	DateTime now = rtc.now();

	int thour;
	thour = now.hour();

	{if (thour >= 12) {
		lcd.setCursor(13, 0);
		lcd.print("PM");
		lcd.setCursor(4, 0);
		thour = thour - 12;
		if (thour == 0) {
			thour = 12; // Day 12 PM
		}
		if (thour <= 9) {
			lcd.setCursor(4, 0);
			lcd.print("0");
			lcd.setCursor(5, 0);
			lcd.print(now.hour(), DEC);
		}
		else {
			lcd.print(now.hour(), DEC);
		}
	}
	else {
		if (thour == 0) {
			thour = 12; // Night 12 AM
		}
		lcd.setCursor(13, 0);
		lcd.print("AM");
		lcd.setCursor(4, 0);
		if (thour <= 9) {
			lcd.setCursor(4, 0);
			lcd.print("0");
			lcd.setCursor(5, 0);
			lcd.print(now.hour(), DEC);
		}
		else {
			lcd.print(now.hour(), DEC);
		}
	}}
	lcd.setCursor(6, 0);
	lcd.print(":");
	lcd.setCursor(7, 0);
	if (now.minute() <= 9) {
		lcd.setCursor(7, 0);
		lcd.print("0");
		lcd.setCursor(8, 0);
		lcd.print(now.minute(), DEC);
	}
	else {
		lcd.print(now.minute(), DEC);
	}
	lcd.setCursor(9, 0);
	lcd.print(":");
	lcd.setCursor(10, 0);
	if (now.second() <= 9) {
		lcd.setCursor(10, 0);
		lcd.print("0");
		lcd.setCursor(11, 0);
		lcd.print(now.second(), DEC);
	}
	else {
		lcd.print(now.second(), DEC);
	}


	lcd.setCursor(5, 1);
	lcd.print(now.year(), DEC);
	lcd.print('/');
	lcd.setCursor(10, 1);
	if (now.month() <= 9) {
		lcd.print("0");
		lcd.setCursor(11, 1);
		lcd.print(now.month(), DEC);
	}
	else {
		lcd.print(now.month(), DEC);
	}
	lcd.setCursor(12, 1);
	lcd.print('/');
	lcd.setCursor(13, 1);
	if (now.day() <= 9) {
		lcd.print("0");
		lcd.setCursor(14, 1);
		lcd.print(now.day(), DEC);
	}
	else {
		lcd.print(now.day(), DEC);
	}


	lcd.setCursor(6, 2);
	lcd.print("MUESTRAS");
	lcd.setCursor(9, 3);
	lcd.print(cont);


}

void SENSORLEVEL()
{
	adc0 = ads.readADC_SingleEnded(0);
	const float LMP = 0.0625F;
	lcd.setCursor(2, 1);
	lcd.print("NIVEL DEL CANAL");
	lcd.setCursor(6, 2);
	lcd.print(Level);
	lcd.print(" CM");
	lcd.setCursor(6, 3);
	lcd.print(adc0 * LMP);
}

void SENSORVELOCITY()
{
	adc1 = ads.readADC_SingleEnded(1); 
	const float VMP = 0.0625F;
	lcd.setCursor(4, 0);
	lcd.print("VELOCIDAD DEL");
	lcd.setCursor(8, 1);
	lcd.print("AGUA");
	lcd.setCursor(6, 2);
	lcd.print(Velocity);
	lcd.print(" M/S");
	lcd.setCursor(6, 3);
	lcd.print(adc1 * VMP);
}

void DISCHARGE()
{
	DC = (AT) * (Velocity);
	lcd.setCursor(4, 1);
	lcd.print("GASTO TOTAL");
	lcd.setCursor(4, 2);
	lcd.print(DC);
	lcd.print(" M^3/S");
}

void conta()
{
	unsigned long _S = 100;
	unsigned long previusMillisS = 0;
	unsigned long currentMillis = millis();

	if ((currentMillis - previusMillisS) >= _S)
	{
		previusMillisS = currentMillis;

		if (cont < 20)
		{
			cont++;
		}
		else if (cont > 19)
		{
			cont = 0;
		}
	}
}

void Sampling()
{
	adc0 = ads.readADC_SingleEnded(0);
	adc1 = ads.readADC_SingleEnded(1);
	const float LMP = 0.0625F;
	const float VMP = 0.0625F;

	Buffer[index] = (adc0 * LMP);
	index = ++index % N;

	Buffer0[index0] = (adc1 * VMP);
	index0 = ++index0 % N0;


	unsigned long currentMillis = millis();
	unsigned long _Sampling0 = 250;
	unsigned long previusMillisSpg0 = 0;

	if ((currentMillis - previusMillisSpg0) >= _Sampling0)
	{
		previusMillisSpg0 = currentMillis;

		float Tmedia = 0;
		for (int i = 0; i < N; i++)
		{
			Tmedia = Tmedia + Buffer[i];
			sav = Tmedia;
		}

		float Tmedia0 = 0;
		for (int i = 0; i < N0; i++)
		{
			Tmedia0 = Tmedia0 + Buffer0[i];
			sav0 = Tmedia0;
		}
	}

	float SSL;
	float SL;
	SSL = (sav / N);
	SL = (SSL / 216.7);
	Level = (((SL - 4) / 16) * 365.76);

	float SSV;
	float SV;
	SSV = (sav0 / N0);
	SV = (SSV / 216.3);
	Velocity = (((SV - 4) / 16) * 3.0);

}

void AREA()
{
	float bx = 2.30; //Plantilla del canal
	float A; //Talud o Hipotenusa
	float B; //Base del triangulo necesario para sacar el area ((B*C)/2)
	float C; //Nivel
	float T = 1.803; //Variable para el calculo del Talud o Hipotenusa dada por la relacion de 1 a 1.5 

	C = (Level / 100);
	A = C * T;

	B = sqrt(((A * A) - (C * C)));
	AT = (bx + B) * (C);

	lcd.setCursor(3, 1);
	lcd.print("AREA HIDRAULICA");
	lcd.setCursor(6, 2);
	lcd.print(AT);
	lcd.print(" M^2");
}

void SAVE()
{
	DateTime now = rtc.now();
	String DataChar = "";
	String DataData = "";

	File dataFile = SD.open("datalog.txt", FILE_WRITE);

	DataChar += String("Gasto ") + String(DC) + String(" M^3/S") + String(". ") + String("Velocidad ")
		+ String(Velocity) + String(" M/S") + String(". ") + String("Nivel ") + String(Level) + String(" CM")
		+ String(". ") + String("Area ") + String(AT) + String(" M^2") + String(". ");

	DataData += String(now.hour()) + String(":") + String(now.minute()) + String(":") + String(now.second())
		+ String(" ") + String(now.day()) + String("/") + String(now.month()) + String("/") + String(now.year());

	if (dataFile) {
		dataFile.print(DataData);
		dataFile.print(", ");
		dataFile.print(DataChar);
		dataFile.println("DRRY");
		dataFile.close();
	}
}


