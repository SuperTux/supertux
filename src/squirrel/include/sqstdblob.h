/*	see copyright notice in squirrel.h */
#ifndef _SQSTDBLOB_H_
#define _SQSTDBLOB_H_

#ifdef __cplusplus
extern "C" {
#endif

SQUIRREL_API SQUserPointer sqstd_createblob(HSQUIRRELVM v, int size);
SQUIRREL_API SQRESULT sqstd_getblob(HSQUIRRELVM v,int idx,SQUserPointer *ptr);
SQUIRREL_API int sqstd_getblobsize(HSQUIRRELVM v,int idx);

SQUIRREL_API SQRESULT sqstd_register_bloblib(HSQUIRRELVM v);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*_SQSTDBLOB_H_*/

