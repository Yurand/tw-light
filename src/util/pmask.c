#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pmask.h"

#ifdef USE_ALLEGRO
#   include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif
#endif

#ifdef main
#   undef main
#endif

#ifdef USE_SDL
#   include <SDL_video.h>
#   include <SDL_endian.h>
#endif

//#include "base.h"
#include "round.h"

#define MAX_INTVAL(int_type) ((((unsigned int_type)(-1))-1)/2)

int get_pmask_pixel(CONST struct PMASK *mask, int x, int y)
{
	return 1 & (mask->mask[(mask->h * (x >> MASK_WORD_BITBITS)) + y] >> (x & (MASK_WORD_BITS-1)));
}


void set_pmask_pixel(struct PMASK *mask, int x, int y, int value)
{
	if (value) {
		mask->mask[(mask->h * (x >> MASK_WORD_BITBITS)) + y] |= 1 << (x & (MASK_WORD_BITS-1));
	} else {
		mask->mask[(mask->h * (x >> MASK_WORD_BITBITS)) + y] &=~(1 << (x & (MASK_WORD_BITS-1)));
	}
}


#define COMPILE_TIME_ASSERT(condition) typedef char _compile_time_assert__[(condition) ? 1 : -1];
void install_pmask()
{
	COMPILE_TIME_ASSERT((1 << MASK_WORD_BITBITS) == MASK_WORD_BITS);
	return;
}


void init_pmask (struct PMASK *mask, int w, int h)
{
	int words, total_words, x;

	if ((w > MAX_INTVAL(short int)) || (h > MAX_INTVAL(short int)) || (w < 0) || (h < 0)) {
		mask->w = mask->h = 0;
		#ifndef MASK_SINGLE_MEMORY_BLOCK
		mask->mask = NULL;
		#endif
		return;
	}

	words = 1 + ((w-1) >> MASK_WORD_BITBITS);

	total_words = words * h;

	#ifdef MASK_SINGLE_MEMORY_BLOCK

	#else
	mask->mask = (MASK_WORD_TYPE *) malloc(
		MASK_WORD_SIZE * total_words);
	if (!mask->mask) {
		mask->w = mask->h = 0;
		return;
	}
	#endif

	//Now we initialize some of the fields of the structure...
	mask->w = w;
	mask->h = h;

	#ifdef CLEAR_pmask
	//Now we have a proper mask structure allocated and mostly initialized, but the mask data has garbage! We have to initialize it to 0's:
	for(x=0; x < total_words; x+=1) {
		maskt->mask[x] = 0;
	}
	#else
	//only clear right hand edge if CLEAR_MASK is not defined
	for(x=total_words-h; x < total_words; x+=1) {
		mask->mask[x] = 0;
	}
	#endif
	return;
}


void deinit_pmask(struct PMASK *mask)
{
	mask->w = 0;
	mask->h = 0;
	#ifndef MASK_SINGLE_MEMORY_BLOCK
	if (mask->mask) free(mask->mask);
	mask->mask = NULL;
	#endif
	return;
}


#define BYTE_N(word,index) (((word)>>((index)*8)) & 255)

int serialize_pmask(void *destination, int maxsize, CONST PMASK *source)
{
	unsigned char *dest = (unsigned char *) destination;
	unsigned int i, j, k;
	int bytes = 1 + ((source->w-1) >> 3);
	int words = 1 + ((source->w-1) >> MASK_WORD_BITBITS);
	int size = sizeof(source->w) + sizeof(source->h) + bytes * source->h;
	if (size > maxsize) return -1;
	for (i = 0; i < sizeof(source->w); i += 1) {
		*dest = BYTE_N(source->w, i);
		dest += 1;
	}
	for (i = 0; i < sizeof(source->h); i += 1) {
		*dest = BYTE_N(source->h, i);
		dest += 1;
	}
	for (j = 0; (int)j < words; j += 1) {
		int base = j * sizeof(MASK_WORD_TYPE);
		for (k = 0; (int)k < source->h; k += 1) {
			MASK_WORD_TYPE tmp = source->mask[j * source->h + k];
			base += bytes;
			for (i = 0; i < sizeof(MASK_WORD_TYPE); i += 1) {
				if ((int)(j*sizeof(MASK_WORD_TYPE)+i) < bytes)
					dest[i+base] = BYTE_N(tmp, i);
			}
		}
	}
	return size;
}


#define PUSH_BYTE(word,byte) ((word) = (word<<8) | (byte))

