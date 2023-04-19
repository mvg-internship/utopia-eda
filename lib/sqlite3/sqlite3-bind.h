/* ---------------------------------------------------------------------------
** sqlite3-bind: SQLite C API - Parameter binding helper for SQLite.
** ---------------------------------------------------------------------------
** Copyright (c) 2016 by Payton Bissell, payton.bissell@gmail.com
** ---------------------------------------------------------------------------
** Permission to use, copy, modify, and/or distribute this software for any
** purpose with or without fee is hereby granted. 
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
** OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
** THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
**
** Also offered with MIT License. 
**
** INTENTION: I, Payton Bissell, the sole author of this source code, dedicate
** any and all copyright interest in this code to the public domain. I make
** this dedication for the benefit of the public at large and to the detriment
** of my heirs and successors. I intend this dedication to be an overt act of
** relinquishment in perpetuity of all present and future rights to this code
** under copyright law.
**
** Copyleft is the antithesis of freedom. Please resist it.
** ---------------------------------------------------------------------------
** You can disable stack termination checking by defining
**   I_SQLITE_BIND_STACK_NOT_CHECKED prior to sqlite3-bind.h
**   e.g. gcc -DI_SQLITE_BIND_STACK_NOT_CHECKED
**
** ---------------------------------------------------------------------------
*/

#ifndef _SQLITE3_BIND_H_
#define _SQLITE3_BIND_H_

#include <stdarg.h>     /* Needed for the definition of va_list */
#include "sqlite3.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
** Error codes: Since this tool simple wraps SQLite calls, most of the 
** return codes are those returned by SQLite. Using negative result/error
** codes until more research can be done. 
*/
#define SQLITE_ERR_BIND_STACK_NOT_TERMINATED    (-1)   // no end marker after all params are bound
#define SQLITE_ERR_BIND_STACK_MISSING_PARAMS    (-2)   // end marker before all params are bound
#define SQLITE_ERR_BIND_STACK_GUIDE_INVALID     (-3)   // guide marker was not recognized.
#define SQLITE_ERR_BIND_RESULT_COLUMNS_COUNT    (-4)   // the number of result columns does not match setup
#define SQLITE_ERR_BIND_ARRAY_CANT_HAVE_RESULTS (-5)   // for now the bind_array functions cannot return results
                                                       // a query that generates results will create this error

/* ---------------------------------------------------------------------------
** INTERNAL random guide bytes that provide some confidence that the stack 
** variables are the correct type
*/
#define I_SQLITE_BIND_TYPE_BLOB    ((unsigned int)0x156a82f4)
#define I_SQLITE_BIND_TYPE_DOUBLE  ((unsigned int)0x3a4947a1)
#define I_SQLITE_BIND_TYPE_INT     ((unsigned int)0xedf19d91)
#define I_SQLITE_BIND_TYPE_INT64   ((unsigned int)0xc3aea174)
#define I_SQLITE_BIND_TYPE_NULL    ((unsigned int)0x93640739)
#define I_SQLITE_BIND_TYPE_TEXT    ((unsigned int)0xa352651d)
#define I_SQLITE_BIND_TYPE_TEXT16  ((unsigned int)0xe55e1879)
#define I_SQLITE_BIND_TYPE_ZBLOB   ((unsigned int)0x7c435b51)  

/* ---------------------------------------------------------------------------
** Public function to use in-place of sqlite3_errmsg
*/
const char *sqlite3_bind_errmsg(sqlite3 *db);

/* ---------------------------------------------------------------------------
** User macros for pushing params on the stack for binding, provide guide marker and casting
*/
#define SQLITE_BIND_BLOB(b, s)    I_SQLITE_BIND_TYPE_BLOB, (int)(s), (const void*)(b)
#define SQLITE_BIND_DOUBLE(d)     I_SQLITE_BIND_TYPE_DOUBLE, (double)(d)
#define SQLITE_BIND_INT(i)        I_SQLITE_BIND_TYPE_INT, (int)(i)
#define SQLITE_BIND_INT64(i)      I_SQLITE_BIND_TYPE_INT64, (sqlite3_int64)(i)
#define SQLITE_BIND_NULL          I_SQLITE_BIND_TYPE_NULL
#define SQLITE_BIND_TEXT(t)       I_SQLITE_BIND_TYPE_TEXT, (const char*)(t)
#define SQLITE_BIND_TEXT16(t)     I_SQLITE_BIND_TYPE_TEXT16, (const void*)(t)
#define SQLITE_BIND_ZBLOB(s)      I_SQLITE_BIND_TYPE_ZBLOB, (int)(s)
#define SQLITE_BIND_END           ((unsigned int)0x87fa3dab)

/* ---------------------------------------------------------------------------
** The sqlite_bind_exec functions follow the sqlite_exec API pattern
** but support variable arguments that will be bound to '?' markers
** in the sql statement.
** ---------------------------------------------------------------------------
*/
int sqlite3_bind_exec      (sqlite3 *db, const char *sql, int (*callback)(void*,int,char**,char**), void *arg, ...);
int sqlite3_bind_exec16    (sqlite3 *db, const void *sql, int (*callback)(void*,int,char**,char**), void *arg, ...);
int sqlite3_bind_exec_va   (sqlite3 *db, const char *sql, int (*callback)(void*,int,char**,char**), void *arg, va_list params);
int sqlite3_bind_exec_va16 (sqlite3 *db, const void *sql, int (*callback)(void*,int,char**,char**), void *arg, va_list params);

/* ---------------------------------------------------------------------------
** The sqlite_bind_array functions are a convienence for inserting arrays
** of data in a single call using the argument binding features of sqlite.
** ---------------------------------------------------------------------------
*/

/* ---------------------------------------------------------------------------
** User macros for pushing params on the stack for binding arrays
*/
#define SQLITE_BIND_ARRAY_BLOB(ba,sa)    (I_SQLITE_BIND_TYPE_BLOB+1), (int*)(sa), (const void**)(ba)
#define SQLITE_BIND_ARRAY_DOUBLE(da)     (I_SQLITE_BIND_TYPE_DOUBLE+1), (double*)(da)
#define SQLITE_BIND_ARRAY_INT(ia)        (I_SQLITE_BIND_TYPE_INT+1), (int*)(ia)
#define SQLITE_BIND_ARRAY_INT64(ia)      (I_SQLITE_BIND_TYPE_INT64+1), (sqlite3_int64*)(da)
#define SQLITE_BIND_ARRAY_TEXT(ta)       (I_SQLITE_BIND_TYPE_TEXT+1), (const char**)(ta)
#define SQLITE_BIND_ARRAY_TEXT16(ta)     (I_SQLITE_BIND_TYPE_TEXT16+1), (const void**)(ta)
#define SQLITE_BIND_ARRAY_NULL           (I_SQLITE_BIND_TYPE_NULL+1)
#define SQLITE_BIND_ARRAY_ZBLOB(s)       (I_SQLITE_BIND_TYPE_ZBLOB+1), (int)(s)

#define SQLITE_BIND_BLOCK_TEXT(t,s)      (I_SQLITE_BIND_TYPE_TEXT+2), (int)(s), (const char*)(t)

int sqlite3_bind_array      (sqlite3 *db, const char *sql, int rows, ...);
int sqlite3_bind_array_va   (sqlite3 *db, const char *sql, int rows, va_list params);
int sqlite3_bind_array16    (sqlite3 *db, const void *sql, int rows, ...);
int sqlite3_bind_array_va16 (sqlite3 *db, const void *sql, int rows, va_list params);

#ifdef __cplusplus
}
#endif

#endif // _SQLITE3_BIND_H_

/* EOF */