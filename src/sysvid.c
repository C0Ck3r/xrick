/*
 * xrick/src/sysvid.c
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
  * The purpose of this file is to implement a set of functions so that the
  * 8bit, palettized frame buffer onto which the entire game is painted can
  * be displayed onto the computer's screen.
  *
  * The only dependency between this and the game is that here we know the
  * frame buffer is 8bit. We don't know its size.
  */



#include <stdlib.h> /* malloc */

#include <SDL.h>

#include "sysvid.h"
#include "sysarg.h"
#include "debug.h"
#include "fb.h"
#include "img.h"


#ifdef __MSVC__
#include <memory.h> /* memset */
#endif



#undef BPP8
#define BPP32

//#define SDL_FULLSCREEN SDL_WINDOW_FULLSCREEN
#define SDL_FULLSCREEN SDL_WINDOW_FULLSCREEN_DESKTOP



rect_t SCREENRECT = {0, 0, FB_WIDTH, FB_HEIGHT, NULL}; /* whole fb */

static U16 paln; /* palette size */
static SDL_Color pals[256], pald[256]; /* fixme: explain */
static U32* pixels;
static SDL_Window *screen;
static SDL_Renderer *renderer;
static SDL_Texture* texture;
static U32 videoFlags;
static U8 gammaSDL;
static U16 fb_width, fb_height;

static U8 zoom = 0; /* actual zoom level */
static U8 wmzoom = SYSVID_ZOOM; /* window mode zoom level */
static U8 mxzoom = SYSVID_ZOOM * 2; /* max zoom level */



#include "img_icon.e"



/*
 * sysvid_setPaletteFromImg
 *
 * sets the palette according to an image palette.
 */
void sysvid_setPaletteFromImg(img_t *img)
{
	U16 i; // FIXME is it ok to have 256 (not 255) colors?

	if ((paln = img->ncolors) == 0) return;

	for (i = 0; i < paln; ++i)
	{
		pals[i].r = img->colors[i].r;
		pals[i].g = img->colors[i].g;
		pals[i].b = img->colors[i].b;
	}

	sysvid_setDisplayPalette();

#ifdef BPP8
	//SDL_SetColors(screen, (SDL_Color *)&pald, 0, paln);
#endif
}



/*
 * sysvid_setPaletteFromRGB
 *
 * sets the palette according to RGB infos.
 */
void sysvid_setPaletteFromRGB(U8 *r, U8 *g, U8 *b, U16 n)
{
	U16 i;

	if ((paln = n) == 0) return;

	for (i = 0; i < paln; ++i)
	{
		pals[i].r = r[i];
		pals[i].g = g[i];
		pals[i].b = b[i];
	}

	sysvid_setDisplayPalette();

#ifdef BPP8
	//SDL_SetColors(screen, (SDL_Color *)&pald, 0, paln);
#endif
}



/*
 * sysvid_setDisplayPalette
 *
 * sets (again) the display palette, useful when visibility has changed.
 */
void sysvid_setDisplayPalette(void)
{
	U16 i;

	if (paln == 0) return;

	for (i = 0; i < paln; i++)
	{
		pald[i].r = pals[i].r * gammaSDL / 255;
		pald[i].g = pals[i].g * gammaSDL / 255;
		pald[i].b = pals[i].b * gammaSDL / 255;
	}
}



/*
 * sysvid_init
 *
 * initialize the video layer.
 */
