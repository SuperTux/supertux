/*
Copyright (c) 2003-2005 Alberto Demichelis

This software is provided 'as-is', without any 
express or implied warranty. In no event will the 
authors be held liable for any damages arising from 
the use of this software.

Permission is granted to anyone to use this software 
for any purpose, including commercial applications, 
and to alter it and redistribute it freely, subject 
to the following restrictions:

		1. The origin of this software must not be 
		misrepresented; you must not claim that 
		you wrote the original software. If you 
		use this software in a product, an 
		acknowledgment in the product 
		documentation would be appreciated but is 
		not required.

		2. Altered source versions must be plainly 
		marked as such, and must not be 
		misrepresented as being the original 
		software.

		3. This notice may not be removed or 
		altered from any source distribution.

*/
#ifndef _SQUIRREL_H_
#define _SQUIRREL_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SQUIRREL_API
#define SQUIRREL_API extern
#endif

typedef float SQFloat;
typedef int SQInteger;
typedef void* SQUserPointer;
typedef unsigned int SQBool;
typedef int SQRESULT;

#define SQTrue	1
#define SQFalse	0


struct SQVM;
struct SQTable;
struct SQArray;
struct SQString;
struct SQClosure;
struct SQGenerator;
struct SQNativeClosure;
struct SQUserData;
struct SQFunctionProto;
struct SQRefCounted;
struct SQClass;
struct SQInstance;
struct SQDelegable;

#ifdef _UNICODE
#define SQUNICODE
#endif

#ifdef SQUNICODE
typedef unsigned short SQChar;
#define _SC(a) L##a
#define	scstrcmp	wcscmp
#define scsprintf	swprintf
#define scstrlen	wcslen
#define scstrtod	wcstod
#define scatoi		_wtoi
#define scstrtoul	wcstoul
#define scvsprintf	vswprintf
#define scstrstr	wcsstr
#define scisspace	iswspace
#define scisdigit	iswdigit
#define scisalpha	iswalpha
#define sciscntrl	iswcntrl
#define scisalnum	iswalnum
#define scprintf	wprintf
#define MAX_CHAR 0xFFFF
#else
typedef char SQChar;
#define _SC(a) a
#define	scstrcmp	strcmp
#define scsprintf	sprintf
#define scstrlen	strlen
#define scstrtod	strtod
#define scatoi		atoi
#define scstrtoul	strtoul
#define scvsprintf	vsprintf
#define scstrstr	strstr
#define scisspace	isspace
#define scisdigit	isdigit
#define sciscntrl	iscntrl
#define scisalpha	isalpha
#define scisalnum	isalnum
#define scprintf	printf
#define MAX_CHAR 0xFF
#endif

#define SQUIRREL_VERSION	_SC("Squirrel 2.0.1 stable")
#define SQUIRREL_COPYRIGHT	_SC("Copyright (C) 2003-2005 Alberto Demichelis")
#define SQUIRREL_AUTHOR		_SC("Alberto Demichelis")

#define SQ_VMSTATE_IDLE			0
#define SQ_VMSTATE_RUNNING		1
#define SQ_VMSTATE_SUSPENDED	2

#define SQUIRREL_EOB 0
#define SQ_BYTECODE_STREAM_TAG	0xFAFA

#define SQOBJECT_REF_COUNTED	0x00800000
#define SQOBJECT_NUMERIC		0x00080000
#define SQOBJECT_DELEGABLE		0x08000000
#define SQOBJECT_CANBEFALSE		0x80000000
//typedef unsigned int SQObjectType;

#define _RT_MASK 0x0000FFFF
#define _RAW_TYPE(type) (type&_RT_MASK)

#define _RT_NULL			0x00000000
#define _RT_INTEGER			0x00000001
#define _RT_FLOAT			0x00000002
#define _RT_BOOL			0x00000004
#define _RT_STRING			0x00000008
#define _RT_TABLE			0x00000010
#define _RT_ARRAY			0x00000020
#define _RT_USERDATA		0x00000040
#define _RT_CLOSURE			0x00000080
#define _RT_NATIVECLOSURE	0x00000100
#define _RT_GENERATOR		0x00000200
#define _RT_USERPOINTER		0x00000400
#define _RT_THREAD			0x00000800
#define _RT_FUNCPROTO		0x00001000
#define _RT_CLASS			0x00002000
#define _RT_INSTANCE		0x00004000