int init_deserialize_pmask(void *source, int maxsize, PMASK *pmask)
{
	unsigned char *src = (unsigned char *) source;
	int w = 0, h = 0;
	int i, j, k;
	int size, bytes, words;

	pmask->w = 0;
	pmask->h = 0;
	#ifndef MASK_SINGLE_MEMORY_BLOCK
	pmask->mask = NULL;
	#endif
	size = sizeof(pmask->w) + sizeof(pmask->h);
	if (maxsize < size) return -1;

	for (i = sizeof(pmask->w)-1; i >= 0; i -= 1) {
		PUSH_BYTE(w, src[i]);
	}
	src += sizeof(pmask->w);
	for (i = sizeof(pmask->h)-1; i >= 0; i -= 1) {
		PUSH_BYTE(h, src[i]);
	}
	src += sizeof(pmask->h);

	bytes = 1 + ((w-1) >> 3);
	words = 1 + ((w-1) >> MASK_WORD_BITBITS);
	size += bytes * h;
	if (maxsize < size) return -1;
	init_pmask(pmask, w, h);
	if (pmask->w != w) return -1;

	for (j = 0; j < words; j += 1) {
		int base = j * sizeof(MASK_WORD_TYPE);
		for (k = 0; k < pmask->h; k += 1) {
			MASK_WORD_TYPE tmp = 0;
			base += bytes;
			for (i = sizeof(MASK_WORD_TYPE)-1; i >= 0; i -= 1) {
				if ((int)(j*sizeof(MASK_WORD_TYPE)+i) > bytes) PUSH_BYTE(tmp,0);
				else PUSH_BYTE(tmp, src[base+i]);
			}
			pmask->mask[j * h + k] = tmp;
		}
	}
	return size;
}


PMASK *create_deserialize_pmask(void *source, int maxsize, int *ret_size)
{
	int w = 0, h = 0;
	int i, j, k;
	int size, bytes, words;
	PMASK *pmask;
	unsigned char *src = (unsigned char *) source;

	*ret_size = -1;
	size = sizeof(pmask->w) + sizeof(pmask->h);
	if (maxsize < size) return NULL;

	for (i = sizeof(pmask->w)-1; i >= 0; i -= 1) {
		PUSH_BYTE(w, src[i]);
	}
	src += sizeof(pmask->w);
	for (i = sizeof(pmask->h)-1; i >= 0; i -= 1) {
		PUSH_BYTE(h, src[i]);
	}
	src += sizeof(pmask->h);

	bytes = 1 + ((w-1) >> 3);
	words = 1 + ((w-1) >> MASK_WORD_BITBITS);
	size += bytes * h;
	if (maxsize < size) return NULL;
	pmask = create_pmask(w, h);
	if (!pmask) return NULL;
	*ret_size = size;

	for (j = 0; j < words; j += 1) {
		int base = j * sizeof(MASK_WORD_TYPE);
		for (k = 0; k < pmask->h; k += 1) {
			MASK_WORD_TYPE tmp = 0;
			base += bytes;
			for (i = sizeof(MASK_WORD_TYPE)-1; i >= 0; i -= 1) {
				if ((int)(j*sizeof(MASK_WORD_TYPE)+i) > bytes) PUSH_BYTE(tmp,0);
				else PUSH_BYTE(tmp, src[base+i]);
			}
			pmask->mask[j * h + k] = tmp;
		}
	}
	return pmask;
}


void destroy_pmask(struct PMASK *mask)
{
	deinit_pmask(mask);
	free(mask);
	return;
}


PMASK *create_pmask (int w, int h)
{
	struct PMASK *maskout;

	#ifdef MASK_SINGLE_MEMORY_BLOCK
	int words, total_words;
	words = 1 + ((w-1) >> MASK_WORD_BITBITS);
	total_words = words * h;
	maskout = (PMASK *) malloc(
		sizeof(PMASK) +
		MASK_WORD_SIZE * total_words );
	if (!maskout) return NULL;
	#else
	maskout = (PMASK *) malloc(sizeof(PMASK));
	if (!maskout) return NULL;
	#endif

	init_pmask(maskout, w, h);

	#ifndef MASK_SINGLE_MEMORY_BLOCK
	if (!maskout->mask) {
		destroy_pmask(maskout);
		return NULL;
	}
	#endif

	return maskout;
}


