/* see copyright notice in squirrel.h */
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdblob.h>
#include "sqstdstream.h"
#include "sqstdblobimpl.h"

#define SETUP_STREAM(v) \
	SQStream *self = NULL; \
	if(SQ_FAILED(sq_getinstanceup(v,1,(SQUserPointer*)&self,SQSTD_STREAM_TYPE_TAG))) \
		return sq_throwerror(v,_SC("invalid type tag")); \
	if(!self->IsValid())  \
		return sq_throwerror(v,_SC("the stream is invalid"));

int _stream_readstr(HSQUIRRELVM v)
{
    SETUP_STREAM(v);
	SQInteger type = _SC('a'), size = 0;
	sq_getinteger(v, 2, &size);
	if(size <= 0) return sq_throwerror(v,_SC("invalid size"));
	if(sq_gettop(v) > 2)
		sq_getinteger(v, 3, &type);
	SQChar *dest = NULL;
	switch(type) {
	case _SC('a'): {
		char *temp;
		if(self->Read(sq_getscratchpad(v, size+1), size) != size)
			return sq_throwerror(v, _SC("io failure"));
#ifdef _UNICODE
		temp = (char*) sq_getscratchpad(v, size + (size * sizeof(SQChar)));
		dest = (SQChar*) &temp[size];
		size = (SQInteger)mbstowcs(dest, (const char*)temp, size);
#else
		temp = (char *) sq_getscratchpad(v, -1);
		dest = temp;
#endif
				   }
		break;
	case _SC('u'): {
		wchar_t *temp;
		if(self->Read(sq_getscratchpad(v, (size + 1) * sizeof(wchar_t)),size * sizeof(wchar_t)) != (size * sizeof(wchar_t)))
			return sq_throwerror(v, _SC("io failure"));
		
#ifdef _UNICODE
		temp = (wchar_t*) sq_getscratchpad(v, -1);
		dest = (SQChar*) temp;
#else
		temp = (wchar_t*) sq_getscratchpad(v,(size * 3) + (size * sizeof(wchar_t)));
		dest = (char*) &temp[size];
		size = (SQInteger)wcstombs(dest, (const wchar_t*)temp, size);
#endif
				   }
		break;
	default:
		return sq_throwerror(v, _SC("invalid coding"));
	}

	sq_pushstring(v, dest, size);
	return 1;
}

int _stream_readblob(HSQUIRRELVM v)
{
	SETUP_STREAM(v);
	SQUserPointer data,blobp;
	SQInteger size,res;
	sq_getinteger(v,2,&size);
	if(size > self->Len()) {
		size = self->Len();
	}
	data = sq_getscratchpad(v,size);
	res = self->Read(data,size);
	if(res <= 0)
		return sq_throwerror(v,_SC("no data left to read"));
	blobp = sqstd_createblob(v,res);
	memcpy(blobp,data,res);
	return 1;
}

#define SAFE_READN(ptr,len) { \
	if(self->Read(ptr,len) != len) return sq_throwerror(v,_SC("io error")); \
	}
int _stream_readn(HSQUIRRELVM v)
{
	SETUP_STREAM(v);
	SQInteger format;
	sq_getinteger(v, 2, &format);
	switch(format) {
	case 'i': {
		int i;
		SAFE_READN(&i, sizeof(i));
		sq_pushinteger(v, i);
			  }
		break;
	case 's': {
		short s;
		SAFE_READN(&s, sizeof(short));
		sq_pushinteger(v, s);
			  }
		break;
	case 'w': {
		unsigned short w;
		SAFE_READN(&w, sizeof(unsigned short));
		sq_pushinteger(v, w);
			  }
		break;
	case 'c': {
		char c;
		SAFE_READN(&c, sizeof(char));
		sq_pushinteger(v, c);
			  }
		break;
	case 'b': {
		unsigned char c;
		SAFE_READN(&c, sizeof(unsigned char));
		sq_pushinteger(v, c);
			  }
		break;
	case 'f': {
		float f;
		SAFE_READN(&f, sizeof(float));
		sq_pushfloat(v, f);
			  }
		break;
	case 'd': {
		double d;
		SAFE_READN(&d, sizeof(double));
		sq_pushfloat(v, (SQFloat)d);
			  }
		break;
	default:
		return sq_throwerror(v, _SC("invalid format"));
	}
	return 1;
}

