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
#include <DeviceManager.h>

#include "constants.h"
#include "pins.h"
#include "BrewBot.h"
#include "Buttons.h"
#include "UIFunction.h"

UIFunction::UIFunction(BrewBot *brewBot, Display *display)
: _brewBot(brewBot), _display(display),
  _buttons(Buttons(handleButtons, this)), _step(0), _probeTemp(0.00)
{
}

void UIFunction::setName(char *name)
{
  for (unsigned i = 0; i < UIFUNCTION_NAME_LEN; i++)
  {
    _name[i] = name[i];
    _nameDisplay[i] = name[i];
  }
}

void UIFunction::setProbe(OneWireTemperatureDevice *devProbe)
{
  _devProbe = devProbe;
}

void UIFunction::setNumSteps(unsigned numSteps)
{
  if (numSteps > UIFUNCTION_MAX_STEPS)
  {
    numSteps = UIFUNCTION_MAX_STEPS;
  }

  _numSteps = numSteps;
}

void UIFunction::setup(void)
{
  for (unsigned i = 0; i < UIFUNCTION_MAX_STEPS; i++)
  {
    _time[i] = UIFUNCTION_TIME_DEFAULT;
    _targetTemp[i] = UIFUNCTION_TEMP_DEFAULT;
  }
}

void UIFunction::loop(void)
{
  switch (_state)
  {
    case STATE_TIME:
    {
      /* Blink the time. */
      displayBlinkTime();

      /* Update the probe temperature. */
      displayProbeTemp();

      /* Look for button presses. */
      _buttons.update();

      break;
    }

    case STATE_TEMP:
    {
      /* Blink the target temperature. */
      displayBlinkTemp();

      /* Update the probe temperature. */
      displayProbeTemp();

      /* Look for button presses. */
      _buttons.update();

      break;
    }

    case STATE_NEXT:
    {
      setState(STATE_TIME);
      break;
    }

    case STATE_PREV:
    {
      setState(STATE_TEMP);
      break;
    }

    case STATE_EXEC:
    {
      /* Blink the ":" in the time. */
      displayBlinkIndicator();

      /* Update the probe temperature. */
      /* XXX: Plumb into RIMS element control? */
      displayProbeTemp();

      /* Update the timer. */
      displayTimer();

      /* Look for button presses. */
      _buttons.update();

      /* Check if this step is done. */
      if (_time[_step] == 0)
      {
        /* Check if there are any other steps. */
        if (nextStep())
        {
          /* Start beeping. */
          unsigned long now = millis();
          _brewBot->devBeeper.Write(true);

          /* Don't tick for 2 seconds.
           * One second to display zero, then another second to display the
           * start of the next timer. */
          _nextTickTimer = now + (TIMER_TIME * 2);

          /* Finishing stalling and beeping. */
          while (now + BEEP_TIME > millis());
          _brewBot->devBeeper.Write(false);

          /* Wait for "0:00" to display for one second. */
          while (now + (BEEP_TIME *2) > millis());

          /* Update display. */
          display();
        }
        else
        {
          setState(STATE_DONE);
        }
      }

      break;
    }

    case STATE_DONE:
    {
      /* Beep if we still need to. */
      if (_numBeeps > 0)
      {
        if (updateBeeper())
        {
          _numBeeps--;
        }
      }

      /* Update the reminder timer. */
      updateReminder();

      /* Update the probe temperature. */
      /* XXX: Plumb into RIMS element control? */
      displayProbeTemp();

      /* Look for button presses. */
      _buttons.update();

      break;
    }

    default:
      // Do nothing
      break;
  }
}

inline bool UIFunction::nextStep()
{
  setStep(_step + 1);
  return (_time[_step] != 0);
}

inline bool UIFunction::setStep(unsigned step)
{
  /* Check if this is a valid step. */
  if (step < _numSteps)
  {
    _step = step;

    return true;
  }

  return false;
}

inline void UIFunction::setTime(double time)
{
  _time[_step] = time;
}

UIFunction::states UIFunction::getState(void)
{
  return _state;
}

