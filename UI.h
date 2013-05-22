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

#include "Buttons.h"
#include "Mash.h"

#define UI_MENU_MASH    0
#define UI_MENU_SPARGE  1
#define UI_MENU_BOIL    2

#define UI_MENU_MAX     2

class UI
{
  public:
    enum states
    {
      STATE_MENU,
      STATE_MASH,
      STATE_SPARGE,
      STATE_BOIL,
    };

    UI()
    : m_buttons(Buttons(handleButtons, this)), m_mash(Mash(&m_display)) {};
    ~UI(){};

    void setup(void);
    void loop(void);

    void setState(states state);
    states getState(void);

    void keyPress(unsigned key);

  protected:
    Display m_display;

  private:
    static void displayBlinkMenuItem(void *ptr);
    static void handleButtons(void *cookie, int id, bool held);

    Buttons m_buttons;
    states m_state;

    Mash m_mash;

    int m_menuPosition;

    Timer m_displayTimer;
    uint8_t m_displayBlinkEvent;
};

#endif