void sysvid_init(U16 width, U16 height)
{
	SDL_Surface *s;
	U8 *mask, tpix;
	U32 len, i;

	fb_width = width;
	fb_height = height;

	IFDEBUG_VIDEO(sys_printf("xrick/video: start\n"););

	/* various WM stuff */
	SDL_ShowCursor(SDL_DISABLE);

	s = SDL_CreateRGBSurfaceFrom(IMG_ICON->pixels, IMG_ICON->w, IMG_ICON->h, 8, IMG_ICON->w, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);
	//SDL_SetColors(s, (SDL_Color *)IMG_ICON->colors, 0, IMG_ICON->ncolors);
	tpix = *(IMG_ICON->pixels);
IFDEBUG_VIDEO(
	sys_printf("xrick/video: icon is %dx%d\n", IMG_ICON->w, IMG_ICON->h);
	sys_printf("xrick/video: icon transp. color is #%d (%d,%d,%d)\n", tpix,
		IMG_ICON->colors[tpix].r,
		IMG_ICON->colors[tpix].g,
		IMG_ICON->colors[tpix].b);
);
	/*

	* old dirty stuff to implement transparency. SetColorKey does it
	* on Windows w/out problems. Linux? FIXME!

	len = IMG_ICON->w * IMG_ICON->h;
	mask = (U8 *)malloc(len/8);
	memset(mask, 0, len/8);
	for (i = 0; i < len; i++)
	if (IMG_ICON->pixels[i] != tpix) mask[i/8] |= (0x80 >> (i%8));
	*/
	/*
	* FIXME
	* Setting a mask produces strange results depending on the
	* Window Manager. On fvwm2 it is shifted to the right ...
	*/
	/*SDL_WM_SetIcon(s, mask);*/

	// fixme?
	//SDL_SetColorKey(s,
	//	SDL_SRCCOLORKEY,
	//	SDL_MapRGB(s->format,IMG_ICON->colors[tpix].r,IMG_ICON->colors[tpix].g,IMG_ICON->colors[tpix].b));

	// fixme - which window?!
	//SDL_WM_SetIcon(s, NULL);

	/* video modes and screen */
	videoFlags = 0;
#ifdef BPP8
	videoFlags |= SDL_HWPALETTE;
#endif
	//chkVideo();  /* check video modes */

	/* if a zoom was specified, use it -- but check it is ok */
	if (sysarg_args_zoom)
	{
		zoom = sysarg_args_zoom > 0 && sysarg_args_zoom <= mxzoom ? sysarg_args_zoom : mxzoom;
	}

	/* prepare for fullscreen, initialize zoom w/default values */
	if (sysarg_args_fullscreen)
	{
		videoFlags |= SDL_FULLSCREEN;
		zoom = 1;
	}
	else
	{
		zoom = wmzoom;
	}

	/* initialize screen surface */
//#ifdef BPP8
//	screen = SDL_SetVideoMode(fb_width * zoom, fb_height * zoom,
//		8, videoFlags);
//#endif
//#ifdef BPP32
//	screen = SDL_SetVideoMode(fb_width * zoom, fb_height * zoom,
//		32, videoFlags);
//#endif

	// create pixels
	// FIXME free pixels!
	pixels = (U32*)malloc(fb_width * fb_height * sizeof(U32));

	// create window/screen
	screen = SDL_CreateWindow("xrick", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, fb_width * zoom, fb_height * zoom, videoFlags);
	SDL_SetWindowIcon(screen, s);

	// create renderer
	renderer = SDL_CreateRenderer(screen, -1, 0);

	// needed for fullscreen-desktop mode?
	SDL_RenderSetLogicalSize(renderer, fb_width, fb_height);

	// clear
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	// scaling hint - before texture creation
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

	// fixme this is temp
	// not using rects for now but we could ...
	texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		fb_width, fb_height);

	SDL_UpdateTexture(texture, NULL, pixels, fb_width * sizeof(U32));
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

// http://www.linuxdevcenter.com/pub/a/linux/2003/08/07/sdl_anim.html
// http://www.linuxdevcenter.com/linux/2003/08/07/examples/hardlines.cpp

	// fixme what shall we do with this?
//IFDEBUG_VIDEO2(
//	sys_printf("xrick/video: mode: %dx%d %dbpp pitch=%d %s\n",
//		screen->w, screen->h, screen->format->BitsPerPixel, screen->pitch,
//		videoFlags & SDL_FULLSCREEN ? "fullscreen" : "");
//	sys_printf("xrick/video: HWSURFACE: %s\n", screen->flags & SDL_HWSURFACE ? "Y" : "N");
//	sys_printf("xrick/video: DOUBLEBUF: %s\n", screen->flags & SDL_DOUBLEBUF ? "Y" : "N");
//	/* FIXME also report the REAL HW infos i.e. the actual screen resolution */
//	/* i.e. if we ask for 996x600 SDL says OK but then it maps <screen> to the actual screen */
//);

	IFDEBUG_VIDEO(sys_printf("xrick/video: ready\n"););
}



