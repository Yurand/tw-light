
#include <stdlib.h>
#include <string.h>
#include "base.h"
#include "errors.h"

COMPILE_TIME_ASSERT(sizeof(char)==1);
COMPILE_TIME_ASSERT(sizeof(short)==2);
COMPILE_TIME_ASSERT(sizeof(int)==4);
//COMPILE_TIME_ASSERT(sizeof(long)==4);

__call_before_main::__call_before_main ( void (*func)() )
{
	func();
}


char *tw_strdup ( const char *str )
{
	int l = strlen(str);
	char *r = (char*) malloc(l+1);
	strcpy(r, str);
	return r;
}


#ifdef TW_MALLOC_CHECK
#   undef malloc
#   undef realloc
#   undef free
void *tw_malloc(int size)
{
	void *r = malloc(size);
	if (!r) error_oom();
	return r;
}


void *tw_realloc(void *old, int size)
{
	void *r = realloc(old, size);
	if (!r && size) error_oom();
	return r;
}


void tw_free(void *mem)
{
	STACKTRACE;
	free(mem);
}
#endif

/*------------------------------
		Base Class
------------------------------*/

void BaseClass::preinit()
{
	STACKTRACE;
	return;
}


BaseClass::~BaseClass()
{
}


void **BaseClass::get_vtable_pointer() const
{
	if (sizeof(this) != sizeof(void*))
		throw "get_vtable_pointer failed (ptr* size weird)";
	//error("get_vtable_pointer failed (size == %d != !%d)", sizeof(this), sizeof(void*));
	return ((void**)this);
}


int BaseClass::serialize(void *stream)
{
	STACKTRACE;
	return 0;
}


int BaseClass::_get_size() const
{
	return 0;
}


void BaseClass::_event( Event *e)
{
	STACKTRACE;
	return;
}


void BaseClass::issue_event ( std::list<BaseClass*>& recipients, Event *e)
{
	for(std::list<BaseClass*>::iterator i = recipients.begin();
	i != recipients.end(); i++) {
		(*i)->_event(e);
	}
}
