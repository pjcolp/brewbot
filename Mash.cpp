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
#include <AnalogButtons.h>
#include "pins.h"
#include "Buttons.h"
#include "BrewTimer.h"
#include "Mash.h"

void Mash::setup(void)
{
  m_brewTimer.setup();
}

void Mash::loop(void)
{
  delay(500);
  setState(STATE_TIME);

  while (m_state != STATE_MENU)
  {
    switch (m_state)
    {
      case STATE_TIME:
      case STATE_TEMP:
      {
        m_displayTimer.update();
        m_buttons.update();

        break;
      }

      case STATE_EXEC:
      {
        m_brewTimer.update();
        m_display->printTime(m_brewTimer.getTime());
        m_buttons.update();

        break;
      }

      case STATE_DONE:
      default:
        // Do nothing
        break;
    }
  }
}

Mash::states Mash::getState(void)
{
  return m_state;
}

void Mash::setState(states state)
{
  if (state == m_state)
  {
    return;
  }

  switch(state)
  {
    case STATE_MENU:
    {
      /* Specific transition stuff based on previous state. */
      switch(m_state)
      {
        case STATE_TIME:
        case STATE_TEMP:
        {
          m_displayTimer.stop(m_displayBlinkEvent);

          break;
        }

        case STATE_EXEC:
        {
          m_brewTimer.stop();

          break;
        }
      }

      m_display->clear();

      break;
    }

    case STATE_TIME:
    {
      /* Specific transition stuff based on previous state. */
      switch(m_state)
      {
        case STATE_TEMP:
        {
          m_displayTimer.stop(m_displayBlinkEvent);
          m_display->printTargetTemp(m_targetTemp);

          break;
        }

        case STATE_EXEC:
        {
          m_brewTimer.stop();

          break;
        }
      }

      /* Blink the timer to show it has focus. */
      m_displayBlinkEvent = m_displayTimer.every(500, displayBlinkTime, this);

      break;
    }

    case STATE_TEMP:
    {
      /* Specific transition stuff based on previous state. */
      switch(m_state)
      {
        case STATE_TIME:
        {
          m_displayTimer.stop(m_displayBlinkEvent);
          m_display->printTime(m_brewTimer.getTime());

          break;
        }

        case STATE_EXEC:
        {
          m_brewTimer.stop();

          break;
        }
      }

      /* Blink the target temperature to show it has focus. */
      m_displayBlinkEvent = m_displayTimer.every(500, displayBlinkTargetTemp, this);

      break;
    }

    case STATE_EXEC:
    {
      /* Specific transition stuff based on previous state. */
      m_displayTimer.stop(m_displayBlinkEvent);
      m_display->printTime(m_brewTimer.getTime());
      m_display->printTargetTemp(m_targetTemp);

      /* Start the timer. */
      m_brewTimer.begin();

      break;
    }

    default:
      break;
  }

  m_state = state;
}

/* Set the time. */
void Mash::timeKeyPress(unsigned key)
{
  switch (key)
  {
    /* Set temperature down. */
    case KEY_DOWN:
    {
      unsigned long time = m_brewTimer.getTime();
      if (time > 0)
      {
        time--;
        m_brewTimer.setTime(time);
        m_display->printTime(time);
      }
      break;
    }

    /* Set temperature up. */
    case KEY_UP:
    {
      unsigned long time = m_brewTimer.getTime();
      if (time < MASH_TIME_MAX)
      {
        time++;
        m_brewTimer.setTime(time);
        m_display->printTime(time);
      }
      break;
    }

    /* Start program. */
    case KEY_SELECT:
    {
      setState(STATE_EXEC);
      break;
    }

    /* Move focus to the timer. */
    case KEY_LEFT:
    {
      setState(STATE_MENU);
      break;
    }

    /* Move to the next step. */
    case KEY_RIGHT:
    {
      setState(STATE_TEMP);
      break;
    }

    case KEY_NONE:
    default:
      // do nothing
      break;
  }
}

/* Set the target temperature. */
void Mash::tempKeyPress(unsigned key)
{
  switch (key)
  {
    /* Move to the next step. */
    case KEY_RIGHT:
    {
      break;
    }

    /* Set temperature up. */
    case KEY_UP:
    {
      m_targetTemp += 0.5;
      m_display->printTargetTemp(m_targetTemp);
      break;
    }

    /* Set temperature down. */
    case KEY_DOWN:
    {
      m_targetTemp -= 0.5;
      m_display->printTargetTemp(m_targetTemp);
      break;
    }

    /* Move focus to the timer. */
    case KEY_LEFT:
    {
      setState(STATE_TIME);
      break;
    }

    /* Start program. */
    case KEY_SELECT:
    {
      setState(STATE_EXEC);
      break;
    }

    default:
      // do nothing
      break;
  }
}

/* Exec mode key press handler. */
void Mash::execKeyPress(unsigned key)
{
  switch (key)
  {
    case KEY_RIGHT:
    case KEY_UP:
    case KEY_DOWN:
    case KEY_LEFT:
    case KEY_SELECT:
    {
      setState(STATE_TIME);
      break;
    }

    default:
      // do nothing
      break;
  }
}

/* Done mode key press handler. */
void Mash::doneKeyPress(unsigned key)
{
  switch (key)
  {
    case KEY_RIGHT:
    case KEY_UP:
    case KEY_DOWN:
    case KEY_SELECT:
    {
      setState(STATE_TIME);
      break;
    }

    case KEY_LEFT:
    {
      setState(STATE_MENU);
      break;
    }

    default:
      // do nothing
      break;
  }
}

void Mash::displayBlinkTime(void *ptr)
{
  Mash *mash = (Mash *)ptr;
  static boolean blink = true;
  
  if (blink)
  {
    mash->m_display->clearTime();
  }
  else
  {
    mash->m_display->printTime(mash->m_brewTimer.getTime());
  }

  blink = !blink;
}

void Mash::displayBlinkTargetTemp(void *ptr)
{
  Mash *mash = (Mash *)ptr;
  static boolean blink = true;
  
  if (blink)
  {
    mash->m_display->clearTargetTemp();
  }
  else
  {
    mash->m_display->printTargetTemp(mash->m_targetTemp);
  }

  blink = !blink;
}

void Mash::handleButtons(void *ptr, int id, bool held)
{
  Mash *mash = (Mash *)ptr;

  switch (mash->getState())
  {
    case STATE_MENU:
    {
      mash->setState(STATE_TIME);

      break;
    }

    case STATE_TIME:
    {
      mash->timeKeyPress(id);

      break;
    }

    case STATE_TEMP:
    {
      mash->tempKeyPress(id);

      break;
    }

    case STATE_EXEC:
    {
      mash->execKeyPress(id);

      break;
    }
    default:
      // Do nothing
      break;
  }
}

float Mash::getTargetTemp()
{
  return m_targetTemp;
}

void Mash::display(void)
{
  m_display->printMash(getTargetTemp(), -127.00, m_brewTimer.getTime(), false);
}

