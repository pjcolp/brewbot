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

#ifndef BREWBOT_H
#define BREWBOT_H

#include <LiquidCrystal.h>
#include <AnalogButtons.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h>

#include <DeviceManager.h>

#include <Device.h>
#include <BooleanDevice.h>
#include <OneWireTemperatureDevice.h>
#include <PidRelayDevice.h>
#include <ShiftRegisterDevice.h>
#include <ShiftBitDevice.h>

#include "constants.h"
#include "pins.h"

double getProbeRIMSTemp(void);
void setElementRIMS(bool value);
bool requestTemperatures(void);

class BrewBot
{
  public:
    BrewBot();

    void setup(void);
    bool requestTemperatures(void);

    DeviceAddress addrRIMS;
    DeviceAddress addrBK;

    OneWire oneWire;
    DallasTemperature sensors;

    OneWireTemperatureDevice devProbeRIMS;
    OneWireTemperatureDevice devProbeBK;

    BooleanDevice devIndicator;
    BooleanDevice devBeeper;

    PidRelayDevice devPIDRIMS;

    ShiftRegisterDevice devRelays;
    ShiftBitDevice devElementControl;
    ShiftBitDevice devElementRIMS;
    ShiftBitDevice devElementBK;
    ShiftBitDevice devPump;
    ShiftBitDevice devFan;

  private:
    unsigned long _nextTickSensor;
};

#endif