typedef enum {
	OT_NULL =			(_RT_NULL|SQOBJECT_CANBEFALSE),
	OT_INTEGER =		(_RT_INTEGER|SQOBJECT_NUMERIC|SQOBJECT_CANBEFALSE),
	OT_FLOAT =			(_RT_FLOAT|SQOBJECT_NUMERIC|SQOBJECT_CANBEFALSE),
	OT_BOOL =			(_RT_BOOL|SQOBJECT_CANBEFALSE),
	OT_STRING =			(_RT_STRING|SQOBJECT_REF_COUNTED),
	OT_TABLE =			(_RT_TABLE|SQOBJECT_REF_COUNTED|SQOBJECT_DELEGABLE),
	OT_ARRAY =			(_RT_ARRAY|SQOBJECT_REF_COUNTED),
	OT_USERDATA =		(_RT_USERDATA|SQOBJECT_REF_COUNTED|SQOBJECT_DELEGABLE),
	OT_CLOSURE =		(_RT_CLOSURE|SQOBJECT_REF_COUNTED),
	OT_NATIVECLOSURE =	(_RT_NATIVECLOSURE|SQOBJECT_REF_COUNTED),
	OT_GENERATOR =		(_RT_GENERATOR|SQOBJECT_REF_COUNTED),
	OT_USERPOINTER =	_RT_USERPOINTER,
	OT_THREAD =			(_RT_THREAD|SQOBJECT_REF_COUNTED) ,
	OT_FUNCPROTO =		(_RT_FUNCPROTO|SQOBJECT_REF_COUNTED), //internal usage only
	OT_CLASS =			(_RT_CLASS|SQOBJECT_REF_COUNTED),
	OT_INSTANCE =		(_RT_INSTANCE|SQOBJECT_REF_COUNTED|SQOBJECT_DELEGABLE)
}SQObjectType;

#define ISREFCOUNTED(t) (t&SQOBJECT_REF_COUNTED)


typedef union tagSQObjectValue
{
	struct SQTable *pTable;
	struct SQArray *pArray;
	struct SQClosure *pClosure;
	struct SQGenerator *pGenerator;
	struct SQNativeClosure *pNativeClosure;
	struct SQString *pString;
	struct SQUserData *pUserData;
	SQInteger nInteger;
	SQFloat fFloat;
	SQUserPointer pUserPointer;
	struct SQFunctionProto *pFunctionProto;
	struct SQRefCounted *pRefCounted;
	struct SQDelegable *pDelegable;
	struct SQVM *pThread;
	struct SQClass *pClass;
	struct SQInstance *pInstance;
}SQObjectValue;


typedef struct tagSQObject
{
	SQObjectValue _unVal;
	SQObjectType _type;
}SQObject;

typedef struct tagSQStackInfos{
	const SQChar* funcname;
	const SQChar* source;
	int line;
}SQStackInfos;

typedef struct SQVM* HSQUIRRELVM;
typedef SQObject HSQOBJECT;
typedef int (*SQFUNCTION)(HSQUIRRELVM);
typedef int (*SQRELEASEHOOK)(SQUserPointer,int size);
typedef void (*SQCOMPILERERROR)(HSQUIRRELVM,const SQChar * /*desc*/,const SQChar * /*source*/,int /*line*/,int /*column*/);
typedef void (*SQPRINTFUNCTION)(HSQUIRRELVM,const SQChar * ,...);

typedef int (*SQWRITEFUNC)(SQUserPointer,SQUserPointer,int);
typedef int (*SQREADFUNC)(SQUserPointer,SQUserPointer,int);

typedef SQInteger (*SQLEXREADFUNC)(SQUserPointer);

typedef struct tagSQRegFunction{
	const SQChar *name;
	SQFUNCTION f;
	int nparamscheck;
	const SQChar *typemask;
}SQRegFunction;

