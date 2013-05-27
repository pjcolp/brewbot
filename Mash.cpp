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

Mash::Mash(Display *display, BooleanDevice *devIndicator, BooleanDevice *devBuzzer)
: _display(display), _devIndicator(devIndicator), _devBuzzer(devBuzzer), _buttons(Buttons(handleButtons, this)), _targetTemp(25), _brewTimer(BrewTimer(_devBuzzer))
{
}

void Mash::setup(void)
{
  _brewTimer.setup();
  _time = _brewTimer.getTime();
}

void Mash::loop(void)
{
  delay(500);
  setState(STATE_TIME);

  while (_state != STATE_MENU)
  {
    switch (_state)
    {
      case STATE_MENU:
      {
        break;
      }

      case STATE_TIME:
      case STATE_TEMP:
      {
        _buttons.update();
        _displayTimer.update();

        break;
      }

      case STATE_EXEC:
      {
        _buttons.update();
        _displayTimer.update();
        _brewTimer.update();

        /* Update the time on the display. */
        unsigned long time = _brewTimer.getTime();
        if (_time != time)
        {
          _time = time;
          _display->printTime(_time);

          /* Check if we're done. */
          if (_time == 0)
          {
            setState(STATE_DONE);
          }

        }

        break;
      }

      case STATE_DONE:
      {
        _buttons.update();
        _brewTimer.update();

        break;
      }

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
          /* Stop blinking. */
          _displayTimer.stop(_displayBlinkEvent);

          break;
        }

        case STATE_EXEC:
        {
          /* Stop blinking. */
          _displayTimer.stop(_displayBlinkEvent);

          /* Stop timer. */
          _brewTimer.stop();

          /* Turn off indicator light. */
          _devIndicator->Write(false);

          break;
        }

        case STATE_DONE:
        {
          /* Stop blinking. */
          _displayTimer.stop(_displayBlinkEvent);

          /* Stop timer. */
          _brewTimer.stop();

          /* Turn off indicator light. */
          _devIndicator->Write(false);

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
          /* Stop blinking. */
          _displayTimer.stop(_displayBlinkEvent);
          _display->printTargetTemp(_targetTemp);

          break;
        }

        case STATE_EXEC:
        {
          /* Start beeping. */
          unsigned long now = millis();
          _devBuzzer->Write(true);

          /* Stop blinking. */
          _displayTimer.stop(_displayBlinkEvent);
          _display->printIndicator();

          /* Stop timer. */
          _brewTimer.stop();

          /* Turn off indicator light. */
          _devIndicator->Write(false);

          /* Finishing stalling and beeping. */
          while (now + 500 > millis());
          _devBuzzer->Write(false);

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
          /* Stop blinking. */
          _displayTimer.stop(_displayBlinkEvent);
          _display->printTime(_brewTimer.getTime());

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
      switch (_state)
      {
        case STATE_TIME:
        {
          /* Stop blinking. */
          _displayTimer.stop(_displayBlinkEvent);
          _display->printTime(_brewTimer.getTime());

          break;
        }

        case STATE_TEMP:
        {
          /* Stop blinking. */
          _displayTimer.stop(_displayBlinkEvent);
          _display->printTargetTemp(_targetTemp);

          break;
        }
      }

      /* Start beeping. */
      unsigned long now = millis();
      _devBuzzer->Write(true);

      /* Turn on indicator light. */
      _devIndicator->Write(true);

      /* Start the timer. */
      _brewTimer.begin();

      /* Blink the colon to show it's running. */
      _displayBlinkEvent = _displayTimer.every(500, displayBlinkIndicator, this);

      /* Finishing stalling and beeping. */
      while (now + 500 > millis());
      _devBuzzer->Write(false);

      break;
    }

    case STATE_DONE:
    {
      /* Specific transition stuff based on previous state. */
      switch (_state)
      {
        case STATE_EXEC:
        {
          /* Stop blinking. */
          _displayTimer.stop(_displayBlinkEvent);

          /* Turn off indicator light. */
          _devIndicator->Write(false);

          break;
        }
      }

      /* Put the timer into "done" mode. */
      _brewTimer.done();

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
      if (_time > 0)
      {
        _time--;
        _brewTimer.setTime(_time);
        _display->printTime(_time);
      }
      break;
    }

    /* Set temperature up. */
    case KEY_UP:
    {
      if (_time < MASH_TIME_MAX)
      {
        _time++;
        _brewTimer.setTime(_time);
        _display->printTime(_time);
      }
      break;
    }

    /* Start program. */
    case KEY_SELECT:
    {
      if (_time != 0)
      {
        setState(STATE_EXEC);
      }
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
      if (_time != 0)
      {
        setState(STATE_EXEC);
      }
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

void Mash::displayBlinkIndicator(void *ptr)
{
  Mash *mash = (Mash *)ptr;
  static bool blink = true;
  
  if (blink)
  {
    mash->_display->clearIndicator();
  }
  else
  {
    mash->_display->printIndicator();
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

    case STATE_DONE:
    {
      mash->doneKeyPress(id);

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

