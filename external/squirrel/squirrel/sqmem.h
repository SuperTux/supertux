/*	see copyright notice in squirrel.h */
#ifndef _SQMEM_H_
#define _SQMEM_H_

void *sq_vm_malloc(SQUnsignedInteger size);
void *sq_vm_realloc(void *p,SQUnsignedInteger oldsize,SQUnsignedInteger size);
void sq_vm_free(void *p,SQUnsignedInteger size);
#endif //_SQMEM_H_
