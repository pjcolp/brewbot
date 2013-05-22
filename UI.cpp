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
  /* Setup display. */
  m_display.setup();

  /* Setup subfunctions. */
  m_mash.setup();

  /* Start-up message. */
  m_display.lcd.setCursor(0, 0);
  m_display.lcd.print("BrewBot  v1.0");
  m_display.lcd.setCursor(0, 1);
  m_display.lcd.print("Initialising...");
  delay(2000);

  /* Set initial state. */
  setState(STATE_MENU);
}

void UI::setState(UI::states state)
{
  switch(state)
  {
    case STATE_MENU:
    {
      /* Specific transition stuff based on previous state. */
      switch(m_state)
      {
        case STATE_MASH:
        {
          break;
        }
      }

      m_display.printMenu(m_menuPosition);

      /* Blink the select menu item show it has focus. */
      m_displayBlinkEvent = m_displayTimer.every(500, displayBlinkMenuItem, this);

      break;
    }

    case STATE_MASH:
    {
      switch (m_state)
      {
        case STATE_MENU:
        {
          m_displayTimer.stop(m_displayBlinkEvent);
          break;
        }
      }

      m_mash.display();

      break;
    }

    default:
      break;
  }

  m_state = state;
}

UI::states UI::getState()
{
  return m_state;
}

/* Menu mode key press handler. */
void UI::keyPress(unsigned key)
{
  switch (key)
  {
    case KEY_RIGHT:
    case KEY_SELECT:
    {
      switch (m_menuPosition)
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
      if (m_menuPosition > 0)
      {
        m_menuPosition--;
        m_display.printMenu(m_menuPosition);
      }
      break;
    }

    case KEY_DOWN:
    {
      if (m_menuPosition < UI_MENU_MAX)
      {
        m_menuPosition++;
        m_display.printMenu(m_menuPosition);
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

/* Main loop */
void UI::loop(void)
{
  switch(m_state)
  {
    case STATE_MENU:
    {
      m_displayTimer.update();
      m_buttons.update();

      break;
    }

    case STATE_MASH:
    {
      m_mash.loop();
      setState(STATE_MENU);

      break;
    }

    default:
      break;
  }
}

void UI::displayBlinkMenuItem(void *ptr)
{
  UI *ui = (UI *)ptr;
  static boolean blink = true;
  
  if (blink)
  {
    switch (ui->m_menuPosition)
    {
      case UI_MENU_MASH:
      {
        ui->m_display.clearMenuMash(0, 0);
        break;
      }

      case UI_MENU_SPARGE:
      {
        ui->m_display.clearMenuSparge(0, 0);
        break;
      }

      case UI_MENU_BOIL:
      {
        ui->m_display.clearMenuBoil(0, 0);
        break;
      }
    }
  }
  else
  {
    switch (ui->m_menuPosition)
    {
      case UI_MENU_MASH:
      {
        ui->m_display.printMenuMash(0, 0);
        break;
      }

      case UI_MENU_SPARGE:
      {
        ui->m_display.printMenuSparge(0, 0);
        break;
      }

      case UI_MENU_BOIL:
      {
        ui->m_display.printMenuBoil(0, 0);
        break;
      }
    }
  }

  blink = !blink;
}

