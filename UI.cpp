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

#include "pins.h"
#include "BrewBot.h"
#include "Display.h"
#include "Mash.h"
#include "UI.h"

UI::UI(BrewBot *brewBot)
: _brewBot(brewBot), _buttons(Buttons(handleButtons, this)),
  _mash(Mash(_brewBot, &_display))
{
}

/* UI setup function. */
void UI::setup(void)
{
  unsigned long start_time = millis();

  /* Turn on indicator light. */
  _brewBot->devIndicator.Write(true);

  /* Turn on start-up beep. */
  _brewBot->devBeeper.Write(true);

  /* Setup display. */
  _display.setup();

  /* Start-up message. */
  _display.printStartupMessage();

  /* Setup subfunctions. */
  _mash.setup();

  /* XXX: Some delay so the init message is visible. */
  while (millis() < start_time + 2000)
  {
    if (millis() >= start_time + (BEEP_TIME * 2))
    {
      _brewBot->devBeeper.Write(false);
    }

    delay(100);
  }

  /* Turn off indicator light. */
  _brewBot->devIndicator.Write(false);

  /* Set initial state. */
  _menuPosition = UI_MENU_MASH;
  setState(STATE_MENU);
}

/* Main loop */
void UI::loop(void)
{
  switch (_state)
  {
    case STATE_MENU:
    {
      /* Blink the menu item. */
      displayBlinkMenuItem();

      /* Look for button presses. */
      _buttons.update();

      break;
    }

    case STATE_MASH:
    {
      /* Do mash stuff. */
      _mash.loop();

      /* Check if we've gone back to the main menu. */
      if (_mash.getState() == Mash::STATE_MENU)
      {
        setState(STATE_MENU);
      }

      break;
    }

    case STATE_BOIL:
    {
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
      _display.printMenu(_menuPosition);

      break;
    }

    case STATE_MASH:
    {
      /* Stop blinking. */
      _display.printMenu(_menuPosition);

      /* Switch element control to RIMS tube. */
#if 0
      brewBot->devElementControl.Write(ELEMENT_CONTROL_RIMS);
#endif

      /* Setup initial display. */
      _mash.display();

      /* Switch to default state. */
      _mash.setState(Mash::STATE_TIME);

      /* Pause so things don't happen too quickly. */
      delay(500);

      break;
    }

    case STATE_SPARGE:
    {
      /* Switch element control to RIMS tube. */
#if 0
      brewBot->devElementControl.Write(ELEMENT_CONTROL_RIMS);
#endif

#if 0
      /* Setup initial display. */
      _sparge.display();

      /* Switch to default state. */
      _sparge.setState(Sparge::STATE_TIME);
#endif

      /* Pause so things don't happen too quickly. */
      delay(500);

      break;
    }

    case STATE_BOIL:
    {
      /* Switch element control to brew kettle. */
#if 0
      brewBot->devElementControl.Write(ELEMENT_CONTROL_BK);
#endif

#if 0
      /* Setup initial display. */
      _boil.display();

      /* Switch to default state. */
      _boil.setState(Boil::STATE_TIME);
#endif

      /* Pause so things don't happen too quickly. */
      delay(500);

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
          setState(STATE_BOIL);
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

void UI::displayBlinkMenuItem()
{
  static bool blink = true;
  unsigned long now = millis();

  if (now >= _nextTickBlink)
  {
    switch (_menuPosition)
    {
      case UI_MENU_MASH:
      {
        if (blink)
        {
          _display.clearMenuMash(0, 0);
        }
        else
        {
          _display.printMenuMash(0, 0);
        }

        break;
      }

      case UI_MENU_SPARGE:
      {
        if (blink)
        {
          _display.clearMenuSparge(0, 0);
        }
        else
        {
          _display.printMenuSparge(0, 0);
        }

        break;
      }

      case UI_MENU_BOIL:
      {
        if (blink)
        {
          _display.clearMenuBoil(0, 0);
        }
        else
        {
          _display.printMenuBoil(0, 0);
        }

        break;
      }
    }

    blink = !blink;
    _nextTickBlink = now + BLINK_TIME;
  }
}
