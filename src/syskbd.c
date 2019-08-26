/*
 * xrick/src/syskbd.c
 *
 * Copyright (C) 1998-2019 bigorno (bigorno@bigorno.net). All rights reserved.
 *
 * The use and distribution terms for this software are contained in the file
 * named README, which can be found in the root of this distribution. By
 * using this software in any fashion, you are agreeing to be bound by the
 * terms of this license.
 *
 * You must not remove this notice, or any other, from this software.
 */

#include <SDL.h>

#include "system.h"
//#include "switch.h"
/*
 * Using the SDL_SCANCODE_xxx keysyms, which map to a QWERTY keyboard.
 * We get them via SDL_KEYDOWN.
 * We do *not* use SDL_TEXTINPUT nor SDLK_ to get true key mappings, so
 * for instance left on an AZERTY keyboard will be 'w' instead of 'z'.
 */

U8 syskbd_up = SDLK_UP;
U8 syskbd_down = SDLK_DOWN;
U8 syskbd_left = SDLK_LEFT;
U8 syskbd_right = SDLK_RIGHT;
U8 syskbd_pause = SDLK_MINUS;
U8 syskbd_end = SDLK_PLUS;
U8 syskbd_xtra = SDLK_a;
U8 syskbd_fire = SDLK_b;

/* eof */


