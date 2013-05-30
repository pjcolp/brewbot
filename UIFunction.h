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

#ifndef UIFUNCTION_H
#define UIFUNCTION_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <DallasTemperature.h>
#include <BooleanDevice.h>
#include <OneWireTemperatureDevice.h>
#include <PidRelayDevice.h>

#include "pins.h"
#include "Buttons.h"
#include "Display.h"

#define UIFUNCTION_NAME_LEN        6
#define UIFUNCTION_NAME_DISP_LEN   9

#define UIFUNCTION_MAX_STEPS  3

#define UIFUNCTION_TIME_MAX  599UL // 9h59m
#define UIFUNCTION_TIME_MIN  0UL // 0h00m
#define UIFUNCTION_TIME_DEFAULT  0UL // 0h30m

#define UIFUNCTION_TEMP_MAX  120.00F // 120C
#define UIFUNCTION_TEMP_MIN  0.00F // 0C
#define UIFUNCTION_TEMP_DEFAULT  25.00F // 65C

class UIFunction
{
  public:
    enum states {
      STATE_MENU,
      STATE_TIME,
      STATE_TEMP,
      STATE_NEXT,
      STATE_PREV,
      STATE_EXEC,
      STATE_DONE,
    };

    UIFunction(BrewBot *brewBot, Display *display);

    void setup(void);
    void loop(void);

    void display(void);

    void setName(char *name);
    void setProbe(OneWireTemperatureDevice *devProbe);
    void setNumSteps(unsigned numSteps);

    void setState(states state);
    states getState();

  protected:
    BrewBot *_brewBot;
    Display *_display;

  private:
    static void handleButtons(void *ptr, int id, bool held);

    Buttons _buttons;

    states _state;

    char _name[UIFUNCTION_NAME_LEN];
    char _nameDisplay[UIFUNCTION_NAME_DISP_LEN];

    OneWireTemperatureDevice *_devProbe;

    unsigned int _numSteps;
    unsigned int _step;

    unsigned long _nextTickBlink;
    unsigned long _nextTickTimer;
    unsigned long _nextTickReminder;
    unsigned long _nextTickBeeper;

    unsigned int _numBeeps;

    unsigned long _time[UIFUNCTION_MAX_STEPS];
    double _targetTemp[UIFUNCTION_MAX_STEPS];
    double _probeTemp;

    bool nextStep(void);
    bool setStep(unsigned step);
    void setTime(double time);

    char *getName();
    unsigned long getTime();
    double getTargetTemp();
    double getProbeTemp();

    bool updateProbeTemp();
    bool updateTimer();
    bool updateReminder();
    bool updateBeeper();

    void displayBlinkTime();
    void displayBlinkTemp();
    void displayBlinkIndicator();
    void displayProbeTemp();
    void displayTimer();

    void keyPressTime(unsigned key, bool held);
    void keyPressTemp(unsigned key, bool held);
    void keyPressExec(unsigned key, bool held);
    void keyPressDone(unsigned key, bool held);
};

#endif
