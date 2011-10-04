/*
This file is part of "TW-Light"
					http://tw-light.appspot.com/
Copyright (C) 2001-2004  TimeWarp development team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

//#define NO_WINMM
//#define NO_WINMM_TGT
//#define NO_WINMM_QPC
//#define NO_RDTSC
//#define NO_LIBC
//#define NO_ALLEGRO_TIME
//#define NO_SDL_TIME

#define PLATFORM_IS_ALLEGRO
//#define PLATFORM_IS_SDL

#ifndef _DEBUG
//# define NO_RDTSC
#endif

#ifdef NO_ALLEGRO_TIME
#   undef PLATFORM_IS_ALLEGRO
#endif
#ifdef NO_SDL_TIME
#   undef PLATFORM_IS_SDL
#endif

/*
------------------------------
		Timers & Stuff
------------------------------

Three sections:
1.  idle() functions for Allegro / SDL / unrecognized platform
2.  Platform-specific time functions:
		LibC time
		Allegro interrupts (stupid allegro...)
		SDL ticks
Windows Multi-Media timeGetTime
Windows Multi-Media Performance Counter
Intel RDTSC
3.  Wrappers to pick which time functions to use
*/

#ifndef NO_LIBC
#   include <time.h>
#   include <math.h>
#endif
#ifdef PLATFORM_IS_SDL
#   include <SDL.h>
#else
#   include "types.h"
#endif
#include "get_time.h"
#include "base.h"
#include "errors.h"

//1.  idle() functions for Allegro / SDL / unknown platform

int _no_idle = 0;

#if defined(PLATFORM_IS_ALLEGRO) && !defined(NO_ALLEGRO_TIME)
#   include <allegro.h>
#   if defined WIN32
#       include <winalleg.h>
#   endif
int idle ( int time )
{
	if (_no_idle) return 0;
	rest(time);
	return time;
}


#elif defined(PLATFORM_IS_SDL) && !defined(NO_SDL_TIME)
#   include <SDL.h>
int idle ( int time )
{
	if (_no_idle) return 0;
	SDL_Delay ( time );
	return time;
}


#else
int idle ( int time )
{
	if (_no_idle) return 0;
	return 0;
	/*int i = get_time() + time;
	while (i > get_time()) {
		//do nothing real fast
	}
	return time;*/
}
#endif

//2.  Platform specific time functions:

//RDTSC:
//RDTSC is only valid on pentiums & later, right?
//so how do we detect this at run-time? (without crashing)
#if !defined NO_RDTSC
#   if defined _MSC_VER
#       define RDTSC_TIMER
static double rdtsc_period_f = 0;
static int rdtsc_period_i = 0;
static Uint64 rdtsc_base;
static volatile Uint64 rdtsc_get_time()
{
	unsigned int a, b;
	__asm RDTSC
		__asm mov a, eax
		__asm mov b, edx
		return (a + ((Sint64)b << 32)) - rdtsc_base;
}


#   elif defined(__GNUC__) && defined(__i386__)
//#     define RDTSC_TIMER
//need to double-check the inline asm here
//static double rdtsc_period_f = 0;
//static int rdtsc_period_i = 0;
static Uint64 rdtsc_base;
__volatile__ Uint64 rdtsc_get_time()
{
	unsigned int a, b;
	asm ("RDTSC" : "=a" (a), "=d" (b) : );
	//__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return (a + ((Sint64)b << 32)) - rdtsc_base;
}
#   endif
#endif

#if defined PLATFORM_IS_ALLEGRO
#   include <allegro.h>
#   define allegro_period 5
static volatile int allegro_time = 0;
static int allegro_base = 0;
static volatile int allegro_get_time()
{
	return allegro_time - allegro_base;
}


END_OF_STATIC_FUNCTION(allegro_get_time);
static void global_timer(void)
{
	allegro_time += allegro_period;
}


END_OF_STATIC_FUNCTION(global_timer);
void init_allegro_time()
{
	LOCK_FUNCTION(global_timer);
	LOCK_FUNCTION(allegro_get_time);
	LOCK_VARIABLE(allegro_time);
	if (install_timer() < 0) tw_error("Allegro timer installation failed");
	install_int(&global_timer, allegro_period);
}


void deinit_allegro_time()
{
	remove_int(&global_timer);
}


#elif defined PLATFORM_IS_SDL
#   include <SDL.h>
static int sdl_base;
static int sdl_get_time()
{
	return SDL_GetTicks() - sdl_base;
}