int _stream_writestr(HSQUIRRELVM v)
{
	SETUP_STREAM(v);
	const SQChar *str,*res;
	SQInteger trgformat = 'a',len = 0;
	sq_getstring(v,2,&str);
	len = sq_getsize(v,2);
	if(sq_gettop(v)>2)
		sq_getinteger(v,3,&trgformat);
	switch(trgformat)
	{
	case 'a':
#ifdef _UNICODE
		res = sq_getscratchpad(v,len*3);
		len = (SQInteger) wcstombs((char *)res, (const wchar_t*)str, len);
#else
		res = str;
#endif
		self->Write((void *)res,len);
		break;
	case 'u':
#ifdef _UNICODE
		res = str;
#else
		res = sq_getscratchpad(v,len*sizeof(wchar_t));
		len = (SQInteger) mbstowcs((wchar_t*)res, str, len);
#endif
		self->Write((void *)res,len*sizeof(wchar_t));
		break;
	default:
		return sq_throwerror(v,_SC("wrong encoding"));
	}
	
	return 0;
}

int _stream_writeblob(HSQUIRRELVM v)
{
	SQUserPointer data;
	int size;
	SETUP_STREAM(v);
	if(SQ_FAILED(sqstd_getblob(v,2,&data)))
		return sq_throwerror(v,_SC("invalid parameter"));
	size = sqstd_getblobsize(v,2);
	if(self->Write(data,size) != size)
		return sq_throwerror(v,_SC("io error"));
	sq_pushinteger(v,size);
	return 1;
}

int _stream_writen(HSQUIRRELVM v)
{
	SETUP_STREAM(v);
	SQInteger format, ti;
	SQFloat tf;
	sq_getinteger(v, 3, &format);
	switch(format) {
	case 'i': {
		int i;
		sq_getinteger(v, 2, &ti);
		i = ti;
		self->Write(&i, sizeof(int));
			  }
		break;
	case 's': {
		short s;
		sq_getinteger(v, 2, &ti);
		s = ti;
		self->Write(&s, sizeof(short));
			  }
		break;
	case 'w': {
		unsigned short w;
		sq_getinteger(v, 2, &ti);
		w = ti;
		self->Write(&w, sizeof(unsigned short));
			  }
		break;
	case 'c': {
		char c;
		sq_getinteger(v, 2, &ti);
		c = ti;
		self->Write(&c, sizeof(char));
				  }
		break;
	case 'b': {
		unsigned char b;
		sq_getinteger(v, 2, &ti);
		b = ti;
		self->Write(&b, sizeof(unsigned char));
			  }
		break;
	case 'f': {
		float f;
		sq_getfloat(v, 2, &tf);
		f = tf;
		self->Write(&f, sizeof(float));
			  }
		break;
	case 'd': {
		double d;
		sq_getfloat(v, 2, &tf);
		d = tf;
		self->Write(&d, sizeof(double));
			  }
		break;
	default:
		return sq_throwerror(v, _SC("invalid format"));
	}
	return 0;
}

int _stream_seek(HSQUIRRELVM v)
{
	SETUP_STREAM(v);
	SQInteger offset, origin = SQ_SEEK_SET;
	sq_getinteger(v, 2, &offset);
	if(sq_gettop(v) > 2) {
		SQInteger t;
		sq_getinteger(v, 3, &t);
		switch(t) {
			case 'b': origin = SQ_SEEK_SET; break;
			case 'c': origin = SQ_SEEK_CUR; break;
			case 'e': origin = SQ_SEEK_END; break;
			default: return sq_throwerror(v,_SC("invalid origin"));
		}
	}
	sq_pushinteger(v, self->Seek(offset, origin));
	return 1;
}

int _stream_tell(HSQUIRRELVM v)
{
	SETUP_STREAM(v);
	sq_pushinteger(v, self->Tell());
	return 1;
}

int _stream_len(HSQUIRRELVM v)
{
	SETUP_STREAM(v);
	sq_pushinteger(v, self->Len());
	return 1;
}