void UIFunction::setState(states state)
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
        case STATE_EXEC:
        case STATE_DONE:
        {
          /* Turn off indicator light. */
          _brewBot->devIndicator.Write(false);

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
          _display->printTargetTemp(getTargetTemp());

          break;
        }

        case STATE_EXEC:
        {
          /* Start beeping. */
          unsigned long now = millis();
          _brewBot->devBeeper.Write(true);

          /* Stop blinking. */
          _display->printIndicator();

          /* Turn off indicator light. */
          _brewBot->devIndicator.Write(false);

          /* Finishing stalling and beeping. */
          while (now + BEEP_TIME > millis());
          _brewBot->devBeeper.Write(false);

          break;
        }

        case STATE_DONE:
        {
          /* Start beeping. */
          unsigned long now = millis();
          _brewBot->devBeeper.Write(true);

          /* Move to the first step. */
          setStep(0);

          /* Reset to default time. */
          setTime(UIFUNCTION_TIME_DEFAULT);

          /* Reset display. */
          display();

          /* Finishing stalling and beeping. */
          while (now + BEEP_TIME > millis());
          _brewBot->devBeeper.Write(false);

          break;
        }
      }

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
          _display->printTime(getTime());

          break;
        }
      }

      break;
    }

    case STATE_NEXT:
    {
      /* Specific transition stuff based on previous state. */
      switch(_state)
      {
        case STATE_TEMP:
        {
          /* Stop blinking. */
          _display->printTargetTemp(getTargetTemp());

          break;
        }
      }

      if (setStep(_step + 1))
      {
        display();
      }

      break;
    }

    case STATE_PREV:
    {
      /* Specific transition stuff based on previous state. */
      switch(_state)
      {
        case STATE_TIME:
        {
          /* Stop blinking. */
          _display->printTime(getTime());

          break;
        }
      }

      if (setStep(_step - 1))
      {
        display();
      }

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
          _display->printTime(getTime());

          break;
        }

        case STATE_TEMP:
        {
          /* Stop blinking. */
          _display->printTargetTemp(getTargetTemp());

          break;
        }
      }

      /* Start beeping. */
      unsigned long now = millis();
      _brewBot->devBeeper.Write(true);

      /* Turn on indicator light. */
      _brewBot->devIndicator.Write(true);

      /* Move to first (active) step. */
      for (unsigned i = 0; i < UIFUNCTION_MAX_STEPS; i++)
      {
        if (_time[i])
        {
          setStep(i);
          break;
        }
      }
      display();

      /* Start the timer. */
      _nextTickTimer = now + TIMER_TIME;
      _nextTickBlink = now + BLINK_TIME;

      /* Finishing stalling and beeping. */
      while (now + BEEP_TIME > millis());
      _brewBot->devBeeper.Write(false);

      break;
    }

    case STATE_DONE:
    {
      /* Specific transition stuff based on previous state. */
      switch (_state)
      {
        case STATE_EXEC:
        {
          /* Turn off indicator light. */
          _brewBot->devIndicator.Write(false);

          /* Make sure the ":" in the time appears. */
          _display->printIndicator();

          break;
        }
      }

      /* Put the timer into "done" mode. */
      unsigned long now = millis();
      _nextTickReminder = now + REMINDER_TIME;
      _nextTickBeeper = now + BEEP_TIME;
      _numBeeps = 6;

      break;
    }

    default:
      break;
  }

  _state = state;
}

