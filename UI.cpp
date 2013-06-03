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
#include "UI.h"

UI::UI(BrewBot *brewBot)
: _brewBot(brewBot), _buttons(Buttons(handleButtons, this)),
  _name({ "MASH  ", "SPARGE", "BOIL  " }),
  _step(0), _probeTemp(0.00)
{
}

/* UI setup function. */
void UI::setup(void)
{
  unsigned long start_time = millis();

  /* Setup display. */
  _display.setup();

  /* Start-up message. */
  _display.printStartupMessage();

  /* Turn on indicator light. */
  _brewBot->devIndicator.Write(true);

  /* Turn on start-up beep. */
  _brewBot->devBeeper.Write(true);

  /* Setup default times and temps. */
  for (_function = 0; _function < UI_MAX_FUNCS; _function++)
  {
    for (_step = 0; _step < UI_MAX_STEPS; _step++)
    {
      setTime(UI_TIME_DEFAULT);
      setTargetTemp(UI_TEMP_DEFAULT);
    }
  }

  _function = 0;
  _step = 0;

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
    case STATE_SPARGE:
    case STATE_BOIL:
    {
      setState(STATE_TIME);

      break;
    }

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
      /* XXX: Plumb into element control? */
      displayProbeTemp();

      /* Update the timer. */
      displayTimer();

      /* Look for button presses. */
      _buttons.update();

      /* Check if this step is done. */
      if (_time[_function][_step] == 0)
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
        case STATE_TIME:
        {
          switch (_function)
          {
            case UI_FUNC_BOIL:
            {
              _brewBot->devFan.Write(false);

              break;
            }

            case UI_FUNC_MASH:
            case UI_FUNC_SPARGE:
            default:
              break;
          }

          break;
        }

        case STATE_DONE:
        {
          switch (_function)
          {
            case UI_FUNC_BOIL:
            {
              _brewBot->devFan.Write(false);

              break;
            }

            case UI_FUNC_MASH:
            case UI_FUNC_SPARGE:
            default:
              break;
          }

          /* Turn off PID device. */
          _devPID->enable(false);

          /* Turn off indicator light. */
          _brewBot->devIndicator.Write(false);

          break;
        }

        default:
          break;
      }

      _display.printMenu(_menuPosition);

      break;
    }

    case STATE_MASH:
    {
      /* Switch element control to RIMS tube. */
#if 0
      brewBot->devElementControl.Write(ELEMENT_CONTROL_RIMS);
#endif

      /* Setup initial display. */
      setFunction(UI_FUNC_MASH);
      setNumSteps(UI_MAX_STEPS);
      setProbeDev(&(_brewBot->devProbeRIMS));
      setPIDDev(&(_brewBot->devPIDRIMS));

      /* Stop blinking. */
      _display.printMenu(_menuPosition);

      /* Display sub-function. */
      display();

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

      /* Setup initial display. */
      setFunction(UI_FUNC_SPARGE);
      setNumSteps(1);
      setProbeDev(&(_brewBot->devProbeRIMS));
      setPIDDev(&(_brewBot->devPIDRIMS));

      /* Stop blinking. */
      _display.printMenu(_menuPosition);

      /* Display sub-function. */
      display();

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

      /* Setup initial display. */
      setFunction(UI_FUNC_BOIL);
      setNumSteps(UI_MAX_STEPS);
      setProbeDev(&(_brewBot->devProbeBK));
      setPIDDev(&(_brewBot->devPIDBK));

      /* Specific device setup. */
      _brewBot->devFan.Write(true);

      /* Stop blinking. */
      _display.printMenu(_menuPosition);

      /* Display sub-function. */
      display();

      /* Pause so things don't happen too quickly. */
      delay(500);

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
          _display.printTargetTemp(getTargetTemp());

          break;
        }

        case STATE_EXEC:
        {
          /* Start beeping. */
          unsigned long now = millis();
          _brewBot->devBeeper.Write(true);

          /* Stop blinking. */
          _display.printIndicator();

          /* Turn off PID device. */
          _devPID->enable(false);

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

          /* Turn off PID device. */
          _devPID->enable(false);

          /* Turn off indicator light. */
          _brewBot->devIndicator.Write(false);

          /* Move to the first step. */
          setStep(0);

          /* Reset to default time. */
          setTime(UI_TIME_DEFAULT);

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
          _display.printTime(getTime());

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
          _display.printTargetTemp(getTargetTemp());

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
          _display.printTime(getTime());

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
          _display.printTime(getTime());

          break;
        }

        case STATE_TEMP:
        {
          /* Stop blinking. */
          _display.printTargetTemp(getTargetTemp());

          break;
        }
      }

      /* Start beeping. */
      unsigned long now = millis();
      _brewBot->devBeeper.Write(true);

      /* Turn on indicator light. */
      _brewBot->devIndicator.Write(true);

      /* Turn on PID device. */
      _devPID->enable(true);

      /* Move to first (active) step. */
      for (unsigned int i = 0; i < UI_MAX_STEPS; i++)
      {
        if (_time[_function][i])
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
#if 0
          /* Turn off PID device. */
          _devPID->enable(false);

          /* Turn off indicator light. */
          _brewBot->devIndicator.Write(false);
#endif

          /* Make sure the ":" in the time appears. */
          _display.printIndicator();

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

  /* We're done switching states. */
  _state = state;
}

