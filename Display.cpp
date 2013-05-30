/******************************************************************************
 * Copyright (c) 2013 Patrick Colp
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <LiquidCrystal.h>
#include "Display.h"

#define TARGET_TEMP_X 9
#define TARGET_TEMP_Y 0
#define PROBE_TEMP_X 9
#define PROBE_TEMP_Y 1
#define TIME_X 0
#define TIME_Y 1
#define ELEMENT_STATUS_X 7
#define ELEMENT_STATUS_Y 1

#define TEMP_SIZE 7
#define TIME_SIZE 4

Display::Display()
: _lcd(LiquidCrystal(PIN_LCD_RS, PIN_LCD_ENABLE, PIN_LCD_D0, PIN_LCD_D1, PIN_LCD_D2, PIN_LCD_D3))
{
};

void Display::setup(void)
{
  /* Start LCD screen. */
  _lcd.begin(16, 2);
}

void Display::printStartupMessage()
{
  _lcd.setCursor(0, 0);
  _lcd.print("BrewBot  v1.0");
}

void Display::clear(int x, int y, int length)
{
  _lcd.setCursor(x, y);
  for (int i = 0; i < length; i++)
  {
      _lcd.print(" ");
  }
}

void Display::clear()
{
  _lcd.clear();
}

/* Menu functions. */
void Display::clearMenuMash(int x, int y)
{
  clear(x, y, 4);
}

void Display::clearMenuSparge(int x, int y)
{
  clear(x, y, 6);
}

void Display::clearMenuBoil(int x, int y)
{
  clear(x, y, 4);
}

void Display::printMenuMash(int x, int y)
{
  _lcd.setCursor(x, y);
  _lcd.print("Mash");
}

void Display::printMenuSparge(int x, int y)
{
  _lcd.setCursor(x, y);
  _lcd.print("Sparge");
}

void Display::printMenuBoil(int x, int y)
{
  _lcd.setCursor(x, y);
  _lcd.print("Boil");
}

void Display::printMenu(int pos)
{
  clear();

  switch (pos)
  {
    case 0:
    {
      printMenuMash(0, 0);
      printMenuSparge(0, 1);

      break;
    }

    case 1:
    {
      printMenuSparge(0, 0);
      printMenuBoil(0, 1);

      break;
    }

    case 2:
    {
      printMenuBoil(0, 0);

      break;
    }
  }
}


/* Mash functions. */
void Display::clearTemp(int x, int y)
{
  clear(x, y, TEMP_SIZE);
}

void Display::clearTargetTemp()
{
  clearTemp(TARGET_TEMP_X, TARGET_TEMP_Y);
}

void Display::clearTime(int x, int y)
{
  clear(x, y, TIME_SIZE);
}

void Display::clearTime()
{
  clearTime(TIME_X, TIME_Y);
}

void Display::clearIndicator(int x, int y)
{
  clear(x, y, 1);
}

void Display::clearIndicator()
{
  clearIndicator(TIME_X + 1, TIME_Y);
}

void Display::clearElementStatus(int x, int y)
{
  clear(x, y, 1);
}

void Display::clearElementStatus()
{
  clearElementStatus(ELEMENT_STATUS_X, ELEMENT_STATUS_Y);
}

void Display::printUIFunction(char *name, double targetTemp, double probeTemp,
                              unsigned long time, bool elementStatus)
{
  clear();

  _lcd.setCursor(0, 0);
  _lcd.print(name);

  printTargetTemp(targetTemp);
  printProbeTemp(probeTemp);
  printTime(time);
  printIndicator();

  if (elementStatus)
  {
    printElementStatus();
  }
  else
  {
    clearElementStatus();
  }
}

/* Display ":" if needed. */
void Display::printIndicator(int x, int y)
{
  _lcd.setCursor(x, y);
  _lcd.print(":");
}

/* Display ":" if needed. */
void Display::printIndicator()
{
  printIndicator(TIME_X + 1, TIME_Y);
}

void Display::printTime(unsigned long time, int x, int y)
{
  unsigned long hours;
  unsigned long mins;

  /* Display the hours left. */
  hours = time / 60;

  _lcd.setCursor(x, y);
  _lcd.print(hours);

  /* Display indicator. */
  printIndicator(x + 1, y);

  /* Display the minutes left. */
  mins = time % 60;

  _lcd.setCursor(x + 2, y);
  if (mins < 10)
  {
    _lcd.print(0);
  }
  _lcd.print(mins);
}

void Display::printTime(unsigned long time)
{
  printTime(time, 0, 1);
}

/* Display a temperature. */
void Display::printTemp(double temp, int x, int y)
{
  /* Clear the old value. */
  clear(x, y, TEMP_SIZE);

  /* Check for a negative temperature. */
  if (temp < 0)
  {
    temp -= (2 * temp);
    _lcd.setCursor(x, y);
    _lcd.print("-");
  }

  /* Right align cursor. */
  if (temp >= 100)
  {
    _lcd.setCursor(x + 1, y);
  }
  else if (temp >= 10)
  {
    _lcd.setCursor(x + 2, y);
  }
  else
  {
    _lcd.setCursor(x + 3, y);
  }

  /* Print the temperature. */
  _lcd.print(temp);
}

/* Display the target temperature. */
void Display::printTargetTemp(double temp)
{
  printTemp(temp, TARGET_TEMP_X, TARGET_TEMP_Y);
}

/* Display a probe temperature. */
void Display::printProbeTemp(double temp)
{
  printTemp(temp, PROBE_TEMP_X, PROBE_TEMP_Y);
}

/* Display the element status. */
void Display::printElementStatus(int x, int y)
{
  _lcd.setCursor(x, y);
  _lcd.print("*");
}

/* Display the element status. */
void Display::printElementStatus()
{
  printElementStatus(ELEMENT_STATUS_X, ELEMENT_STATUS_Y);
}