void pmask_load_func   (struct PMASK *mask, int _x, int _y, void *surface, int trans_color, int (*func)(void*,int,int))
{
	int words, x, y, x2, w, h;
	if (!mask) return;

	w = mask->w;
	h = mask->h;

	words = 1 + ((w-1) >> MASK_WORD_BITBITS);

	//Now we have to create the bit mask array for the sprite:
	for(x=0; x < words; x+=1) {
		for(y=0; y < h; y+=1) {
			MASK_WORD_TYPE m = 0;
			for (x2=MASK_WORD_BITS-1; x2 >= 0; x2-=1) {
				int x3 = (x << MASK_WORD_BITBITS) + x2 + _x;
				m <<= 1;
				if ( x3 < w ) {
					if ( func(surface, x3, y+_y) != trans_color ) {
						m |= 1;
					}
				}
			}
			mask->mask[y+x*h] = m;
		}
	}
	return;
}


void pmask_load_pixels (struct PMASK *mask, void *pixels, int pitch, int bytes_per_pixel, int trans_color)
{
	int words, x, y, x2, w, h;
	if (!mask) return;

	w = mask->w;
	h = mask->h;

	words = 1 + ((w-1) >> MASK_WORD_BITBITS);

	//Now we have to create the bit mask array for the sprite:
	for(x=0; x < words; x+=1) {
		for(y=0; y < h; y+=1) {
			MASK_WORD_TYPE m = 0;
			for (x2=MASK_WORD_BITS-1; x2 >= 0; x2-=1) {
				int x3 = (x << MASK_WORD_BITBITS) + x2;
				m <<= 1;
				if ( x3<w ) {
					//beware of endianness!!!!!!!!!!
					if ( memcmp(((char*)pixels) + x3 * bytes_per_pixel + y * pitch, &trans_color, bytes_per_pixel) == 0 ) {
						m |= 1;
					}
				}
			}
			mask->mask[y+x*h] = m;
		}
	}
	return;
}


#ifdef USE_ALLEGRO
static void load_allegro_pmask(PMASK *mask, BITMAP *sprite)
{
	pmask_load_func (mask, 0, 0, sprite, bitmap_mask_color(sprite), (int (*)(void*,int,int))getpixel);
}


void init_allegro_pmask(struct PMASK *mask, struct BITMAP *sprite)
{
	init_pmask(mask, sprite->w, sprite->h);
	load_allegro_pmask(mask, sprite);
}


PMASK *create_allegro_pmask(struct BITMAP *sprite)
{
	PMASK *ret;
	ret = create_pmask(sprite->w, sprite->h);
	load_allegro_pmask(ret, sprite);
	return ret;
}


void draw_allegro_pmask(CONST PMASK *mask, BITMAP *destination, int x, int y, int color)
{
	int mx, my;
	for (my = 0; my < mask->h; my += 1) {
		for (mx = 0; mx < mask->w; mx += 1) {
			if (_get_pmask_pixel(mask, mx, my))
				putpixel(destination, x+mx, y+my, color);
		}
	}
	return;
}