/* Done mode key press handler. */
void UI::keyPressDone(unsigned int key, bool held)
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

/* Exec mode key press handler. */
void UI::keyPressExec(unsigned int key, bool held)
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

/* Menu mode key press handler. */
void UI::keyPressMenu(unsigned int key, bool held)
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
          setState(STATE_SPARGE);

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
      break;
  }
}

/* Temp mode key press handler. */
void UI::keyPressTemp(unsigned int key, bool held)
{
  switch (key)
  {
    /* Set temperature up. */
    case KEY_UP:
    {
      if (getTargetTemp() < UI_TEMP_MAX)
      {
        if (held)
        {
          /* Prevent it from blinking. */
          _nextTickBlink += 500;

          /* Set the new temperature. */
          if (getTargetTemp() < UI_TEMP_MAX - 10)
          {
            setTargetTemp(getTargetTemp() + 10);
          }
          else
          {
            setTargetTemp(UI_TEMP_MAX);
          }
        }
        else
        {
          setTargetTemp(getTargetTemp() + 0.5);
        }

        _display.printTargetTemp(getTargetTemp());
      }

      break;
    }

    /* Set temperature down. */
    case KEY_DOWN:
    {
      if (getTargetTemp() > UI_TEMP_MIN)
      {
        if (held)
        {
          /* Prevent it from blinking. */
          _nextTickBlink += 500;

          /* Set the new temperature. */
          if (getTargetTemp() > (UI_TEMP_MIN + 10))
          {
            setTargetTemp(getTargetTemp() - 10);
          }
          else
          {
            setTargetTemp(UI_TEMP_MIN);
          }
        }
        else
        {
          setTargetTemp(getTargetTemp() - 0.5);
        }

        _display.printTargetTemp(getTargetTemp());
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

/* Time mode key press. */
void UI::keyPressTime(unsigned int key, bool held)
{
  switch (key)
  {
    /* Set time up. */
    case KEY_UP:
    {
      if (getTime() < UI_TIME_MAX)
      {
        if (held)
        {
          /* Prevent it from blinking. */
          _nextTickBlink += 500;

          /* Set the new time. */
          if (getTime() < UI_TIME_MAX - 10)
          {
            setTime(getTime() + 10);
          }
          else
          {
            setTime(UI_TIME_MAX);
          }
        }
        else
        {
          setTime(getTime() + 1);
        }

        _display.printTime(getTime());
      }

      break;
    }

    /* Set temperature down. */
    case KEY_DOWN:
    {
      if (getTime() > UI_TIME_MIN)
      {
        if (held)
        {
          /* Prevent it from blinking. */
          _nextTickBlink += 500;

          /* Set the new time. */
          if (getTime() > (UI_TIME_MIN + 10)) {
            setTime(getTime() - 10);
          }
          else
          {
            setTime(UI_TIME_MIN);
          }
        }
        else
        {
          setTime(getTime() - 1);

        }

        _display.printTime(getTime());
      }

      break;
    }

    /* Start program. */
    case KEY_SELECT:
    {
      if (getTime() != 0)
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

void UI::handleButtons(void *ptr, int id, bool held)
{
  UI *ui = (UI *)(ptr);
  switch (ui->getState())
  {
    case STATE_MENU:
    {
      ui->keyPressMenu(id, held);

      break;
    }

    case STATE_TIME:
    {
      ui->keyPressTime(id, held);

      break;
    }

    case STATE_TEMP:
    {
      ui->keyPressTemp(id, held);

      break;
    }

    case STATE_EXEC:
    {
      ui->keyPressExec(id, held);

      break;
    }

    case STATE_DONE:
    {
      ui->keyPressDone(id, held);

      break;
    }

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


void UI::setFunction(unsigned int function)
{
  if (function >= UI_MAX_FUNCS)
  {
    function = UI_MAX_FUNCS - 1;
  }

  setName(function);

  _function = function;
}

inline void UI::setName(unsigned int function)
{
  for (unsigned int i = 0; i < (UI_NAME_LEN - 1); i++)
  {
    _nameDisplay[i] = _name[function][i];
  }
}

void UI::setProbeDev(OneWireTemperatureDevice *devProbe)
{
  _devProbe = devProbe;
  _probeTemp = _devProbe->Read();
}

void UI::setPIDDev(PidRelayDevice *devPID)
{
  _devPID = devPID;
}

void UI::setNumSteps(unsigned int numSteps)
{
  if (numSteps > UI_MAX_STEPS)
  {
    numSteps = UI_MAX_STEPS;
  }

  _numSteps = numSteps;
}

void UI::setTargetTemp(double temp)
{
  if (temp > UI_TEMP_MAX)
  {
    temp = UI_TEMP_MAX;
  }

  /* Update PID set point. */
  double normalised = temp / (UI_TEMP_MAX - UI_TEMP_MIN);
  double setPoint = normalised * PID_MAX;
  if (_devPID)
  {
    _devPID->Write(setPoint);
  }

  _targetTemp[_function][_step] = temp;
}

inline bool UI::nextStep()
{
  setStep(_step + 1);
  return (_time[_function][_step] != 0);
}

inline bool UI::setStep(unsigned int step)
{
  /* Check if this is a valid step. */
  if (step < _numSteps)
  {
    _step = step;

    return true;
  }

  return false;
}

inline void UI::setTime(double time)
{
  _time[_function][_step] = time;
}

UI::states UI::getState(void)
{
  return _state;
}

/* Display a sub-function. */
void UI::display(void)
{
  _display.printUIFunction(getName(), getTargetTemp(), getProbeTemp(), getTime(), false);
}

void UI::displayBlink(void (*clear)(void), void (*print)(void))
{
  static bool blink = true;
  unsigned long now = millis();

  /* Is it time to blink? */
  if (now >= _nextTickBlink)
  {
    if (blink)
    {
      clear();
    }
    else
    {
      print();
    }

    blink = !blink;
    _nextTickBlink = now + BLINK_TIME;
  }
}

/* Blink the time. */
void UI::displayBlinkTime()
{
  static bool blink = true;
  unsigned long now = millis();

  /* Is it time to blink? */
  if (now >= _nextTickBlink)
  {
    if (blink)
    {
      _display.clearTime();
    }
    else
    {
      _display.printTime(getTime());
    }

    blink = !blink;
    _nextTickBlink = now + BLINK_TIME;
  }
}

/* Blink the target temperature. */
void UI::displayBlinkTemp()
{
  static bool blink = true;
  unsigned long now = millis();

  /* Is it time to blink? */
  if (now >= _nextTickBlink)
  {
    if (blink)
    {
      _display.clearTargetTemp();
    }
    else
    {
      _display.printTargetTemp(getTargetTemp());
    }

    blink = !blink;
    _nextTickBlink = now + BLINK_TIME;
  }
}

/* Blink ":" in the time. */
void UI::displayBlinkIndicator()
{
  static bool blink = true;
  unsigned long now = millis();

  /* Is it time to blink? */
  if (now >= _nextTickBlink)
  {
    if (blink)
    {
      _display.clearIndicator();
    }
    else
    {
      _display.printIndicator();
    }

    blink = !blink;
    _nextTickBlink = now + BLINK_TIME;
  }
}

/* Display the probe temperature. */
void UI::displayProbeTemp()
{
  if (updateProbeTemp())
  {
    _display.printProbeTemp(_probeTemp);
  }
}

/* Display the timer. */
void UI::displayTimer()
{
  if (updateTimer())
  {
    _display.printTime(_time[_function][_step]);
  }
}

char *UI::getName()
{
  if (_numSteps > 1)
  {
    unsigned int i = (UI_NAME_LEN - 1);
    _nameDisplay[i++] = ' ';
    itoa(_step + 1, &_nameDisplay[i++], 10);
    _nameDisplay[i++] = '\0';
  }
  else
  {
    for (unsigned int i = UI_NAME_LEN; i <UI_NAME_DISP_LEN; i++)
    {
      _nameDisplay[i] = ' ';
    }
  }

  return _nameDisplay;
}

inline unsigned long UI::getTime()
{
  return _time[_function][_step];
}

inline double UI::getTargetTemp()
{
  return _targetTemp[_function][_step];
}

inline double UI::getProbeTemp()
{
  return _probeTemp;
}

bool UI::updateProbeTemp()
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

bool UI::updateTimer()
{
  unsigned long now = millis();
  bool updated = false;

  if (now >= _nextTickTimer)
  {
    /* Tick timer down. */
    if (_time[_function][_step] > 0)
    {
      _time[_function][_step]--;
      updated = true;
    }

    /* Set next tick. */
    _nextTickTimer = now + TIMER_TIME;
  }

  return updated;
}

bool UI::updateReminder()
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

bool UI::updateBeeper()
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
