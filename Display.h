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

#ifndef DISPLAY_H
#define DISPLAY_H

#include <LiquidCrystal.h>

#include "pins.h"

class Display
{
  public:
    Display();

    void setup();

    void printStartupMessage();

    void clear(int x, int y, int length);
    void clear();

    /* Menu functions. */
    void clearMenuMash(int x, int y);
    void clearMenuMash();
    void clearMenuSparge(int x, int y);
    void clearMenuSparge();
    void clearMenuBoil(int x, int y);
    void clearMenuBoil();

    void printMenuMash(int x, int y);
    void printMenuMash();
    void printMenuSparge(int x, int y);
    void printMenuSparge();
    void printMenuBoil(int x, int y);
    void printMenuBoil();

    void printMenu(int pos);

    /* Mash functions. */
    void clearTemp(int x, int y);
    void clearTargetTemp();
    void clearTime(int x, int y);
    void clearTime();
    void clearIndicator(int x, int y);
    void clearIndicator();
    void clearElementStatus(int x, int y);
    void clearElementStatus();

    void printTemp(double temp, int x, int y);
    void printTargetTemp(double temp);
    void printProbeTemp(double temp);
    void printTime(unsigned long time, int x, int y);
    void printTime(unsigned long time);
    void printIndicator(int x, int y);
    void printIndicator(void);
    void printElementStatus(int x, int y);
    void printElementStatus();

    void printFunction(char *name, double targetTemp, double probeTemp,
                       unsigned long time, bool elementStatus);

  private:
    LiquidCrystal _lcd;
};

#endif