/*vm*/
SQUIRREL_API HSQUIRRELVM sq_open(int initialstacksize);
SQUIRREL_API HSQUIRRELVM sq_newthread(HSQUIRRELVM friendvm, int initialstacksize);
SQUIRREL_API void sq_seterrorhandler(HSQUIRRELVM v);
SQUIRREL_API void sq_close(HSQUIRRELVM v);
SQUIRREL_API void sq_setforeignptr(HSQUIRRELVM v,SQUserPointer p);
SQUIRREL_API SQUserPointer sq_getforeignptr(HSQUIRRELVM v);
SQUIRREL_API void sq_setprintfunc(HSQUIRRELVM v, SQPRINTFUNCTION printfunc);
SQUIRREL_API SQPRINTFUNCTION sq_getprintfunc(HSQUIRRELVM v);
SQUIRREL_API SQRESULT sq_suspendvm(HSQUIRRELVM v);
SQUIRREL_API SQRESULT sq_wakeupvm(HSQUIRRELVM v,SQBool resumedret,SQBool retval);
SQUIRREL_API int sq_getvmstate(HSQUIRRELVM v);

/*compiler*/
SQUIRREL_API SQRESULT sq_compile(HSQUIRRELVM v,SQLEXREADFUNC read,SQUserPointer p,const SQChar *sourcename,SQBool raiseerror);
SQUIRREL_API SQRESULT sq_compilebuffer(HSQUIRRELVM v,const SQChar *s,int size,const SQChar *sourcename,SQBool raiseerror);
SQUIRREL_API void sq_enabledebuginfo(HSQUIRRELVM v, SQBool debuginfo);
SQUIRREL_API void sq_setcompilererrorhandler(HSQUIRRELVM v,SQCOMPILERERROR f);

/*stack operations*/
SQUIRREL_API void sq_push(HSQUIRRELVM v,int idx);
SQUIRREL_API void sq_pop(HSQUIRRELVM v,int nelemstopop);
SQUIRREL_API void sq_remove(HSQUIRRELVM v,int idx);
SQUIRREL_API int sq_gettop(HSQUIRRELVM v);
SQUIRREL_API void sq_settop(HSQUIRRELVM v,int newtop);
SQUIRREL_API void sq_reservestack(HSQUIRRELVM v,int nsize);
SQUIRREL_API int sq_cmp(HSQUIRRELVM v);
SQUIRREL_API void sq_move(HSQUIRRELVM dest,HSQUIRRELVM src,int idx);

