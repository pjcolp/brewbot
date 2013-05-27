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

#include <LiquidCrystal.h>
#include <AnalogButtons.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Timer.h>
#include <DeviceBus.h>
#include <RingDeviceBus.h>
#include <DeviceManager.h>
#include <Device.h>
#include <BooleanDevice.h>
#include <HighBooleanDevice.h>
#include <OneWireTemperatureDevice.h>

#include "pins.h"
#include "BrewTimer.h"
#include "UI.h"
#include "BrewBot.h"

OneWire oneWire(PIN_ONE_WIRE);
DallasTemperature sensors(&oneWire);
unsigned long sensorNextTick;
const unsigned long sensorInterval = 1000;

Ring ringReq;
Ring ringRsp;
DeviceBus devBus = RingDeviceBus(&ringReq, &ringRsp);
DeviceBus uiBus = RingDeviceBus(&ringRsp, &ringReq);

BooleanDevice devIndicator = HighBooleanDevice(PIN_INDICATOR, false);
BooleanDevice devBuzzer = HighBooleanDevice(PIN_BUZZER, false);
OneWireTemperatureDevice devProbeRIMS(&sensors, addrRIMS);

UI ui = UI(&uiBus, &devIndicator, &devBuzzer, &devProbeRIMS);

/* Core setup function. */
void setup(void)
{
  /* Setup the debug LED. */
  pinMode(PIN_DEBUG_LED, OUTPUT);

  /* Start serial port. */
  Serial.begin(9600);
  Serial.println("BrewBot");

  DeviceManager::SetBus(&devBus);

  /* Setup temperature sensors. */
  sensors.begin();
  int numberOfDevices = sensors.getDeviceCount();
  for (int i = 0; i < numberOfDevices; i++)
  {
    DeviceAddress tempDeviceAddress;

    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i))
    {
      sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
    }
  }

  /* Setup devices. */
  devIndicator.Setup(0);
  devBuzzer.Setup(1);
  devProbeRIMS.Setup(2);

  /* Setup UI. */
  ui.setup();
}

void loop(void)
{
  if (sensorNextTick <= millis())
  {
    Serial.println("Requesting temperatures");
    sensorNextTick = millis() + sensorInterval;
    sensors.requestTemperatures();
  }

  DeviceManager::ProcessMessages();
  DeviceManager::TickAll();
  ui.loop();
}

