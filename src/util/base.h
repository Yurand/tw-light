#ifndef __BASE_H__
#define __BASE_H__

#include "types.h"

#ifdef _MSC_VER
#define for  if (0) ; else for
#ifndef INLINE
#define INLINE __inline
#endif
#else
#ifndef INLINE
#define INLINE inline
#endif
#endif

#ifndef __i386__
#if defined(__I386__) || defined(__IA32__) || defined(__ia32__)
#define __i386__
#endif
#endif

//char *tw_strdup(const char *str);
//#define strdup  tw_strdup

//#define TW_MALLOC_CHECK
#ifdef TW_MALLOC_CHECK
#   ifdef __cplusplus
extern "C"
{
	#   endif
	void *tw_malloc(int size);
	void *tw_realloc(void *old, int size);
	void tw_free(void *item);
	#   define malloc  tw_malloc
	#   define realloc tw_realloc
	#   define free    tw_free
	#   ifdef __cplusplus
}


static inline void *operator new(unsigned int size) {return tw_malloc(size);}
static inline void operator delete(void *item) {tw_free(item);}
#   endif
#endif

#ifdef __cplusplus

#include <list>

class __call_before_main
{
	public:__call_before_main ( void (*func)());
};
#define CALL_BEFORE_MAIN(a) static __call_before_main __call_ ## a ( a ) ;
#define CALL_BEFORE_MAIN2(id, code) static void __call_before_main2 ## id(){code;} CALL_BEFORE_MAIN(__call_before_main2 ## id)

//#define REGISTER_UDT(a,b) CALL_BEFORE_MAIN ( __udt_register_ ## a );
//#define DECLARE_UDT virtual int getsize(); virtual const char *get_class_name(); virtual const char *get_class_parent_name();

struct _Ignore_Me				 //dummy type
{
};

class Event;
class BaseClass
{
	public:
		virtual void preinit();
		virtual void _event( Event * e);
		virtual ~BaseClass ();	 //does nothing, but is necessary
								 //returns the address of the virtual table pointer inside of an instance
		void **get_vtable_pointer () const;
		void issue_event ( std::list<BaseClass*>& list, Event *event);
								 //returns 0 on failure
		virtual int serialize (void *stream);
								 //returns 0 on failure
		virtual int _get_size() const;
};

/// \brief Game Event (VIDEO, KEYBOARD, MOUSE ...)
class Event : public BaseClass
{
	public:
		short type;
		short subtype;

		enum {
			VIDEO = 1,
			KEYBOARD,
			MOUSE,
			NETWORK,
			LAST_GENERIC_EVENT = 15
		};						 //generic events

		enum {
			TW_NET1 = LAST_GENERIC_EVENT + 1,
			TW_CONFIG
		};						 //TimeWarp events

		virtual int _get_size() const {return sizeof(*this);}
};
#endif
#endif							 // __BASE_H__