/*object creation handling*/
SQUIRREL_API SQUserPointer sq_newuserdata(HSQUIRRELVM v,unsigned int size);
SQUIRREL_API void sq_newtable(HSQUIRRELVM v);
SQUIRREL_API void sq_newarray(HSQUIRRELVM v,int size);
SQUIRREL_API void sq_newclosure(HSQUIRRELVM v,SQFUNCTION func,unsigned int nfreevars);
SQUIRREL_API SQRESULT sq_setparamscheck(HSQUIRRELVM v,int nparamscheck,const SQChar *typemask);
SQUIRREL_API void sq_pushstring(HSQUIRRELVM v,const SQChar *s,int len);
SQUIRREL_API void sq_pushfloat(HSQUIRRELVM v,SQFloat f);
SQUIRREL_API void sq_pushinteger(HSQUIRRELVM v,SQInteger n);
SQUIRREL_API void sq_pushbool(HSQUIRRELVM v,SQBool b);
SQUIRREL_API void sq_pushuserpointer(HSQUIRRELVM v,SQUserPointer p);
SQUIRREL_API void sq_pushnull(HSQUIRRELVM v);
SQUIRREL_API SQObjectType sq_gettype(HSQUIRRELVM v,int idx);
SQUIRREL_API SQInteger sq_getsize(HSQUIRRELVM v,int idx);
SQUIRREL_API SQRESULT sq_getstring(HSQUIRRELVM v,int idx,const SQChar **c);
SQUIRREL_API SQRESULT sq_getinteger(HSQUIRRELVM v,int idx,SQInteger *i);
SQUIRREL_API SQRESULT sq_getfloat(HSQUIRRELVM v,int idx,SQFloat *f);
SQUIRREL_API SQRESULT sq_getbool(HSQUIRRELVM v,int idx,SQBool *b);
SQUIRREL_API SQRESULT sq_getthread(HSQUIRRELVM v,int idx,HSQUIRRELVM *thread);
SQUIRREL_API SQRESULT sq_getuserpointer(HSQUIRRELVM v,int idx,SQUserPointer *p);
SQUIRREL_API SQRESULT sq_getuserdata(HSQUIRRELVM v,int idx,SQUserPointer *p,unsigned int *typetag);
SQUIRREL_API SQRESULT sq_settypetag(HSQUIRRELVM v,int idx,unsigned int typetag);
SQUIRREL_API SQRESULT sq_gettypetag(HSQUIRRELVM v,int idx,unsigned int *typetag);
SQUIRREL_API void sq_setreleasehook(HSQUIRRELVM v,int idx,SQRELEASEHOOK hook);
SQUIRREL_API SQChar *sq_getscratchpad(HSQUIRRELVM v,int minsize);
SQUIRREL_API SQRESULT sq_getclosureinfo(HSQUIRRELVM v,int idx,unsigned int *nparams,unsigned int *nfreevars);
SQUIRREL_API SQRESULT sq_setnativeclosurename(HSQUIRRELVM v,int idx,const SQChar *name);
SQUIRREL_API SQRESULT sq_setinstanceup(HSQUIRRELVM v, int idx, SQUserPointer p);
SQUIRREL_API SQRESULT sq_getinstanceup(HSQUIRRELVM v, int idx, SQUserPointer *p,unsigned int typetag);
SQUIRREL_API SQRESULT sq_newclass(HSQUIRRELVM v,SQBool hasbase);
SQUIRREL_API SQRESULT sq_createinstance(HSQUIRRELVM v,int idx);
SQUIRREL_API SQRESULT sq_setattributes(HSQUIRRELVM v,int idx);
SQUIRREL_API SQRESULT sq_getattributes(HSQUIRRELVM v,int idx);
SQUIRREL_API SQRESULT sq_getclass(HSQUIRRELVM v,int idx);

/*object manipulation*/
SQUIRREL_API void sq_pushroottable(HSQUIRRELVM v);
SQUIRREL_API void sq_pushregistrytable(HSQUIRRELVM v);
SQUIRREL_API SQRESULT sq_setroottable(HSQUIRRELVM v);
SQUIRREL_API SQRESULT sq_createslot(HSQUIRRELVM v,int idx);
SQUIRREL_API SQRESULT sq_deleteslot(HSQUIRRELVM v,int idx,SQBool pushval);
SQUIRREL_API SQRESULT sq_set(HSQUIRRELVM v,int idx);
SQUIRREL_API SQRESULT sq_get(HSQUIRRELVM v,int idx);
SQUIRREL_API SQRESULT sq_rawget(HSQUIRRELVM v,int idx);
SQUIRREL_API SQRESULT sq_rawset(HSQUIRRELVM v,int idx);
SQUIRREL_API SQRESULT sq_rawdeleteslot(HSQUIRRELVM v,int idx,SQBool pushval);
SQUIRREL_API SQRESULT sq_arrayappend(HSQUIRRELVM v,int idx);
SQUIRREL_API SQRESULT sq_arraypop(HSQUIRRELVM v,int idx,SQBool pushval); 
SQUIRREL_API SQRESULT sq_arrayresize(HSQUIRRELVM v,int idx,int newsize); 
SQUIRREL_API SQRESULT sq_arrayreverse(HSQUIRRELVM v,int idx); 
SQUIRREL_API SQRESULT sq_setdelegate(HSQUIRRELVM v,int idx);
SQUIRREL_API SQRESULT sq_getdelegate(HSQUIRRELVM v,int idx);
SQUIRREL_API SQRESULT sq_clone(HSQUIRRELVM v,int idx);
SQUIRREL_API SQRESULT sq_setfreevariable(HSQUIRRELVM v,int idx,unsigned int nval);
SQUIRREL_API SQRESULT sq_next(HSQUIRRELVM v,int idx);