/* Set the time. */
void UIFunction::keyPressTime(unsigned key, bool held)
{
  switch (key)
  {
    /* Set temperature down. */
    case KEY_DOWN:
    {
      if (_time[_step] > UIFUNCTION_TIME_MIN)
      {
        if (held)
        {
          if (_time[_step] > (UIFUNCTION_TIME_MIN + 10)) {
            _time[_step] -= 10;
          }
          else
          {
            setTime(UIFUNCTION_TIME_MIN);
          }
        }
        else
        {
          setTime(_time[_step] - 1);

        }

        _display->printTime(_time[_step]);
      }

      break;
    }

    /* Set temperature up. */
    case KEY_UP:
    {
      if (_time[_step] < UIFUNCTION_TIME_MAX)
      {
        if (held)
        {
          if (_time[_step] < UIFUNCTION_TIME_MAX - 10)
          {
            setTime(_time[_step] + 10);
          }
          else
          {
            setTime(UIFUNCTION_TIME_MAX);
          }
        }
        else
        {
          setTime(_time[_step] + 1);
        }

        _display->printTime(_time[_step]);
      }

      break;
    }

    /* Start program. */
    case KEY_SELECT:
    {
      if (_time[_step] != 0)
      {
        setState(STATE_EXEC);
      }
      break;
    }

    /* Move focus to the timer. */
    case KEY_LEFT:
    {
      if (_step != 0)
      {
        setState(STATE_PREV);
      }
      else
      {
        setState(STATE_MENU);
      }

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
void UIFunction::keyPressTemp(unsigned key, bool held)
{
  switch (key)
  {
    /* Set temperature up. */
    case KEY_UP:
    {
      if (_targetTemp[_step] < UIFUNCTION_TEMP_MAX)
      {
        if (held)
        {
          if (_targetTemp[_step] < UIFUNCTION_TEMP_MAX - 10)
          {
            _targetTemp[_step] += 10;
          }
          else
          {
            _targetTemp[_step] = UIFUNCTION_TEMP_MAX;
          }
        }
        else
        {
          _targetTemp[_step] += 0.5;
        }

        _display->printTargetTemp(_targetTemp[_step]);
      }

      break;
    }

    /* Set temperature down. */
    case KEY_DOWN:
    {
      if (_targetTemp[_step] > UIFUNCTION_TEMP_MIN)
      {
        if (held)
        {
          if (_targetTemp[_step] > (UIFUNCTION_TEMP_MIN + 10))
          {
            _targetTemp[_step] -= 10;
          }
          else
          {
            _targetTemp[_step] = UIFUNCTION_TEMP_MIN;
          }
        }
        else
        {
          _targetTemp[_step] -= 0.5;
        }

        _display->printTargetTemp(_targetTemp[_step]);
      }

      break;
    }

    /* Move to the next step. */
    case KEY_RIGHT:
    {
      if (_step < (_numSteps - 1))
      {
        setState(STATE_NEXT);
      }

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
      if (_time[_step] != 0)
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
void UIFunction::keyPressExec(unsigned key, bool held)
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
void UIFunction::keyPressDone(unsigned key, bool held)
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

void UIFunction::displayBlinkTime()
{
  static bool blink = true;
  unsigned long now = millis();

  /* Is it time to blink? */
  if (now >= _nextTickBlink)
  {
    if (blink)
    {
      _display->clearTime();
    }
    else
    {
      _display->printTime(getTime());
    }

    blink = !blink;
    _nextTickBlink = now + BLINK_TIME;
  }
}

void UIFunction::displayBlinkTemp()
{
  static bool blink = true;
  unsigned long now = millis();

  /* Is it time to blink? */
  if (now >= _nextTickBlink)
  {
    if (blink)
    {
      _display->clearTargetTemp();
    }
    else
    {
      _display->printTargetTemp(getTargetTemp());
    }

    blink = !blink;
    _nextTickBlink = now + BLINK_TIME;
  }
}

void UIFunction::displayBlinkIndicator()
{
  static bool blink = true;
  unsigned long now = millis();

  /* Is it time to blink? */
  if (now >= _nextTickBlink)
  {
    if (blink)
    {
      _display->clearIndicator();
    }
    else
    {
      _display->printIndicator();
    }

    blink = !blink;
    _nextTickBlink = now + BLINK_TIME;
  }
}

void UIFunction::displayProbeTemp()
{
  if (updateProbeTemp())
  {
    _display->printProbeTemp(_probeTemp);
  }
}

void UIFunction::displayTimer()
{
  if (updateTimer())
  {
    _display->printTime(_time[_step]);
  }
}

void UIFunction::display(void)
{
  _display->printUIFunction(getName(), getTargetTemp(), getProbeTemp(), getTime(), false);
}

void UIFunction::handleButtons(void *ptr, int id, bool held)
{
  UIFunction *uiFunction = (UIFunction *)ptr;

  switch (uiFunction->getState())
  {
    case STATE_MENU:
    {
      uiFunction->setState(STATE_TIME);

      break;
    }

    case STATE_TIME:
    {
      uiFunction->keyPressTime(id, held);

      break;
    }

    case STATE_TEMP:
    {
      uiFunction->keyPressTemp(id, held);

      break;
    }

    case STATE_EXEC:
    {
      uiFunction->keyPressExec(id, held);

      break;
    }

    case STATE_DONE:
    {
      uiFunction->keyPressDone(id, held);

      break;
    }

    default:
      // Do nothing
      break;
  }
}

char *UIFunction::getName()
{
  if (_numSteps > 1)
  {
    unsigned i = UIFUNCTION_NAME_LEN;
    _nameDisplay[i++] = ' ';
    itoa(_step + 1, &_nameDisplay[i++], 10);
    _nameDisplay[i++] = '\0';
  }
  else
  {
    for (unsigned i = UIFUNCTION_NAME_LEN; i <UIFUNCTION_NAME_DISP_LEN; i++)
    {
      _nameDisplay[i] = ' ';
    }
  }

  return _nameDisplay;
}

inline unsigned long UIFunction::getTime()
{
  return _time[_step];
}

inline double UIFunction::getTargetTemp()
{
  return _targetTemp[_step];
}

inline double UIFunction::getProbeTemp()
{
  return _probeTemp;
}

bool UIFunction::updateProbeTemp()
{
  bool updated = false;

  /* Read temperature. */
  if (_devProbe->report_status)
  {
    _probeTemp = _devProbe->Read();
    _devProbe->report_status = false;
    updated = true;
  }

  return updated;
}

bool UIFunction::updateTimer()
{
  unsigned long now = millis();
  bool updated = false;

  if (now >= _nextTickTimer)
  {
    /* Tick timer down. */
    if (_time[_step] > 0)
    {
      _time[_step]--;
      updated = true;
    }

    /* Set next tick. */
    _nextTickTimer = now + TIMER_TIME;
  }

  return updated;
}

bool UIFunction::updateReminder()
{
  unsigned long now = millis();
  bool updated = false;

  if (now >= _nextTickReminder)
  {
    /* Beep. */
    _numBeeps = 2;

    /* Set next tick. */
    _nextTickReminder = now + REMINDER_TIME;

    updated = true;
  }

  return updated;
}

bool UIFunction::updateBeeper()
{
  static bool beep = false;
  unsigned long now = millis();
  bool updated = false;

  if (now >= _nextTickBeeper)
  {
    /* Toggle beeper. */
    beep = !beep;
    _brewBot->devBeeper.Write(beep);

    _nextTickBeeper = now + BEEP_TIME;

    updated = true;
  }

  return updated;
}
