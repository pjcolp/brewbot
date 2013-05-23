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
  _brewTimer.setup();
}

void Mash::loop(void)
{
  while (_state != STATE_MENU)
  {
    switch (_state)
    {
      case STATE_MENU:
      {
        setState(STATE_TIME);
        break;
      }

      case STATE_TIME:
      case STATE_TEMP:
      {
        _displayTimer.update();
        _buttons.update();

        break;
      }

      case STATE_EXEC:
      {
        _brewTimer.update();
        _display->printTime(_brewTimer.getTime());
        _buttons.update();

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
  return _state;
}

void Mash::setState(states state)
{
  if (state == _state)
  {
    return;
  }

  switch(state)
  {
    case STATE_MENU:
    {
      /* Specific transition stuff based on previous state. */
      switch(_state)
      {
        case STATE_TIME:
        case STATE_TEMP:
        {
          _displayTimer.stop(_displayBlinkEvent);

          break;
        }

        case STATE_EXEC:
        {
          _brewTimer.stop();

          break;
        }
      }

      _display->clear();

      break;
    }

    case STATE_TIME:
    {
      /* Specific transition stuff based on previous state. */
      switch(_state)
      {
        case STATE_TEMP:
        {
          _displayTimer.stop(_displayBlinkEvent);
          _display->printTargetTemp(_targetTemp);

          break;
        }

        case STATE_EXEC:
        {
          _brewTimer.stop();

          break;
        }
      }

      /* Blink the timer to show it has focus. */
      _displayBlinkEvent = _displayTimer.every(500, displayBlinkTime, this);

      break;
    }

    case STATE_TEMP:
    {
      /* Specific transition stuff based on previous state. */
      switch(_state)
      {
        case STATE_TIME:
        {
          _displayTimer.stop(_displayBlinkEvent);
          _display->printTime(_brewTimer.getTime());

          break;
        }

        case STATE_EXEC:
        {
          _brewTimer.stop();

          break;
        }
      }

      /* Blink the target temperature to show it has focus. */
      _displayBlinkEvent = _displayTimer.every(500, displayBlinkTargetTemp, this);

      break;
    }

    case STATE_EXEC:
    {
      /* Specific transition stuff based on previous state. */
      _displayTimer.stop(_displayBlinkEvent);
      _display->printTime(_brewTimer.getTime());
      _display->printTargetTemp(_targetTemp);

      /* Start the timer. */
      _brewTimer.begin();

      break;
    }

    default:
      break;
  }

  _state = state;
}

/* Set the time. */
void Mash::timeKeyPress(unsigned key)
{
  switch (key)
  {
    /* Set temperature down. */
    case KEY_DOWN:
    {
      unsigned long time = _brewTimer.getTime();
      if (time > 0)
      {
        time--;
        _brewTimer.setTime(time);
        _display->printTime(time);
      }
      break;
    }

    /* Set temperature up. */
    case KEY_UP:
    {
      unsigned long time = _brewTimer.getTime();
      if (time < MASH_TIME_MAX)
      {
        time++;
        _brewTimer.setTime(time);
        _display->printTime(time);
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
      _targetTemp += 0.5;
      _display->printTargetTemp(_targetTemp);
      break;
    }

    /* Set temperature down. */
    case KEY_DOWN:
    {
      _targetTemp -= 0.5;
      _display->printTargetTemp(_targetTemp);
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
  static bool blink = true;
  
  if (blink)
  {
    mash->_display->clearTime();
  }
  else
  {
    mash->_display->printTime(mash->_brewTimer.getTime());
  }

  blink = !blink;
}

void Mash::displayBlinkTargetTemp(void *ptr)
{
  Mash *mash = (Mash *)ptr;
  static bool blink = true;
  
  if (blink)
  {
    mash->_display->clearTargetTemp();
  }
  else
  {
    mash->_display->printTargetTemp(mash->_targetTemp);
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
  return _targetTemp;
}

void Mash::display(void)
{
  _display->printMash(getTargetTemp(), -127.00, _brewTimer.getTime(), false);
}