static void init_sdl_time()
{
	SDL_InitSubSystem(SDL_INIT_TIMER);
}


static void deinit_sdl_time()
{
	SDL_QuitSubSystem(SDL_INIT_TIMER);
}


#else
//# error unknown platform
#endif

#if !defined NO_LIBC
#   define LIBC_TIME
int libc_base = 0;
int libc_get_time()
{
	return (clock() * 1000) / CLOCKS_PER_SEC - libc_base;
}
#endif

//3.  The exported wrappers for time functions

int get_time()
{
	#if 0
	#elif defined WINMM_TGT
	return winmm_tgt_get_time();
	#elif defined PLATFORM_IS_SDL
	return sdl_get_time();
	#elif defined PLATFORM_IS_ALLEGRO
	return allegro_get_time();
	#elif defined LIBC_TIME
	return libc_get_time();
	#else
	#   error No integer time function specified!
	#endif
}


/*
#ifdef USE_ALLEGRO
	END_OF_FUNCTION(get_time);
#endif
*/

double get_time2()
{
	#   if 0
	#   elif defined WINMM_QPC
	return (Sint64)winmm_qpc_get_time() * winmm_qpc_period_f;
	#   elif defined RDTSC_TIMER
	//require debuging mode, since we don't have run-time check
	return (Sint64)rdtsc_get_time() * rdtsc_period_f;
	#   else
	return get_time();
	#   endif
}


Sint64 get_time3()
{
	#   if 0
	#   elif defined RDTSC_TIMER
	//ought to require debuging mode, since we don't have run-time check that RDTSC is a valid instruction
	return rdtsc_get_time();
	#   elif defined WINMM_QPC
	return winmm_qpc_get_time();
	#   else
	return get_time();
	#   endif
}


static unsigned char timer_attributes = 0;

int is_time_initialized()
{
	return (timer_attributes & 1);
}


void init_time()
{
	int ms;
	if (timer_attributes & 1) return;
	timer_attributes |= 1;
	#   if defined WINMM_TGT
	{
		int tmp = winmm_tgt_get_time();
		int i;
		for (i = 0; i < 1000000; i += 1)
			if (winmm_tgt_get_time() != tmp) break;
		winmm_tgt_base = winmm_tgt_get_time() - 1;
	}
	#   endif
	#   if defined PLATFORM_IS_SDL
	init_sdl_time();
	sdl_base = sdl_get_time() - 1;
	#   endif
	#   if defined PLATFORM_IS_ALLEGRO
	LOCK_FUNCTION(get_time);
	init_allegro_time();
	allegro_base = allegro_get_time() - 1;
	#   endif
	#   if defined LIBC_TIME
	libc_base = libc_get_time() - 1;
	#   endif

	ms = get_time();
	//finished initializing get_time()
	//now we do get_time2() / get_time3()

	#   if defined RDTSC_TIMER
	//require debuging mode, since we don't have a run-time check
	rdtsc_base = rdtsc_get_time();
	#   endif
	#   if defined WINMM_QPC
	winmm_qpc_base = winmm_qpc_get_time();
	winmm_qpc_period_f = 1000.0 / winmm_qpc_get_freq();
	#   endif

	#   if defined RDTSC_TIMER
	{
		Sint64 tmpl;
		double tmpd;
		while (get_time() - ms < 100) ;
		tmpl = rdtsc_get_time();
		tmpd = tmpl * (1.0 / (get_time() - ms));
		#       if !defined NO_LIBC
		{
			double tmpd2, tmpd3;
			//most clock speeds are multiples of nice numbers
			//like 25.0 or 33.333 Mega-Hertz.
			tmpd2 = floor(0.5 + tmpd / (100000/4.0)) * (100000/4.0);
			tmpd3 = floor(0.5 + tmpd / (100000/3.0)) * (100000/3.0);
			if (fabs(tmpd2-tmpd) < fabs(tmpd3-tmpd)) tmpd = tmpd2;
			else tmpd = tmpd3;
		}
		#       endif
		rdtsc_period_f = 1 / tmpd;
	}
	#   endif
}


//not tested, not even a little bit...
void deinit_time()
{
	if (!(timer_attributes & 1)) return;
	timer_attributes &=~1;
	#   if defined PLATFORM_IS_SDL
	deinit_sdl_time();
	#   endif
	#   if defined PLATFORM_IS_ALLEGRO
	deinit_allegro_time();
	#   endif
	return;
}
