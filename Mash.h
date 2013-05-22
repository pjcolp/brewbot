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

#ifndef MASH_H
#define MASH_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif
#include <Timer.h>
#include "pins.h"
#include "BrewTimer.h"
#include "Buttons.h"
#include "Display.h"

#define MASH_TIME_MAX 599 // 9h59m

class Mash
{
  public:
    enum states {
      STATE_MENU,
      STATE_TIME,
      STATE_TEMP,
      STATE_EXEC,
      STATE_DONE,
    };

    Mash(Display *display) : m_display(display), m_buttons(Buttons(handleButtons, this)), m_targetTemp(25) {};
    ~Mash(){};

    void setup();
    void loop();

    void setState(states state);
    states getState();

    float getTargetTemp();

    void timeKeyPress(unsigned key);
    void tempKeyPress(unsigned key);
    void execKeyPress(unsigned key);
    void doneKeyPress(unsigned key);

    void display(void);

  protected:
    Display *m_display;
    float m_targetTemp;

  private:
    static void displayBlinkTime(void *ptr);
    static void displayBlinkTargetTemp(void *ptr);
    static void handleButtons(void *ptr, int id, bool held);

    BrewTimer m_brewTimer;

    Buttons m_buttons;
    states m_state;

    Timer m_displayTimer;
    uint8_t m_displayBlinkEvent;
};

#endif