/*calls*/
SQUIRREL_API SQRESULT sq_call(HSQUIRRELVM v,int params,SQBool retval);
SQUIRREL_API SQRESULT sq_resume(HSQUIRRELVM v,SQBool retval);
SQUIRREL_API const SQChar *sq_getlocal(HSQUIRRELVM v,unsigned int level,unsigned int idx);
SQUIRREL_API SQRESULT sq_throwerror(HSQUIRRELVM v,const SQChar *err);
SQUIRREL_API void sq_reseterror(HSQUIRRELVM v);
SQUIRREL_API void sq_getlasterror(HSQUIRRELVM v);

/*raw object handling*/
SQUIRREL_API SQRESULT sq_getstackobj(HSQUIRRELVM v,int idx,HSQOBJECT *po);
SQUIRREL_API void sq_pushobject(HSQUIRRELVM v,HSQOBJECT obj);
SQUIRREL_API void sq_addref(HSQUIRRELVM v,HSQOBJECT *po);
SQUIRREL_API SQBool sq_release(HSQUIRRELVM v,HSQOBJECT *po);
SQUIRREL_API void sq_resetobject(HSQOBJECT *po);
SQUIRREL_API const SQChar *sq_objtostring(HSQOBJECT *o);
SQUIRREL_API SQInteger sq_objtointeger(HSQOBJECT *o);
SQUIRREL_API SQFloat sq_objtofloat(HSQOBJECT *o);

/*GC*/
SQUIRREL_API int sq_collectgarbage(HSQUIRRELVM v);

/*serialization*/
SQUIRREL_API SQRESULT sq_writeclosure(HSQUIRRELVM vm,SQWRITEFUNC writef,SQUserPointer up);
SQUIRREL_API SQRESULT sq_readclosure(HSQUIRRELVM vm,SQREADFUNC readf,SQUserPointer up);

/*mem allocation*/
SQUIRREL_API void *sq_malloc(unsigned int size);
SQUIRREL_API void *sq_realloc(void* p,unsigned int oldsize,unsigned int newsize);
SQUIRREL_API void sq_free(void *p,unsigned int size);

/*debug*/
SQUIRREL_API SQRESULT sq_stackinfos(HSQUIRRELVM v,int level,SQStackInfos *si);
SQUIRREL_API void sq_setdebughook(HSQUIRRELVM v);

/*UTILITY MACRO*/
#define sq_isnumeric(o) ((o)._type&SQOBJECT_NUMERIC)
#define sq_istable(o) ((o)._type==OT_TABLE)
#define sq_isarray(o) ((o)._type==OT_ARRAY)
#define sq_isfunction(o) ((o)._type==OT_FUNCPROTO)
#define sq_isclosure(o) ((o)._type==OT_CLOSURE)
#define sq_isgenerator(o) ((o)._type==OT_GENERATOR)
#define sq_isnativeclosure(o) ((o)._type==OT_NATIVECLOSURE)
#define sq_isstring(o) ((o)._type==OT_STRING)
#define sq_isinteger(o) ((o)._type==OT_INTEGER)
#define sq_isfloat(o) ((o)._type==OT_FLOAT)
#define sq_isuserpointer(o) ((o)._type==OT_USERPOINTER)
#define sq_isuserdata(o) ((o)._type==OT_USERDATA)
#define sq_isthread(o) ((o)._type==OT_THREAD)
#define sq_isnull(o) ((o)._type==OT_NULL)
#define sq_isclass(o) ((o)._type==OT_CLASS)
#define sq_isinstance(o) ((o)._type==OT_INSTANCE)
#define sq_isbool(o) ((o)._type==OT_BOOL)
#define sq_type(o) ((o)._type)

#define SQ_OK (0)
#define SQ_ERROR (-1)

#define SQ_FAILED(res) (res<0)
#define SQ_SUCCEEDED(res) (res>=0)

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*_SQUIRREL_H_*/
