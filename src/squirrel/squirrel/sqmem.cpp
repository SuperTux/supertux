/*
	see copyright notice in squirrel.h
*/
#include "sqpcheader.h"
void *sq_vm_malloc(unsigned int size){	return malloc(size); }

void *sq_vm_realloc(void *p, unsigned int oldsize, unsigned int size){ return realloc(p, size); }

void sq_vm_free(void *p, unsigned int size){	free(p); }
