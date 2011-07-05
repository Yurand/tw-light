#ifndef __PMASK_H__
#define __PMASK_H__

#include <limits.h>
/*
This is the Pixel MASK library, which does pixel-perfect collisions using
bit masks.

There are several configuration options here, hope they aren't too
confusing.
*/

#define PMASK_VERSION          4

#define USE_ALLEGRO
//#define USE_SDL

#ifdef USE_ALLEGRO
struct BITMAP;
#   ifdef ALLEGRO_LITTLE_ENDIAN
#       define PMASK_LITTLE_ENDIAN
#   endif
#   ifdef ALLEGRO_BIG_ENDIAN
#       define PMASK_BIG_ENDIAN
#   endif
#endif
#ifdef USE_SDL
struct SDL_Surface;
#   if SDL_BYTEORDER == SDL_LITTLE_ENDIAN
#       define PMASK_LITTLE_ENDIAN
#   endif
#   if SDL_BYTEORDER == SDL_BIG_ENDIAN
#       define PMASK_BIG_ENDIAN
#   endif
#endif

#ifdef __cplusplus
#   define pmask_USE_INLINE
extern "C"
{
	#endif

	//MASK_WORD_TYPE and MASK_WORD_BITBITS can be changed for your platform

	//MASK_WORD_TYPE should be the largest fast integer type available
	#define MASK_WORD_TYPE unsigned long int

	//MASK_WORD_BITBITS should be the log base 2
	//of the number of bits in MASK_WORD_TYPE
	//e.g. 4 for 16-bit ints, 5 for 32-bit ints, 6 for 64-bit ints
	//don't worry about setting it incorrectly
	//you'll get a compile error if you do, not a run-time error

	#if ULONG_MAX == 0xFFFFFFFFUL
	#define MASK_WORD_BITBITS 5
	#else
	#define MASK_WORD_BITBITS 6
	#endif

	//#ifdef NATIVE_64
	//#define MASK_WORD_BITBITS 6
	//#else
	//#define MASK_WORD_BITBITS 5
	//#endif

	//if SINGLE_MEMORY_BLOCK is defined
	//then each mask will be allocated as
	//only a single memory block.
	//this means that masks will (in theory)
	//be ever-so-slightly faster and more memory efficient
	//however, if in single memory block mode
	//then the masks are variable-sized
	//so you can not use an array of them
	//#define MASK_SINGLE_MEMORY_BLOCK

	//trying to make portable const-ness easier:
	#ifndef CONST
	#define CONST const
	#endif

	//This is the bounding box collision detection macro.
	//It is a general purpose macro. Pass it the coordinates of one rectangle and the width and
	//height of it, then pass the coordinates of a second rectangle and the width and height of
	//it. It will return 0 if theres not collision or 1 if there is a collision between the
	//rectangles (the ectangles overlap).
	//This macro works looking for out of range values, if some value is out of
	//range it returns 0, if all the values are in range it returns true.
	#define check_bb_collision_general(x1,y1,w1,h1,x2,y2,w2,h2) (!( ((x1)>=(x2)+(w2)) || ((x2)>=(x1)+(w1)) || ((y1)>=(y2)+(h2)) || ((y2)>=(y1)+(h1)) ))
	#define check_bb_collision(mask1,mask2,x1,y1,x2,y2) check_bb_collision_general(x1,y1,mask1->w,mask1->h,x2,y2,mask2->w,mask2->h)

	typedef struct PMASK
	{
		short int w;			 //width
		short int h;			 //height
		#ifdef MASK_SINGLE_MEMORY_BLOCK
		MASK_WORD_TYPE mask[1];	 //mask data (single memory block)
		#else
		MASK_WORD_TYPE *mask;	 //mask data (pointer at second memory block)
		#endif
	} PMASK;

	void install_pmask();		 //sets up library

								 //checks for collision (0 = no collision, 1 = collision)
	int check_pmask_collision(CONST struct PMASK *mask1, CONST struct PMASK *mask2, int x1, int y1, int x2, int y2);

								 //returns 0 if mask is clear at those coordinates, 1 if not clear
	int get_pmask_pixel(CONST struct PMASK *mask, int x, int y) ;
								 //makes mask clear at those coordinates if value is zero, others makes mask not-clear at those coordinates
	void set_pmask_pixel(struct PMASK *mask, int x, int y, int value) ;

								 //initializes a PMASK
	void init_pmask        (struct PMASK *mask, int w, int h);
								 //loads a pmask with pixel data from memory
	void pmask_load_pixels (struct PMASK *mask, void *pixels, int pitch, int bytes_per_pixel, int trans_color);
								 //loads a pmask with pixel data from a function
	void pmask_load_func   (struct PMASK *mask, int x, int y, void *surface, int trans_color, int (*func)(void*,int,int));
								 //de-initializes a pmask
	void deinit_pmask(PMASK *mask);

								 //creates a new pmask and initializes it
	PMASK *create_pmask  (int w, int h);
								 //destroys a pmask created by create_pmask
	void destroy_pmask(struct PMASK *mask);

	#if defined USE_ALLEGRO
	void init_allegro_pmask(struct PMASK *mask, struct BITMAP *sprite);
	PMASK *create_allegro_pmask(struct BITMAP *sprite);
	void draw_allegro_pmask(CONST struct PMASK *mask, struct BITMAP *destination, int x, int y, int color) ;
	void draw_allegro_pmask_stretch(CONST struct PMASK *mask, struct BITMAP *destination, int x, int y, int w, int h, int color) ;
	#endif

	#if defined USE_SDL
	void init_sdl_pmask(struct PMASK *mask, struct SDL_Surface *sprite, int trans_color);
	PMASK *create_sdl_pmask(struct SDL_Surface *sprite, int trans_color);
	#endif

	int serialize_pmask(void *destination, int maxsize, CONST PMASK *source);
	//serialize_pmask is a helper for sending a PMASK to a file or over a network
	//it accepts a buffer to write to, and a maxime number of bytes to write
	//it returns the number of bytes written, or -1 if an error occured
	int init_deserialize_pmask(void *source, int maxsize, PMASK *destination);
	//init_deserialize_pmask is a helper for reading a PMASK from a file or over a network
	//it accepts a buffer to read from, and a maximum number of bytes to read
	//it also accepts a pointer at the PMASK to initialize
	//it returns the number of bytes read
	PMASK *create_deserialize_pmask(void *source, int maxsize, int *size);
	//create_deserialize_pmask is a helper for reading a PMASK from a file or over a network
	//it accepts a buffer to read from, and a maximum number of bytes to read
	//it accepts a pointer at an integer to put the # of bytes read
	//it returns a newly allocated PMASK structure, or NULL if an error occured

	typedef struct PMASKDATA
	{
		int x, y;
		CONST PMASK *pmask;
		CONST void *data;
	} PMASKDATA;
	typedef struct PMASKDATA_FLOAT
	{
		float x, y;
		CONST PMASK *pmask;
		CONST void *data;
	} PMASKDATA_FLOAT;
	int check_pmask_collision_list ( PMASKDATA *input, int num, CONST void **output, int max_collisions );
	int check_pmask_collision_list_float ( PMASKDATA_FLOAT *input, int num, CONST void **output, int max_collisions );
	int check_pmask_collision_list_wrap ( int wrap_x, int wrap_y, PMASKDATA *input, int num, CONST void **output, int max_collisions );
	int check_pmask_collision_list_float_wrap ( float wrap_x, float wrap_y, PMASKDATA_FLOAT *input, int num, CONST void **output, int max_collisions );
	//check_pmask_collision_list is higher-level collision code
	//it accepts an array of PMASKDATA, which contains information about objects
	//and it returns a list of collisions
	//An alternate version is provided if you prefer to work with floating point numbers
	//Alternate versions are provided if you work in a wrapping coordinate system
	//Note that these WILL modify the input array - the order will be changed
	//It returns the number of collisions found, at most max_collisions
	//The output array holds a pair of pointers for each collision
	//Each pointer in a pair is the "data" pointer from the input for one
	//element involved in a collision

	#ifdef __cplusplus
}
#endif

#define MASK_WORD_SIZE sizeof(MASK_WORD_TYPE)
#define MASK_WORD_BITS (MASK_WORD_SIZE*8)

#if defined pmask_USE_INLINED
inline int _get_pmask_pixel(struct PMASK *mask, int x, int y)
{
	//no bounds checking
	return 1 & (mask->mask[(mask->h * (x >> MASK_WORD_BITBITS)) + y] >> (x & (MASK_WORD_BITS-1)));
}


inline void _set_pmask_pixel(struct PMASK *mask, int x, int y, int value)
{
	//no bounds checking
	if (value) {
		mask->mask[(mask->h * (x >> MASK_WORD_BITBITS)) + y] |= 1 << (x & (MASK_WORD_BITS-1));
	} else {
		mask->mask[(mask->h * (x >> MASK_WORD_BITBITS)) + y] &=~(1 << (x & (MASK_WORD_BITS-1)));
	}
}


#else
#   define _get_pmask_pixel get_pmask_pixel
#   define _set_pmask_pixel set_pmask_pixel
#endif
#endif							 // __PMASK_H__
