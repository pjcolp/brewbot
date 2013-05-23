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

#include <inttypes.h>
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif
#include <Timer.h>
#include "pins.h"
#include "BrewTimer.h"

void BrewTimer::setup()
{
  /* Configure buzzer pin. */
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, HIGH);

  /* Set default values. */
  _time = 5;
}

unsigned long BrewTimer::getTime()
{
  return _time;
}

void BrewTimer::setTime(unsigned long time)
{
  _time = time;
}

void BrewTimer::begin()
{
  _tickEvent = _timer.every(BT_TICK, tick, this);
}

void BrewTimer::stop()
{
  _timer.stop(_tickEvent);
}

void BrewTimer::done()
{
  stop();

  _buzzEvent = _timer.oscillate(PIN_BUZZER, BT_BUZZ, LOW, 3);
  _reminderEvent = _timer.every(BT_REMINDER, reminder, this);
}

void BrewTimer::update()
{
  _timer.update();
}

void BrewTimer::tick(void *ptr)
{
  BrewTimer *bt = (BrewTimer *)ptr;

  if (bt->_time > 0)
  {
    bt->_time--;
  }
  else
  {
    bt->done();
  }
}

void BrewTimer::reminder(void *ptr)
{
  BrewTimer *bt = (BrewTimer *)ptr;

  bt->_buzzEvent = bt->_timer.oscillate(PIN_BUZZER, BT_BUZZ, HIGH, 1);
}

