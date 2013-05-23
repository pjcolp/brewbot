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

#include "BrewTimer.h"
#include "Display.h"
#include "UI.h"

/* UI setup function. */
void UI::setup(void)
{
  unsigned long start_time = millis();

  /* Setup display. */
  _display.setup();

  /* Start-up message. */
  _display.printStartupMessage();

  /* Setup subfunctions. */
  _mash.setup();

  /* XXX: Some delay so the init message is visible. */
  while (millis() < start_time + 2000)
  {
    delay(100);
  }

  /* Set initial state. */
  setState(STATE_MENU);
}

/* Main loop */
void UI::loop(void)
{
  switch(_state)
  {
    case STATE_MENU:
    {
      _displayTimer.update();
      _buttons.update();

      break;
    }

    case STATE_MASH:
    {
      _mash.loop();
      setState(STATE_MENU);

      break;
    }

    default:
      break;
  }
}

void UI::setState(UI::states state)
{
  switch(state)
  {
    case STATE_MENU:
    {
      /* Specific transition stuff based on previous state. */
      switch(_state)
      {
        case STATE_MASH:
        {
          break;
        }
      }

      _display.printMenu(_menuPosition);

      /* Blink the select menu item show it has focus. */
      _displayBlinkEvent = _displayTimer.every(500, displayBlinkMenuItem, this);

      break;
    }

    case STATE_MASH:
    {
      switch (_state)
      {
        case STATE_MENU:
        {
          _displayTimer.stop(_displayBlinkEvent);
          break;
        }
      }

      _mash.display();

      break;
    }

    default:
      break;
  }

  _state = state;
}

UI::states UI::getState()
{
  return _state;
}

/* Menu mode key press handler. */
void UI::keyPress(unsigned key)
{
  switch (key)
  {
    case KEY_RIGHT:
    case KEY_SELECT:
    {
      switch (_menuPosition)
      {
        case UI_MENU_MASH:
        {
          setState(STATE_MASH);
          break;
        }

        case UI_MENU_SPARGE:
        {
#if 0
          setState(STATE_SPARGE);
#endif
          break;
        }

        case UI_MENU_BOIL:
        {
#if 0
          setState(STATE_BOIL);
#endif
          break;
        }
      }
      break;
    }

    case KEY_UP:
    {
      if (_menuPosition > 0)
      {
        _menuPosition--;
        _display.printMenu(_menuPosition);
      }
      break;
    }

    case KEY_DOWN:
    {
      if (_menuPosition < UI_MENU_MAX)
      {
        _menuPosition++;
        _display.printMenu(_menuPosition);
      }
      break;
    }

    case KEY_LEFT:
    default:
      // do nothing
      break;
  }
}

void UI::handleButtons(void *ptr, int id, bool held)
{
  UI *ui = (UI *)(ptr);
  switch (ui->getState())
  {
    case STATE_MENU:
    {
      ui->keyPress(id);

      break;
    }

    case STATE_MASH:
    default:
      // Do nothing
      break;
  }
}

void UI::displayBlinkMenuItem(void *ptr)
{
  UI *ui = (UI *)ptr;
  static bool blink = true;
  
  if (blink)
  {
    switch (ui->_menuPosition)
    {
      case UI_MENU_MASH:
      {
        ui->_display.clearMenuMash(0, 0);
        break;
      }

      case UI_MENU_SPARGE:
      {
        ui->_display.clearMenuSparge(0, 0);
        break;
      }

      case UI_MENU_BOIL:
      {
        ui->_display.clearMenuBoil(0, 0);
        break;
      }
    }
  }
  else
  {
    switch (ui->_menuPosition)
    {
      case UI_MENU_MASH:
      {
        ui->_display.printMenuMash(0, 0);
        break;
      }

      case UI_MENU_SPARGE:
      {
        ui->_display.printMenuSparge(0, 0);
        break;
      }

      case UI_MENU_BOIL:
      {
        ui->_display.printMenuBoil(0, 0);
        break;
      }
    }
  }

  blink = !blink;
}

