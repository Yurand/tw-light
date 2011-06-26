#ifndef __ERRORS_H__
#define __ERRORS_H__

#define DO_STACKTRACE
#define USE_ALLEGRO

#define USE_EXCEPTIONS

#define COMPILE_TIME_ASSERT(condition) typedef char _compile_time_assert_type_ ## __LINE__ [(condition) ? 1 : -1];

/*
 * int tw_alert(char *message, char *b1, char *b2, char *b3)
 *
 * Replaces allegro's alert and alert3. It handles carriage returns.
 */

#ifdef __cplusplus
#include <stack>
#include <string>

int tw_alert(const char *message, const char *b1 = 0, const char *b2 = 0, const char *b3 = 0, const char *b4 = 0) ;

// for errors caught in "catch" clauses
void caught_error(const char *format, ...);

//quits TW with an error message
//used for catastrophic errors
void tw_error_exit(const char* message) ;

#include "get_time.h"

struct SOURCE_LINE
{
	int line;					 // __LINE__
	const char *file;			 // __FILE__
	const char *funct;			 // __FUNCTION__
	const char *name;			 // NULL or a descriptor string
};

/// \brief Call stack tracker
///
/// Object of this class is created on stack in STACKTRACE macro, so it's creating/destruction can
/// be used to manage  call stack.
class UserStackTraceHelper
{
	static std::stack<SOURCE_LINE*> call_stack;
	static std::string GetStackNodeString(SOURCE_LINE* stNode);
	public:
		UserStackTraceHelper( SOURCE_LINE* srcline);
		~UserStackTraceHelper();

		/// \brief get stack trace string
		static std::string get_stack_trace_string();
};

#define STACKTRACE static SOURCE_LINE _srcline = { __LINE__, __FILE__, __FUNCTION__, 0 }; UserStackTraceHelper _stacktrace_ ( &_srcline );
#define _STACKTRACE(A) static SOURCE_LINE _srcline = { __LINE__, __FILE__, __FUNCTION__, A }; UserStackTraceHelper _stacktrace_ ( &_srcline );

extern "C"
{
	#endif

	void init_error() ;			 //initialize error handling routines
	void deinit_error();		 //de-initialize error handling routines

	/*
	 * void tw_error(const char *format, ...)
	 *
	 * WARNING: the macro implementation of this function can cause compile
	 *     errors.  If your code complains about a line that calls
	 *     tw_error, wrap the call to tw_error with curly braces like
	 *     this: {tw_error("My error message");}
	 *
	 * This function is used to query the user what action to take when an error
	 * occurs.  It will pop up a box displaying the error string and prompting
	 * the user to abort, retry, or debug.  It may also display some diagnostic
	 * data, such as a call stack.  To avoid showing the call stack, call error
	 * instead of tw_error.
	 *
	 * If the user clicks on "retry" then error() will return.
	 *
	 * If the user clicks on "abort" then error() will throw 0. This will cause
	 * one level of the program to be aborted. (i.e. from game to main menu)
	 *
	 * If the user clicks on "debug" then error() will attempt to abort the
	 * entire program in a way that lets the debugger work on things. On MSVC
	 * this is currently implemented with the assembly instruction "int 3".  On
	 * other platforms this is currently implemented by intentionally
	 * derefencing a NULL pointer.
	 *
	 */

	#   define tw_error _prep_error_func(__FILE__, __LINE__)

	void error_handler ( const char *message);
	extern void (*_error_handler) (
		int in_catch_statement,	 //"retry" is invalid inside a catch statement
		const char *src_file,
		int line,
		const char *message
		);

	typedef void (*ERROR_FUNC_TYPE)(const char *fmt, ...);

	ERROR_FUNC_TYPE _prep_error_func ( const char *file, int line );
	void _prep_error(const char *file, int line);
	void _error(const char *format, ...);

	/*
	 * void log_debug(const char *format, ...)
	 *
	 * Records a log of certain information to disk.
	 * Typical log message: "Sound Initialized (10 voices sfx, 22 for music)"
	 */
	void log_debug(const char *format, ...);

	/*
	 * void tw_error_exit(char* message)
	 *
	 * Informs a user about an error and exits. This function does not return.
	 * This is for errors so critical that there is no hope of recovery
	 */

	/*
	 * void tw_exit(int errorcode)
	 *
	 * Quits timewarp.  Calls the standard libc function exit()
	 * after cleaning up a few things.
	 *
	 */
	void tw_exit(int errorcode) ;

	/*
	 * void error_oom()
	 *
	 * If you have a critical Out Of Memory error, call this function.
	 * It will quit TW and attempt to notify the user.
	 *
	 */
	void error_oom();

	// not for user-code
	//this flag is set to 0 normally, or 1 to bypass error catching
	extern int __error_flag;

	#if defined __cplusplus
}
#endif
#endif							 // __ERRORS_H__
