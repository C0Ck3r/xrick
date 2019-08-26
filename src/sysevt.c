/*
 * xrick/src/sysevt.c
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

/*
 * 20021010 SDL_SCANCODE_n replaced by SDL_SCANCODE_Fn because some non-US keyboards
 *          requires that SHIFT be pressed to input numbers.
 */

#include <SDL.h>

#include "system.h"
#include "syskbd.h"
#include "sysvid.h"
#include "game.h"
#include "debug.h"

#include "control.h"
#include "draw.h"

#define SYSJOY_RANGE 3280

#define SETBIT(x,b) x |= (b)
#define CLRBIT(x,b) x &= ~(b)

static SDL_Event event;

/*
 * Process an event
 */
static void
processEvent()
{
	U16 key;
#ifdef ENABLE_FOCUS
	SDL_ActiveEvent *aevent;
#endif

	switch (event.type) {
	case SDL_KEYDOWN:
		key = event.key.keysym.scancode;
		//key = event.key.keysym.sym;
		if (key == syskbd_up || key == SDL_SCANCODE_UP) {
			SETBIT(control_status, CONTROL_UP);
			control_last = CONTROL_UP;
		}
		else if (key == syskbd_down || key == SDL_SCANCODE_DOWN) {
			SETBIT(control_status, CONTROL_DOWN);
			control_last = CONTROL_DOWN;
		}
		else if (key == syskbd_left || key == SDL_SCANCODE_LEFT) {
			SETBIT(control_status, CONTROL_LEFT);
			control_last = CONTROL_LEFT;
		}
		else if (key == syskbd_right || key == SDL_SCANCODE_RIGHT) {
			SETBIT(control_status, CONTROL_RIGHT);
			control_last = CONTROL_RIGHT;
		}
		else if (key == syskbd_pause) {
			SETBIT(control_status, CONTROL_PAUSE);
			control_last = CONTROL_PAUSE;
		}
		else if (key == syskbd_end) {
			SETBIT(control_status, CONTROL_END);
			control_last = CONTROL_END;
		}
		else if (key == syskbd_xtra) {
			SETBIT(control_status, CONTROL_EXIT);
			control_last = CONTROL_EXIT;
		}
		else if (key == syskbd_fire) {
			SETBIT(control_status, CONTROL_FIRE);
			control_last = CONTROL_FIRE;
		}
		else if (key == SDL_SCANCODE_F1) {
			sysvid_toggleFullscreen();
		}
		else if (key == SDL_SCANCODE_F2) {
			sysvid_zoom(-1);
		}
		else if (key == SDL_SCANCODE_F3) {
			sysvid_zoom(+1);
		}
#ifdef ENABLE_SOUND
		else if (key == SDL_SCANCODE_F4) {
			syssnd_toggleMute();
		}
		else if (key == SDL_SCANCODE_F5) {
			syssnd_vol(-1);
		}
		else if (key == SDL_SCANCODE_F6) {
			syssnd_vol(+1);
		}
#endif
		else if (key == SDL_SCANCODE_F7) {
			game_toggleCheat(1);
		}
		else if (key == SDL_SCANCODE_F8) {
			game_toggleCheat(2);
		}
		else if (key == SDL_SCANCODE_F9) {
			game_toggleCheat(3);
		}
		break;
	case SDL_KEYUP:
		key = event.key.keysym.scancode;
		//key = event.key.keysym.sym;
		if (key == syskbd_up || key == SDL_SCANCODE_UP) {
      CLRBIT(control_status, CONTROL_UP);
      control_last = CONTROL_UP;
    }
    else if (key == syskbd_down || key == SDL_SCANCODE_DOWN) {
      CLRBIT(control_status, CONTROL_DOWN);
      control_last = CONTROL_DOWN;
    }
    else if (key == syskbd_left || key == SDL_SCANCODE_LEFT) {
      CLRBIT(control_status, CONTROL_LEFT);
      control_last = CONTROL_LEFT;
    }
    else if (key == syskbd_right || key == SDL_SCANCODE_RIGHT) {
      CLRBIT(control_status, CONTROL_RIGHT);
      control_last = CONTROL_RIGHT;
    }
    else if (key == syskbd_pause) {
      CLRBIT(control_status, CONTROL_PAUSE);
      control_last = CONTROL_PAUSE;
    }
    else if (key == syskbd_end) {
      CLRBIT(control_status, CONTROL_END);
      control_last = CONTROL_END;
    }
    else if (key == syskbd_xtra) {
      CLRBIT(control_status, CONTROL_EXIT);
      control_last = CONTROL_EXIT;
    }
    else if (key == syskbd_fire) {
      CLRBIT(control_status, CONTROL_FIRE);
      control_last = CONTROL_FIRE;
    }
    break;
  case SDL_QUIT:
    /* player tries to close the window -- this is the same as pressing ESC */
    SETBIT(control_status, CONTROL_EXIT);
    control_last = CONTROL_EXIT;
    break;
#ifdef ENABLE_FOCUS
  case SDL_ACTIVEEVENT: {
    aevent = (SDL_ActiveEvent *)&event;
    IFDEBUG_EVENTS(
      sys_printf("xrick/events: active %x %x\n", aevent->gain, aevent->state);
      );
    if (aevent->gain == 1)
      control_active = TRUE;
    else
      control_active = FALSE;
    }
  break;
#endif
#ifdef ENABLE_JOYSTICK

  case SDL_JOYBUTTONDOWN:
 	if (event.jbutton.button == 1) {  /* B Button */
    		SETBIT(control_status, CONTROL_FIRE);
    		break;
	}
	else if (event.jbutton.button == 14) {  /* DPad Right */
    		SETBIT(control_status, CONTROL_RIGHT);
    		break;
	}
	else if (event.jbutton.button == 15) {  /* DPad Down*/
    		SETBIT(control_status, CONTROL_DOWN);
    		break;
	}
	else if (event.jbutton.button == 12) {  /* DPad Left*/
    		SETBIT(control_status, CONTROL_LEFT);
    		break;
	}
	else if (event.jbutton.button == 13) {  /* DPad Up*/
    		SETBIT(control_status, CONTROL_UP);
    		break;
	}
		else if (event.jbutton.button == 10) {  /* Plus*/
    		SETBIT(control_status, CONTROL_PAUSE);
    		break;
	}
	else if (event.jbutton.button == 11) {  /* Minus*/
    		SETBIT(control_status, CONTROL_EXIT);
    		break;
	}
  case SDL_JOYBUTTONUP:
 	if (event.jbutton.button == 1 ) {  /* A Button */
    		CLRBIT(control_status, CONTROL_FIRE);
    		break;
	}
	else if (event.jbutton.button == 14) {  /* DPad Right */
    		CLRBIT(control_status, CONTROL_RIGHT);
    		break;
	}
	else if (event.jbutton.button == 15) {  /* DPad Down*/
    		CLRBIT(control_status, CONTROL_DOWN);
    		break;
	}
	else if (event.jbutton.button == 12) {  /* DPad Left*/
    		CLRBIT(control_status, CONTROL_LEFT);
    		break;
	}
	else if (event.jbutton.button == 13) {  /* DPad Up*/
    		CLRBIT(control_status, CONTROL_UP);
    		break;
	}
	else if (event.jbutton.button == 10) {  /* Plus*/
    		CLRBIT(control_status, CONTROL_PAUSE);
    		break;
	}
	else if (event.jbutton.button == 11) {  /* Minus*/
    		CLRBIT(control_status, CONTROL_EXIT);
    		break;
	}
#endif
  default:
    break;
  }
}

/*
 * Process events, if any, then return
 */
void
sysevt_poll(void)
{
  while (SDL_PollEvent(&event))
    processEvent();
}

/*
 * Wait for an event, then process it and return
 */
void
sysevt_wait(void)
{
  // SDL_WaitEvent locks emscripten
  // this is only for pause really

#ifdef EMSCRIPTEN
  if (SDL_PollEvent(&event))
#else
  SDL_WaitEvent(&event);
#endif
  processEvent();
}

/* eof */



