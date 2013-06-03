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

#ifndef UI_H
#define UI_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <DallasTemperature.h>
#include <BooleanDevice.h>
#include <OneWireTemperatureDevice.h>
#include <PidRelayDevice.h>

#include "constants.h"
#include "pins.h"
#include "BrewBot.h"
#include "Buttons.h"
#include "Display.h"

#define UI_NAME_LEN        7
#define UI_NAME_DISP_LEN   9

#define UI_MAX_FUNCS  5
#define UI_MAX_STEPS  3

#define UI_FUNC_MASH    0
#define UI_FUNC_SPARGE  1
#define UI_FUNC_BOIL    2
#define UI_FUNC_DISINF  3
#define UI_FUNC_COOL    4

#define UI_TIME_MAX      599UL // 9h59m
#define UI_TIME_MIN        0UL // 0h00m
#define UI_TIME_DEFAULT    0UL // 0h30m
#define UI_TIME_BOIL      45UL // 0h15m
#define UI_TIME_DISINF    15UL // 0h15m
#define UI_TIME_COOL     599UL // 9h59m

#define UI_TEMP_MAX      120.00F // 120C
#define UI_TEMP_MIN        0.00F // 0C
#define UI_TEMP_DEFAULT   60.00F // 65C

class UI
{
  public:
    enum states
    {
      STATE_MENU,
      STATE_MASH,
      STATE_SPARGE,
      STATE_BOIL,
      STATE_DISINF,
      STATE_COOL,
      STATE_TIME,
      STATE_TEMP,
      STATE_NEXT,
      STATE_PREV,
      STATE_EXEC,
      STATE_DONE,
    };

    UI(BrewBot *brewBot);

    void setup(void);
    void loop(void);

    void display(void);

    void setFunction(unsigned int function);
    void setProbeDev(OneWireTemperatureDevice *devProbe);
    void setNumSteps(unsigned int numSteps);

    void setState(states state);
    states getState(void);

  private:
    static void handleButtons(void *cookie, int id, bool held);

    void displayBlinkMenuItem(void);

    BrewBot *_brewBot;
    Display _display;

    Buttons _buttons;
    unsigned int _function;
    states _state;

//    char _name[UI_MAX_FUNCS][UI_NAME_LEN];
    char *_name[UI_MAX_FUNCS + 1];
    char _nameDisplay[UI_NAME_DISP_LEN];

    OneWireTemperatureDevice *_devProbe;

    unsigned int _numSteps;
    unsigned int _step;

    unsigned long _nextTickBlink;
    unsigned long _nextTickTimer;
    unsigned long _nextTickReminder;
    unsigned long _nextTickBeeper;

    unsigned int _numBeeps;

    unsigned long _time[UI_MAX_FUNCS][UI_MAX_STEPS];
    double _targetTemp[UI_MAX_FUNCS][UI_MAX_STEPS];
    double _probeTemp;

    int _menuPosition;

    void startFunction(void);
    void stopFunction(void);

    bool nextStep(void);
    bool setStep(unsigned int step);
    void setName(unsigned int function);
    void setTime(double time);
    void setTargetTemp(double temp);

    char *getName();
    unsigned long getTime();
    double getTargetTemp();
    double getProbeTemp();

    bool updateProbeTemp();
    bool updateTimer();
    bool updateReminder();
    bool updateBeeper();

    void displayBlink(void (*clear)(void), void (*print)(void));
    void displayBlinkTime();
    void displayBlinkTemp();
    void displayBlinkIndicator();
    void displayProbeTemp();
    void displayTimer();

    void keyPressMenu(unsigned int key, bool held);
    void keyPressTime(unsigned int key, bool held);
    void keyPressTemp(unsigned int key, bool held);
    void keyPressExec(unsigned int key, bool held);
    void keyPressDone(unsigned int key, bool held);
};

#endif