void draw_allegro_pmask_stretch(CONST PMASK *mask, BITMAP *destination, int x, int y, int w, int h, int color)
{
	int _x, _xmin, _y, _w, _h;
	int scale;
	if (y >= 0) _y = 0; else _y = -y;
	if (y + h <= destination->h) _h = h; else _h = destination->h - y;
	if (x >= 0) _xmin = 0; else _xmin = -x;
	if (x + w <= destination->w) _w = w; else _w = destination->w - x;
	scale = (mask->w << 16) / w;

	bmp_select(destination);
	switch (bitmap_color_depth(destination)) {
		case 8:
		{
			while (_y < _h) {
				int ty, tx;
				unsigned long addr = bmp_write_line(destination, y + _y) + x * sizeof(char);
				//unsigned long *dat = mask->sp_mask[_y * mask->h / h];
				ty = _y * mask->h / h;
				_x = _xmin;
				tx = _x * scale;
				while (_x < _w) {
					//if ( (dat[(tx>>21)] << ((tx>>16) & 31)) & 0x80000000UL )
					if ( _get_pmask_pixel(mask,tx>>16,ty) )
						bmp_write8(addr+_x*sizeof(char), color);
					tx += scale;
					_x += 1;
				}
				_y += 1;
			}
		}
		break;
		case 15:
		{
			bmp_select(destination);
			while (_y < _h) {
				int ty, tx;
				unsigned long addr = bmp_write_line(destination, y + _y) + x * sizeof(short);
				ty = _y * mask->h / h;
				_x = _xmin;
				tx = _x * scale;
				while (_x < _w) {
					//if ( (dat[(tx>>21)] << ((tx>>16) & 31)) & 0x80000000UL )
					if ( _get_pmask_pixel(mask,tx>>16,ty) )
						bmp_write15(addr+_x*sizeof(short), color);
					tx += scale;
					_x += 1;
				}
				_y += 1;
			}
		}
		break;
		case 16:
		{
			bmp_select(destination);
			while (_y < _h) {
				int ty, tx;
				unsigned long addr = bmp_write_line(destination, y + _y) + x * sizeof(short);
				ty = _y * mask->h / h;
				_x = _xmin;
				tx = _x * scale;
				while (_x < _w) {
					//if ( (dat[(tx>>21)] << ((tx>>16) & 31)) & 0x80000000UL )
					if ( _get_pmask_pixel(mask,tx>>16,ty) )
						bmp_write16(addr+_x*sizeof(short), color);
					tx += scale;
					_x += 1;
				}
				_y += 1;
			}
		}
		break;
		case 24:
		{
			bmp_select(destination);
			while (_y < _h) {
				int ty, tx;
				unsigned long addr = bmp_write_line(destination, y + _y) + x * 3;
				ty = _y * mask->h / h;
				_x = _xmin;
				tx = _x * scale;
				while (_x < _w) {
					//if ( (dat[(tx>>21)] << ((tx>>16) & 31)) & 0x80000000UL )
					if ( _get_pmask_pixel(mask,tx>>16,ty) )
						bmp_write24(addr+_x*3, color);
					tx += scale;
					_x += 1;
				}
				_y += 1;
			}
		}
		case 32:
		{
			bmp_select(destination);
			while (_y < _h) {
				int ty, tx;
				unsigned long addr = bmp_write_line(destination, y + _y) + x * 4;
				ty = _y * mask->h / h;
				//unsigned long *dat = mask->mask + ty;
				_x = _xmin;
				tx = _x * scale;
				while (_x < _w) {
					//if ( (dat[(tx>>21)] << ((tx>>16) & 31)) & 0x80000000UL )
					if ( _get_pmask_pixel(mask,tx>>16,ty) )
						bmp_write32(addr+_x*4, color);
					tx += scale;
					_x += 1;
				}
				_y += 1;
			}
		}
		break;
	}
	bmp_unwrite_line(destination);
	return;
}
#endif

#ifdef USE_SDL
static int SDL_getpixel(void *_surface, int x, int y)
{
	int bpp = ((SDL_Surface*)_surface)->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	Uint8 *p = (Uint8 *)((SDL_Surface*)_surface)->pixels + y * ((SDL_Surface*)_surface)->pitch + x * bpp;

	switch(bpp) {
		case 1:
			return *p;

		case 2:
			return *(Uint16 *)p;

		case 3:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
				return p[0] << 16 | p[1] << 8 | p[2];
			else
				return p[0] | p[1] << 8 | p[2] << 16;

		case 4:
			return *(Uint32 *)p;

		default:
			return 0;			 /* shouldn't happen, but avoids warnings */
	}
}


static void load_sdl_pmask(PMASK *mask, SDL_Surface *sprite, , int trans_color)
{
	pmask_load_func (mask, 0, 0, sprite, trans_color, SDL_getpixel);
}


void init_sdl_pmask(struct PMASK *mask, struct SDL_Surface *sprite, int trans_color)
{
	init_pmask(mask, sprite->w, sprite->h);
	load_sdl_pmask(mask, sprite, trans_color);
}


PMASK *create_sdl_pmask(struct SDL_Surface *sprite, int trans_color)
{
	PMASK *ret;
	ret = create_pmask(sprite->w, sprite->h);
	load_sdl_pmask(ret, sprite, trans_color);
	return ret;
}
#endif

