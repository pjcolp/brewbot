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
#include "UI.h"
#include "BrewBot.h"

BrewBot::BrewBot()
: oneWire(PIN_ONE_WIRE), sensors(&oneWire),
  addrRIMS({ 0x28, 0xB5, 0x7E, 0x57, 0x04, 0x00, 0x00, 0xFD }),
  addrBK({ 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }),
  devIndicator(PIN_INDICATOR, false, true), devBeeper(PIN_BEEPER, false, true),
  devProbeRIMS(&sensors, addrRIMS), devProbeBK(&sensors, addrBK),
  devPIDRIMS(getProbeRIMSTemp, setElementRIMS, 1.0, 1.0, 1.0),
  devRelays(PIN_RELAY_CLOCK, PIN_RELAY_LATCH, PIN_RELAY_DATA, 0),
  devElementControl(&devRelays, 1, false),
  devElementRIMS(&devRelays, 2, false), devElementBK(&devRelays, 3, false),
  devPump(&devRelays, 4, false), devFan(&devRelays, 5, false)
{
}

void BrewBot::setup()
{
  /* Setup temperature sensors. */
  sensors.begin();
  for (uint8_t i = 0; i < sensors.getDeviceCount(); i++)
  {
    DeviceAddress tempDeviceAddress;

    /* Search the wire for address. */
    if (sensors.getAddress(tempDeviceAddress, i))
    {
      sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
    }
  }

  /* Setup devices. */
  unsigned devID = 0;
  devIndicator.Setup(devID++);
  devBeeper.Setup(devID++);
  devProbeRIMS.Setup(devID++);
  devRelays.Setup(devID++);
  devPIDRIMS.Setup(devID++);
  devElementControl.Setup(devID++);
  devElementRIMS.Setup(devID++);
  devElementBK.Setup(devID++);
  devPump.Setup(devID++);
  devFan.Setup(devID++);

  /* XXX: Setup PID. */
  devPIDRIMS.Write(512.00);
  devPIDRIMS.enable(true);
}

bool BrewBot::requestTemperatures()
{
  unsigned long now = millis();
  bool updated = false;

  if (now >= _nextTickSensor)
  {
    sensors.requestTemperatures();
    _nextTickSensor = now + SENSOR_TIME;

    updated = true;
  }

  return updated;
}

/* Other stuff */
BrewBot brewBot = BrewBot();
UI ui = UI(&brewBot);

/* Core setup function. */
void setup(void)
{
  /* Setup the debug LED. */
  pinMode(PIN_DEBUG_LED, OUTPUT);

  /* Start serial port. */
  Serial.begin(9600);
  Serial.println("BrewBot");

  /* Setup BrewBot. */
  brewBot.setup();

  /* Setup UI. */
  ui.setup();
}

void loop(void)
{
#if 0
  DeviceManager::ProcessMessages();
#endif

  brewBot.requestTemperatures();
  DeviceManager::TickAll();

#if 0
  DeviceManager::ReportStatusUpdates();
#endif

  /* Now let the UI have a turn to run. */
  ui.loop();
}


bool rims_old_value = false;

double getProbeRIMSTemp()
{
  const double PID_MAX = 1024.00;
  static unsigned long nextTick = 0;
  static double output = 0;
  static double rimsTemp = 45;
  
  if (millis() >= nextTick)
  {
#if 0
    /* Read in temperature. */
    double rimsTemp = devProbeRims.Read();
#else
    if (rims_old_value)
    {
      rimsTemp += 0.5;
    }
    else
    {
      rimsTemp -= 0.5;
    }
#endif

    /* Adjust temperature to be in the range of the PID. */
#if 1
    double normalised = rimsTemp / (MASH_TEMP_MAX - MASH_TEMP_MIN);
#else
    double normalised = rimsTemp / 120;
#endif

    output = normalised * PID_MAX;

    nextTick = millis() + 1000;

    Serial.print("RIMS temp: ");
    Serial.println(rimsTemp);
  }

  return output;
}

void setElementRIMS(bool value)
{
#if 0
  devElementRIMS->Write(value);
#endif

  if (rims_old_value != value)
  {
    if (value)
    {
      Serial.println("RIMS on");
    }
    else
    {
      Serial.println("RIMS off");
    }

#if 0
    devElementRIMS.Write(value);
#endif

    rims_old_value = !rims_old_value;
  }
}
