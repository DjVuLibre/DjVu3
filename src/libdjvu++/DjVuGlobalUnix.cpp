/* This is for building the only object needed if we aren't overloading the
 * memory handlers.
 */

#ifdef UNIX
#ifdef NEED_DJVU_MEMORY
#ifndef NEED_DJVU_MEMORY_IMPLEMENTATION
#define NEED_DJVU_MEMORY_IMPLEMENTATION
#endif
#include "DjVuGlobal.h"

djvu_delete_callback *_djvu_delete_ptr=(djvu_delete_callback *)&(operator delete);
djvu_delete_callback *_djvu_deleteArray_ptr=(djvu_delete_callback *)&(operator delete []);
djvu_new_callback *_djvu_new_ptr=(djvu_new_callback *)&(operator new);
djvu_new_callback *_djvu_newArray_ptr=(djvu_new_callback *)&(operator new []);
#endif
#endif