/*
 * sysvid_shutdown
 *
 * shutdown the video layer.
 */
void
sysvid_shutdown(void)
{
	free(pixels);
	pixels = NULL;

	SDL_DestroyWindow(screen);
}




/*
 * sysvid_update
 *
 * display the 8bit palettized frame buffer onto the screen. zoom, filter, whatever.
 */
void
sysvid_update(rect_t *rects)
{
	SDL_Rect *sdlrects;
	rect_t *rect;
	U16 x, y, xx, yy;
	U8 *src, *dst, *src0, *dst0;
	U8 n;

	if (rects == NULL) /* nothing to do? */
		return;

	int pitch;
	U32* pixelx;

	SDL_LockTexture(texture, NULL, &pixelx, &pitch);

	n = 0;
	rect = rects;
	while (rect)
	{
		U16 o = rect->x + rect->y * fb_width;
		U8* src0 = ((U8*)& fb) + o;
		U8* dst0 = pixelx + o;
		for (int y = rect->y; y < rect->y + rect->height; y++)
		{
			U8* srcx = src0;
			U8* dstx = dst0;

			for (int x = rect->x; x < rect->x + rect->width; x++)
			{
				*dstx = pald[*srcx].b;
				dstx++;
				*dstx = pald[*srcx].g;
				dstx++;
				*dstx = pald[*srcx].r;
				dstx++;
				*dstx = pald[*srcx].a;
				dstx++;
				srcx++;
			}

			src0 += fb_width;
			dst0 += fb_width * 4;
		}
		rect = rect->next;
		n++;
	}

	SDL_UnlockTexture(texture);

	// rects?
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}



/*
 * sysvid_zoom
 *
 * increases or decreases zoom by <z>, if possible.
 */
void
sysvid_zoom(S8 z)
{
	// FIXME fullscreen

	if ((z < 0 && zoom + z > 0) || (z > 0 && zoom + z <= mxzoom))
	{
		zoom += z;
		wmzoom = zoom;

		IFDEBUG_VIDEO(
			sys_printf("xrick/video: zoom=%d window=%dx%d\n", zoom, fb_width * zoom, fb_height * zoom);
		);

		SDL_SetWindowSize(screen, fb_width * zoom, fb_height * zoom);

		sysvid_setDisplayPalette();
		sysvid_update(&SCREENRECT); /* repaint all */ /* FIXME */

		// fixme what shall we do with this?
//IFDEBUG_VIDEO2(
//	sys_printf("xrick/video: mode: %dx%d %dbpp pitch=%d %s\n",
//		screen->w, screen->h, screen->format->BitsPerPixel, screen->pitch,
//		videoFlags & SDL_FULLSCREEN ? "fullscreen" : "");
//);
	}
}



/*
 * sysvid_toggleFullscreen
 *
 * toggles fullscreen.
 */
void
sysvid_toggleFullscreen(void)
{
	videoFlags ^= SDL_FULLSCREEN;
	SDL_SetWindowFullscreen(screen, videoFlags & SDL_FULLSCREEN ? SDL_FULLSCREEN : 0);

	zoom = videoFlags & SDL_FULLSCREEN ? 1 : wmzoom;

	sysvid_setDisplayPalette();
	sysvid_update(&SCREENRECT); /* repaint all */ /* FIXME */

	// fixme what shall we do with this
//IFDEBUG_VIDEO2(
//	sys_printf("xrick/video: mode: %dx%d %dbpp pitch=%d %s\n",
//		screen->w, screen->h, screen->format->BitsPerPixel, screen->pitch,
//		videoFlags & SDL_FULLSCREEN ? "fullscreen" : "");
//);

}



/*
 * sysvid_setGamma
 *
 * sets a "gamma" indication ranging from 0 (dark) to 255 (normal).
 */
void sysvid_setGammaSDL(U8 g)
{
	// FIXME changing the GAMMA without changing the PALETTE just CANNOT WORK if GAMMA is not HARDWARE?
	gammaSDL = g;
	sysvid_setDisplayPalette();

#ifdef BPP8
	//SDL_SetColors(screen, (SDL_Color*)& pald, 0, paln);
#endif
}



/* eof */