int check_pmask_collision(CONST struct PMASK *mask1, CONST struct PMASK *mask2, int x1, int y1, int x2, int y2)
{
	int h1, h2, words1, words2, max1, max2;
	int dx1, dx2, dy1, dy2;		 //We will use this deltas...
	int py;						 //This will store the Y position...
	int maxh;					 //This will store the maximum height...
	int block1, block2;

	//First we do normal bounding box collision detection...
	//If there is not bounding box collision...
	if ( !check_bb_collision(mask1, mask2, x1,y1, x2,y2) )
		return 0;				 //return that there is not collision...

	if (0) {					 //swap 1 & 2
		int tmp;
		CONST PMASK *mtmp;
								 //swap x
		tmp = x1; x1 = x2; x2 = tmp;
								 //swap y
		tmp = y1; y1 = y2; y2 = tmp;
								 //swap masks
		mtmp = mask1; mask1 = mask2; mask2 = mtmp;
	}

	//First we need to see how much we have to shift the coordinates of the masks...
	if (x1>x2) {
		dx1=0;					 //don't need to shift mask 1.
		dx2=x1-x2;				 //shift mask 2 left. Why left? Because we have the mask 1 being on the right of the mask 2, so we have to move mask 2 to the left to do the proper pixel perfect collision...
	} else {
		dx1=x2-x1;				 //shift mask 1 left.
		dx2=0;					 //don't need to shift mask 2.
	}
	if (y1>y2) {
		dy1=0;
		dy2=y1-y2;				 //we need to move this many rows up mask 2. Why up? Because we have mask 1 being down of mask 2, so we have to move mask 2 up to do the proper pixel perfect collision detection...
	} else {
		dy1=y2-y1;				 //we need to move this many rows up mask 1.
		dy2=0;
	}

	block1 = dx1>>MASK_WORD_BITBITS;
	block2 = dx2>>MASK_WORD_BITBITS;
	dx1 &= MASK_WORD_BITS-1;
	dx2 &= MASK_WORD_BITS-1;

	//This will calculate the maximum height that we will reach...
	if (mask1->h-dy1 > mask2->h-dy2) {
		maxh=mask2->h-dy2;
	} else {
		maxh=mask1->h-dy1;
	}
	maxh--;

	h1 = mask1->h;
	h2 = mask2->h;
	words1 = 1 + ((mask1->w-1) >> MASK_WORD_BITBITS);
	words2 = 1 + ((mask2->w-1) >> MASK_WORD_BITBITS);
	max1 = words1 * h1;
	max2 = words2 * h2;
	block1 = block1 * h1 + dy1;
	block2 = block2 * h2 + dy2;

								 //search horizantolly in the outer loop
	while((block1<max1) && (block2<max2) ) {
								 //Search vertically
		for(py=maxh;py>=0;py--) {
			if (
				(mask1->mask[py + block1] >> dx1) &
				(mask2->mask[py + block2] >> dx2)
				)
				return 1;
		}
		//Now we have to move to the next block...
		//we do blocks twice because of the shift
								 //In case both masks are lined up on the x axis...
		if ( (!dx1) && (!dx2) ) {
			block1 += h1;
			block2 += h2;
		} else {
			if (!dx1) {
				block2 += h2;
				dx1 = MASK_WORD_BITS - dx2;
				dx2 = 0;
			} else {
				if (!dx2) {
					block1 += h1;
					dx2 = MASK_WORD_BITS - dx1;
					dx1 = 0;
				}
			}
		}
	}
	return 0;
}


static int pmaskdata_sort ( CONST void *_a, CONST void *_b )
{
	CONST struct PMASKDATA *a = (CONST struct PMASKDATA *) _a;
	CONST struct PMASKDATA *b = (CONST struct PMASKDATA *) _b;
	return a->y - b->y;
}


static int pmaskdata_float_sort ( CONST void *_a, CONST void *_b )
{
	CONST struct PMASKDATA_FLOAT *a = (CONST struct PMASKDATA_FLOAT *) _a;
	CONST struct PMASKDATA_FLOAT *b = (CONST struct PMASKDATA_FLOAT *) _b;
	if (a->y - b->y > 0) return 1;
	else return -1;
}


int check_pmask_collision_list ( PMASKDATA *input, int num, CONST void **output, int max_collisions )
{
	int i, j, ret = 0;
	if (max_collisions <= 0) return 0;
	qsort ( input, num, sizeof(PMASKDATA), pmaskdata_sort);
	for (i = 0; i < num; i += 1) {
		int h = input[i].y + input[i].pmask->h;
		for (j = i+1; (j < num) && (input[j].y < h); j += 1) {
			if (check_pmask_collision(
				input[i].pmask, input[j].pmask,
				input[i].x, input[i].y,
			input[j].x, input[j].y)) {
				output[ret * 2] = input[i].data;
				output[ret * 2 + 1] = input[j].data;
				ret += 1;
				if (ret == max_collisions) return ret;
			}
		}
	}
	return ret;
}


