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

#ifndef BREWTIMER_H
#define BREWTIMER_H

#include <Timer.h>
#include <BooleanDevice.h>

#define BT_TICK      (1000)
#define BT_BUZZ      (500)
#define BT_REMINDER  (1000*10) // 10 seconds

class BrewTimer
{
  public:
    BrewTimer(BooleanDevice *devBuzzer) : _devBuzzer(devBuzzer) {};
    ~BrewTimer(){};

    void setup(void);
    void update(void);

    void begin(void);
    void stop(void);
    void done(void);

    unsigned long getTime();
    void setTime(unsigned long time);

  private:
    static void tick(void *ptr);
    static void reminder(void *ptr);

    Timer _timer;
    unsigned long _time;
    bool _indicator;

    BooleanDevice *_devBuzzer;

    int8_t _tickEvent;
    int8_t _buzzEvent;
    int8_t _reminderEvent;
};

#endif

