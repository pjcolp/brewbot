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

#ifndef BUTTONS_H
#define BUTTONS_H

#include <AnalogButtons.h>
#include "pins.h"

#define NUM_KEYS    5

#define KEY_RIGHT   1
#define KEY_UP      2
#define KEY_DOWN    3
#define KEY_LEFT    4
#define KEY_SELECT  5
#define KEY_NONE    (-1)

#define KEY_DEBOUNCE  100
#define KEY_DURATION    1

#define KEY_RIGHT_LOW        0
#define KEY_RIGHT_HIGH       2
#define KEY_UP_LOW         144
#define KEY_UP_HIGH        148
#define KEY_DOWN_LOW       332
#define KEY_DOWN_HIGH      336
#define KEY_LEFT_LOW       505
#define KEY_LEFT_HIGH      509
#define KEY_SELECT_LOW     740
#define KEY_SELECT_HIGH    744

#define KEY_NONE_LOW      1000
#define KEY_NONE_HIGH     2000

class Buttons
{
  public:
    Buttons(void (*callback)(int, bool))
    : _analogButtons(AnalogButtons(PIN_BUTTONS, KEY_DEBOUNCE, callback)),
      _rightButton(Button(KEY_RIGHT, KEY_RIGHT_LOW, KEY_RIGHT_HIGH, KEY_DURATION)),
      _upButton(Button(KEY_UP, KEY_UP_LOW, KEY_UP_HIGH, KEY_DURATION)),
      _downButton(Button(KEY_DOWN, KEY_DOWN_LOW, KEY_DOWN_HIGH, KEY_DURATION)),
      _leftButton(Button(KEY_LEFT, KEY_LEFT_LOW, KEY_LEFT_HIGH, KEY_DURATION)),
      _selectButton(Button(KEY_SELECT, KEY_SELECT_LOW, KEY_SELECT_HIGH, KEY_DURATION))
    {
      _analogButtons.addButton(_rightButton);
      _analogButtons.addButton(_upButton);
      _analogButtons.addButton(_downButton);
      _analogButtons.addButton(_leftButton);
      _analogButtons.addButton(_selectButton);
    };

    Buttons(void (*callback)(void *, int, bool), void *cookie)
    : _analogButtons(AnalogButtons(PIN_BUTTONS, KEY_DEBOUNCE, callback, cookie)),
      _rightButton(Button(KEY_RIGHT, KEY_RIGHT_LOW, KEY_RIGHT_HIGH, KEY_DURATION)),
      _upButton(Button(KEY_UP, KEY_UP_LOW, KEY_UP_HIGH, KEY_DURATION)),
      _downButton(Button(KEY_DOWN, KEY_DOWN_LOW, KEY_DOWN_HIGH, KEY_DURATION)),
      _leftButton(Button(KEY_LEFT, KEY_LEFT_LOW, KEY_LEFT_HIGH, KEY_DURATION)),
      _selectButton(Button(KEY_SELECT, KEY_SELECT_LOW, KEY_SELECT_HIGH, KEY_DURATION))
    {
      _analogButtons.addButton(_rightButton);
      _analogButtons.addButton(_upButton);
      _analogButtons.addButton(_downButton);
      _analogButtons.addButton(_leftButton);
      _analogButtons.addButton(_selectButton);
    };


    ~Buttons(){};

    void setup(void);
    void update(void);

  private:
    AnalogButtons _analogButtons;
    Button _rightButton;
    Button _upButton;
    Button _downButton;
    Button _leftButton;
    Button _selectButton;
};

#endif