int check_pmask_collision_list_float ( PMASKDATA_FLOAT *input, int num, CONST void **output, int max_collisions )
{
	int i, j, ret = 0;
	if (max_collisions <= 0) return 0;
	qsort ( input, num, sizeof(PMASKDATA), pmaskdata_float_sort);
	for (i = 0; i < num; i += 1) {
		float h = input[i].y + input[i].pmask->h;
		for (j = i+1; (j < num) && (input[j].y < h); j += 1) {
			if (check_pmask_collision(
				input[i].pmask, input[j].pmask,
				iround(input[i].x - input[j].x), iround(input[i].y - input[j].y),
			0, 0)) {
				output[ret * 2] = input[i].data;
				output[ret * 2 + 1] = input[j].data;
				ret += 1;
				if (ret == max_collisions) return ret;
			}
		}
	}
	return ret;
}


int check_pmask_collision_list_wrap ( int maxx, int maxy, PMASKDATA *input, int num, CONST void **output, int max_collisions )
{
	int i, j, ret = 0, maxxh = (maxx+1) >> 1;
	if (max_collisions <= 0) return 0;
	if (maxx <= 0) return 0;
	if (maxy <= 0) return 0;
	qsort ( input, num, sizeof(PMASKDATA), pmaskdata_sort);
	for (i = 0; i < num; i += 1) {
		int h = input[i].y + input[i].pmask->h;
		for (j = i+1; (j < num) && (input[j].y < h); j += 1) {
			int cr;
			int dx = input[i].x - input[j].x;
			if ( abs( dx ) >= maxxh ) {
				while (dx >= maxxh) dx -= maxx;
				while (dx <= -maxxh) dx += maxx;
			}
			cr = check_pmask_collision(
				input[i].pmask, input[j].pmask,
				dx, input[i].y-input[j].y,
				0, 0);
			if ( cr ) {
				output[ret * 2] = input[i].data;
				output[ret * 2 + 1] = input[j].data;
				ret += 1;
				if (ret == max_collisions) return ret;
			}
		}
		if (h > maxy) {
			h -= maxy;
			for (j = 0; (j < i) && (input[j].y < h); j += 1) {
				int dx = input[i].x - input[j].x;
				if ( abs( dx ) >= maxxh ) {
					while (dx >=  maxxh) dx -= maxx;
					while (dx <= -maxxh) dx += maxx;
				}
				if (check_pmask_collision(
					input[i].pmask, input[j].pmask,
					dx, input[i].y-maxy-input[j].y,
				0, 0)) {
					output[ret * 2] = input[i].data;
					output[ret * 2 + 1] = input[j].data;
					ret += 1;
					if (ret == max_collisions) return ret;
				}
			}
		}
	}
	return ret;
}


int check_pmask_collision_list_float_wrap ( float maxx, float maxy, PMASKDATA_FLOAT *input, int num, CONST void **output, int max_collisions )
{
	int i, j, ret = 0;
	float maxxh = maxx / 2;
	if (max_collisions <= 0) return 0;
	if (maxx <= 0) return 0;
	if (maxy <= 0) return 0;
	qsort ( input, num, sizeof(PMASKDATA_FLOAT), pmaskdata_float_sort);
	for (i = 0; i < num; i += 1) {
		float h = input[i].y + input[i].pmask->h;
		for (j = i+1; (j < num) && (input[j].y < h); j += 1) {
			int cr;
			float dx = input[i].x - input[j].x;
			if ( fabs( dx ) >= maxxh ) {
				while (dx >= maxxh) dx -= maxx;
				while (dx <= -maxxh) dx += maxx;
			}
			cr = check_pmask_collision(
				input[i].pmask, input[j].pmask,
				iround(dx), iround(input[i].y-input[j].y),
				0, 0);
			if ( cr ) {
				output[ret * 2] = input[i].data;
				output[ret * 2 + 1] = input[j].data;
				ret += 1;
				if (ret == max_collisions) return ret;
			}
		}
		if (h > maxy) {
			h -= maxy;
			for (j = 0; (j < i) && (input[j].y < h); j += 1) {
				int dx = iround(input[i].x - input[j].x);
				if ( abs( dx ) >= maxxh ) {
					while (dx >=  maxxh) dx -= iround(maxx);
					while (dx <= -maxxh) dx += iround(maxx);
				}
				if (check_pmask_collision(
					input[i].pmask, input[j].pmask,
					dx, iround(input[i].y-maxy-input[j].y),
				0, 0)) {
					output[ret * 2] = input[i].data;
					output[ret * 2 + 1] = input[j].data;
					ret += 1;
					if (ret == max_collisions) return ret;
				}
			}
		}
	}
	return ret;
}
