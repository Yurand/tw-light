#ifndef __GET_TIME_H__
#define __GET_TIME_H__

#ifdef __cplusplus
extern "C"
{
	#endif

	void init_time();			 //to initialize time stuff
	void deinit_time();			 //to de-initialize time stuff

	int is_time_initialized();	 //returns non-zero if init_time() has been called, otherwise 0

	int    get_time();			 //to get the current time in milliseconds
	//in all cases, the current time is measured relative to the call to init_time();

	int idle ( int time );		 //to yeild a number of milliseconds to the OS
	extern int _no_idle;		 //to disable the above function

	// wait (_d) msec until key pressed
	#define HAVE_A_WAIT(_d) \
		{ \
			int time = get_time(); \
			while (time + _d > get_time()) \
			{ \
				while (keypressed()) \
				{ \
					return; \
				} \
				idle(10); \
			} \
		}

	#ifdef __cplusplus
}
#endif
#endif							 // __GET_TIME_H__
