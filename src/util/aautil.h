/*
 * aautil.h --- helpers for anti-aliasing routines for Allegro
 *
 * This file is gift-ware.  This file is given to you freely
 * as a gift.  You may use, modify, redistribute, and generally hack
 * it about in any way you like, and you do not have to give anyone
 * anything in return.
 *
 * I do not accept any responsibility for any effects, adverse or
 * otherwise, that this code may have on just about anything that
 * you can think of.  Use it at your own risk.
 *
 * Copyright (C) 1998, 1999  Michael Bukin
 */

#ifndef __bma_aautil_h
#define __bma_aautil_h

#include <allegro.h>

typedef void PUT_TYPE(unsigned long _addr, int _x);
typedef void ADD_TYPE(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2);

#if ((aa_BITS < 0) || (aa_BITS > 12))
#error aa_BITS must be (0 <= aa_BITS <= 12)
#endif

#define aa_SIZE     (1UL << aa_BITS)
#define aa_MASK     (aa_SIZE - 1)

#define aa_MAX_SIZE (1UL << 12)
#define aa_MAX_NUM  (aa_MAX_SIZE * aa_MAX_SIZE)

#ifdef __cplusplus
extern "C"
{
	#endif

	struct RGBA
	{
		#if defined ALLEGRO_LITTLE_ENDIAN
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
		#elif defined ALLEGRO_BIG_ENDIAN
		unsigned char a;
		unsigned char b;
		unsigned char g;
		unsigned char r;
		#else
		#   error No endianness detected
		#endif
	};

	/// \brief Anti-aliasing settings
	struct _aa_type
	{
		unsigned long r;		 //red
		unsigned long g;		 //green
		unsigned long b;		 //blue
		int mask_color;			 //the current input mask color
		int transparent;		 // 2-level transperancy (0 = solid, other = transparent)
		unsigned long total;	 // total # of pixels
		unsigned long inverse;	 // total # of pixels
		unsigned long trans;	 // # of transperant pixels (normalized to parts / 255 before the _aa_put call)
		union
		{
			struct RGBA rgba;
			struct RGB rgb;
			int i;
		} current, background;
		int roffset24;			 // 24 bpp mode offsets
		int goffset24;
		int boffset24;
		char r_left;			 // dithering state
		char g_left;
		char b_left;
		int y;
		int mode;
		struct BITMAP *destination;
		long blender_n;
	};
	extern struct _aa_type _aa;

	#define AA_PUT_PIXEL(put_func, address, x)  if (!_aa.transparent) put_func(address,x);

	ADD_TYPE *get_aa_add_function(BITMAP *source, int mode);
	PUT_TYPE *get_aa_put_function(BITMAP *destination, int mode);

	void aa_put_dummy(unsigned long _addr, int _x);
	void aa_add_dummy(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2);

	//these #defines determine how 24 bpp bitmaps are read
	//misaligned ones are illegal on some (non-x86) platforms
	//oversized ones are illegal under some (?) circumstances
	//only 1 of these #defines should be uncommented
	#define _READ24_AS_MISALIGNED_OVERSIZE
	//#define _READ24_AS_CHARS
	//#define _READ24_MISALIGNED_MIX

	#ifdef __cplusplus
}
#endif

#define NON_MASK_COLOR_15    (MASK_COLOR_15 - 1)
#define NON_MASK_COLOR_16    (MASK_COLOR_16 - 1)
#define NON_MASK_COLOR_24    (MASK_COLOR_24 - 1)
#define NON_MASK_COLOR_32    (MASK_COLOR_32 - 1)

#define MASK_COLOR_32a       MASK_COLOR_32
#define NON_MASK_COLOR_32a   NON_MASK_COLOR_32
#define MASK_COLOR_16a       MASK_COLOR_16
#define NON_MASK_COLOR_16a   0

//#define makeacol16a(r,g,b,a) ( (r>>4)+(g&0xf0)+((b>>4)<<8)+((a>>4)<<12) )
#define makeacol16a(r,g,b,a) ( ((g+(a<<8))&0xf0f0) + (r>>4) + ((b>>4)<<8) )
#define makeacol12(r,g,b) makecol16a(r,g,b,0)
#define getr12(c) ((c<<4)&0xff)
#define getg12(c) (c&0xf0)
#define getb12(c) ((c>>4)&0xf0)
#define geta12(c) 0
#define getr16a(c) getr12(c)
#define getg16a(c) getg12(c)
#define getb16a(c) getb12(c)
#define geta16a(c) ((c>>8)&0xf0)
#define geta8(c) 0
#define geta15(c) 0
#define geta16(c) 0
#define geta24(c) 0
#define getr32a(c) getr32(c)
#define getg32a(c) getg32(c)
#define getb32a(c) getb32(c)
#define geta32a(c) geta32(c)

/* Prepare Bresenham line parameters.  */
#define aa_PREPARE(inc,dd,i1,i2,_yw,_xw) \
	{ \
		int xw = (_xw); \
		int yw = (_yw); \
		if ((xw == 0) || ((yw < xw) && (yw > -xw))) \
		{ \
			(inc) = 0; \
		} \
		else \
		{ \
			(inc) = yw / xw; \
			yw %= xw; \
		} \
		if (yw < 0) \
		{ \
			(inc) -= 1; \
			yw += xw; \
		} \
		(i2) = ((dd) = ((i1) = 2 * yw) - xw) - xw; \
	}

		/* Advance to the next point.  */
#define aa_ADVANCE(y,inc,dd,i1,i2) \
	{ \
		if ((dd) >= 0) \
		(y) += (inc) + 1, (dd) += (i2); \
		else \
		(y) += (inc), (dd) += (i1); \
	}
#endif							 /* !__bma_aautil_h */

			/*
			 * aautil.h ends here
			 */