int _stream_flush(HSQUIRRELVM v)
{
	SETUP_STREAM(v);
	if(!self->Flush())
		sq_pushinteger(v, 1);
	else
		sq_pushnull(v);
	return 1;
}

int _stream_eos(HSQUIRRELVM v)
{
	SETUP_STREAM(v);
	if(self->EOS())
		sq_pushinteger(v, 1);
	else
		sq_pushnull(v);
	return 1;
}

static SQRegFunction _stream_methods[] = {
	_DECL_STREAM_FUNC(readstr,-2,_SC("xnn")),
	_DECL_STREAM_FUNC(readblob,2,_SC("xn")),
	_DECL_STREAM_FUNC(readn,2,_SC("xn")),
	_DECL_STREAM_FUNC(writestr,-2,_SC("xsn")),
	_DECL_STREAM_FUNC(writeblob,-2,_SC("xx")),
	_DECL_STREAM_FUNC(writen,3,_SC("xnn")),
	_DECL_STREAM_FUNC(seek,-2,_SC("xnn")),
	_DECL_STREAM_FUNC(tell,1,_SC("x")),
	_DECL_STREAM_FUNC(len,1,_SC("x")),
	_DECL_STREAM_FUNC(eos,1,_SC("x")),
	_DECL_STREAM_FUNC(flush,1,_SC("x")),
	{0,0}
};

void init_streamclass(HSQUIRRELVM v)
{
	sq_pushregistrytable(v);
	sq_pushstring(v,_SC("std_stream"),-1);
	if(SQ_FAILED(sq_get(v,-2))) {
		sq_pushstring(v,_SC("std_stream"),-1);
		sq_newclass(v,SQFalse);
		sq_settypetag(v,-1,SQSTD_STREAM_TYPE_TAG);
		int i = 0;
		while(_stream_methods[i].name != 0) {
			SQRegFunction &f = _stream_methods[i];
			sq_pushstring(v,f.name,-1);
			sq_newclosure(v,f.f,0);
			sq_setparamscheck(v,f.nparamscheck,f.typemask);
			sq_createslot(v,-3);
			i++;
		}
		sq_createslot(v,-3);
	}
	else {
		sq_pop(v,1); //result
	}
	sq_pop(v,1);
}

SQRESULT declare_stream(HSQUIRRELVM v,SQChar* name,int typetag,SQChar* reg_name,SQRegFunction *methods,SQRegFunction *globals)
{
	if(sq_gettype(v,-1) != OT_TABLE)
		return sq_throwerror(v,_SC("table expected"));
	int top = sq_gettop(v);
	//create delegate
    init_streamclass(v);
	sq_pushregistrytable(v);
	sq_pushstring(v,reg_name,-1);
	sq_pushstring(v,_SC("std_stream"),-1);
	if(SQ_SUCCEEDED(sq_get(v,-3))) {
		sq_newclass(v,SQTrue);
		sq_settypetag(v,-1,typetag);
		int i = 0;
		while(methods[i].name != 0) {
			SQRegFunction &f = methods[i];
			sq_pushstring(v,f.name,-1);
			sq_newclosure(v,f.f,0);
			sq_setparamscheck(v,f.nparamscheck,f.typemask);
			sq_setnativeclosurename(v,-1,f.name);
			sq_createslot(v,-3);
			i++;
		}
		sq_createslot(v,-3);
		sq_pop(v,1);
		
		i = 0;
		while(globals[i].name!=0)
		{
			SQRegFunction &f = globals[i];
			sq_pushstring(v,f.name,-1);
			sq_newclosure(v,f.f,0);
			sq_setparamscheck(v,f.nparamscheck,f.typemask);
			sq_setnativeclosurename(v,-1,f.name);
			sq_createslot(v,-3);
			i++;
		}
		//register the class in the target table
		sq_pushstring(v,name,-1);
		sq_pushregistrytable(v);
		sq_pushstring(v,reg_name,-1);
		sq_get(v,-2);
		sq_remove(v,-2);
		sq_createslot(v,-3);

		sq_settop(v,top);
		return SQ_OK;
	}
	sq_settop(v,top);
	return SQ_ERROR;
}
