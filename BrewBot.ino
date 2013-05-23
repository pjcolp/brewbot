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
#include <Timer.h>

#include "pins.h"
#include "BrewTimer.h"
#include "UI.h"

UI ui;

Device devBuzzer = BooleanDevice(PIN_BUZZER, PIN_BUZZER, true, false);

/* Core setup function. */
void setup(void)
{
  /* Setup the debug LED. */
  pinMode(PIN_DEBUG_LED, OUTPUT);

  /* Start serial port. */
  Serial.begin(9600);
  Serial.println("BrewBot");

  /* Setup UI. */
  ui.setup();
}

void loop(void)
{
  ui.loop();
}

