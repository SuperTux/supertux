/*
	see copyright notice in squirrel.h
*/
#include "sqpcheader.h"
#include "sqvm.h"
#include "sqstring.h"
#include "sqtable.h"
#include "sqarray.h"
#include "sqfuncproto.h"
#include "sqclosure.h"
#include "sqclass.h"
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

bool str2num(const SQChar *s,SQObjectPtr &res)
{
	SQChar *end;
	if(scstrstr(s,_SC("."))){
		SQFloat r = SQFloat(scstrtod(s,&end));
		if(s == end) return false;
		while (scisspace((*end)) ) end++;
		if (*end != _SC('\0')) return false;
		res = r;
		return true;
	}
	else{
		const SQChar *t = s;
		while(*t != _SC('\0')) if(!scisdigit(*t++)) return false;
		res = SQInteger(scatoi(s));
		return true;
	}
}

#ifndef NO_GARBAGE_COLLECTOR
static int base_collectgarbage(HSQUIRRELVM v)
{
	sq_pushinteger(v, sq_collectgarbage(v));
	return 1;
}
#endif

static int base_getroottable(HSQUIRRELVM v)
{
	v->Push(v->_roottable);
	return 1;
}

static int base_setroottable(HSQUIRRELVM v)
{
	SQObjectPtr &o=stack_get(v,2);
	if(SQ_FAILED(sq_setroottable(v))) return SQ_ERROR;
	v->Push(o);
	return 1;
}

static int base_seterrorhandler(HSQUIRRELVM v)
{
	sq_seterrorhandler(v);
	return 0;
}

static int base_setdebughook(HSQUIRRELVM v)
{
	sq_setdebughook(v);
	return 0;
}

static int base_enabledebuginfo(HSQUIRRELVM v)
{
	SQObjectPtr &o=stack_get(v,2);
	sq_enabledebuginfo(v,(type(o) != OT_NULL)?1:0);
	return 0;
}

static int base_getstackinfos(HSQUIRRELVM v)
{
	SQInteger level;
	SQStackInfos si;
	int seq = 0;
	const SQChar *name = NULL;
	sq_getinteger(v, -1, &level);
	if (SQ_SUCCEEDED(sq_stackinfos(v, level, &si)))
	{
		const SQChar *fn = _SC("unknown");
		const SQChar *src = _SC("unknown");
		if(si.funcname)fn = si.funcname;
		if(si.source)src = si.source;
		sq_newtable(v);
		sq_pushstring(v, _SC("func"), -1);
		sq_pushstring(v, fn, -1);
		sq_createslot(v, -3);
		sq_pushstring(v, _SC("src"), -1);
		sq_pushstring(v, src, -1);
		sq_createslot(v, -3);
		sq_pushstring(v, _SC("line"), -1);
		sq_pushinteger(v, si.line);
		sq_createslot(v, -3);
		sq_pushstring(v, _SC("locals"), -1);
		sq_newtable(v);
		seq=0;
		while (name = sq_getlocal(v, level, seq)) {
			sq_pushstring(v, name, -1);
			sq_push(v, -2);
			sq_createslot(v, -4);
			sq_pop(v, 1);
			seq++;
		}
		sq_createslot(v, -3);
		return 1;
	}

	return 0;
}

static int base_assert(HSQUIRRELVM v)
{
	if(v->IsFalse(stack_get(v,2))){
		return sq_throwerror(v,_SC("assertion failed"));
	}
	return 0;
}

static int get_slice_params(HSQUIRRELVM v,int &sidx,int &eidx,SQObjectPtr &o)
{
	int top = sq_gettop(v);
	sidx=0;
	eidx=0;
	o=stack_get(v,1);
	SQObjectPtr &start=stack_get(v,2);
	if(type(start)!=OT_NULL && sq_isnumeric(start)){
		sidx=tointeger(start);
	}
	if(top>2){
		SQObjectPtr &end=stack_get(v,3);
		if(sq_isnumeric(end)){
			eidx=tointeger(end);
		}
	}
	else {
		eidx = sq_getsize(v,1);
	}
	return 1;
}

static int base_print(HSQUIRRELVM v)
{
	SQObjectPtr &o=stack_get(v,2);
	switch(type(o)){
	case OT_STRING:
		if(_ss(v)->_printfunc) _ss(v)->_printfunc(v,_SC("%s"),_stringval(o));
		break;
	case OT_INTEGER:
		if(_ss(v)->_printfunc) _ss(v)->_printfunc(v,_SC("%d"),_integer(o));
		break;
	case OT_FLOAT:
		if(_ss(v)->_printfunc) _ss(v)->_printfunc(v,_SC("%.14g"),_float(o));
		break;
	default:{
		SQObjectPtr tname;
		v->TypeOf(o,tname);
		if(_ss(v)->_printfunc) _ss(v)->_printfunc(v,_SC("(%s)"),_stringval(tname));
		}
		break;
	}
	return 0;
}

static int base_compilestring(HSQUIRRELVM v)
{
	int nargs=sq_gettop(v);
	const SQChar *src=NULL,*name=_SC("unnamedbuffer");
	SQInteger size;
	sq_getstring(v,2,&src);
	size=sq_getsize(v,2);
	if(nargs>2){
		sq_getstring(v,3,&name);
	}
	if(SQ_SUCCEEDED(sq_compilebuffer(v,src,size,name,SQFalse)))
		return 1;
	else
		return SQ_ERROR;
}

static int base_newthread(HSQUIRRELVM v)
{
	SQObjectPtr &func = stack_get(v,2);
	int stksize = (_funcproto(_closure(func)->_function)->_stacksize << 1) +2;
	HSQUIRRELVM newv = sq_newthread(v, (stksize < MIN_STACK_OVERHEAD + 2)? MIN_STACK_OVERHEAD + 2 : stksize);
	sq_move(newv,v,-2);
	return 1;
}

static int base_suspend(HSQUIRRELVM v)
{
	return sq_suspendvm(v);
}

static int base_array(HSQUIRRELVM v)
{
	SQArray *a;
	SQObject &size = stack_get(v,2);
	if(sq_gettop(v) > 2) {
		a = SQArray::Create(_ss(v),0);
		a->Resize(tointeger(size),stack_get(v,3));
	}
	else {
		a = SQArray::Create(_ss(v),tointeger(size));
	}
	v->Push(a);
	return 1;
}

static int base_type(HSQUIRRELVM v)
{
	SQObjectPtr &o = stack_get(v,2);
	v->Push(SQString::Create(_ss(v),GetTypeName(o),-1));
	return 1;
}

static SQRegFunction base_funcs[]={
	//generic
	{_SC("seterrorhandler"),base_seterrorhandler,2, NULL},
	{_SC("setdebughook"),base_setdebughook,2, NULL},
	{_SC("enabledebuginfo"),base_enabledebuginfo,2, NULL},
	{_SC("getstackinfos"),base_getstackinfos,2, _SC(".n")},
	{_SC("getroottable"),base_getroottable,1, NULL},
	{_SC("setroottable"),base_setroottable,2, NULL},
	{_SC("assert"),base_assert,2, NULL},
	{_SC("print"),base_print,2, NULL},
	{_SC("compilestring"),base_compilestring,-2, _SC(".ss")},
	{_SC("newthread"),base_newthread,2, _SC(".c")},
	{_SC("suspend"),base_suspend,-1, NULL},
	{_SC("array"),base_array,-2, _SC(".n")},
	{_SC("type"),base_type,2, NULL},
#ifndef NO_GARBAGE_COLLECTOR
	{_SC("collectgarbage"),base_collectgarbage,1, _SC("t")},
#endif
	{0,0}
};

void sq_base_register(HSQUIRRELVM v)
{
	int i=0;
	sq_pushroottable(v);
	while(base_funcs[i].name!=0) {
		sq_pushstring(v,base_funcs[i].name,-1);
		sq_newclosure(v,base_funcs[i].f,0);
		sq_setnativeclosurename(v,-1,base_funcs[i].name);
		sq_setparamscheck(v,base_funcs[i].nparamscheck,base_funcs[i].typemask);
		sq_createslot(v,-3);
		i++;
	}
	sq_pushstring(v,_SC("_charsize_"),-1);
	sq_pushinteger(v,sizeof(SQChar));
	sq_createslot(v,-3);
	sq_pop(v,1);
}

static int default_delegate_len(HSQUIRRELVM v)
{
	v->Push(SQInteger(sq_getsize(v,1)));
	return 1;
}

static int default_delegate_tofloat(HSQUIRRELVM v)
{
	SQObjectPtr &o=stack_get(v,1);
	switch(type(o)){
	case OT_STRING:{
		SQObjectPtr res;
		if(str2num(_stringval(o),res)){
			v->Push(SQObjectPtr(tofloat(res)));
			break;
		}}
	case OT_INTEGER:case OT_FLOAT:
		v->Push(SQObjectPtr(tofloat(o)));
		break;
	case OT_BOOL:
		v->Push(SQObjectPtr((SQFloat)(_integer(o)?1:0)));
		break;
	default:
		v->Push(_null_);
		break;
	}
	return 1;
}

static int default_delegate_tointeger(HSQUIRRELVM v)
{
	SQObjectPtr &o=stack_get(v,1);
	switch(type(o)){
	case OT_STRING:{
		SQObjectPtr res;
		if(str2num(_stringval(o),res)){
			v->Push(SQObjectPtr(tointeger(res)));
			break;
		}}
	case OT_INTEGER:case OT_FLOAT:
		v->Push(SQObjectPtr(tointeger(o)));
		break;
	case OT_BOOL:
		v->Push(SQObjectPtr(_integer(o)?1:0));
		break;
	default:
		v->Push(_null_);
		break;
	}
	return 1;
}

static int default_delegate_tostring(HSQUIRRELVM v)
{
	SQObjectPtr &o=stack_get(v,1);
	switch(type(o)){
	case OT_STRING:
		v->Push(o);
		break;
	case OT_INTEGER:
		scsprintf(_ss(v)->GetScratchPad(rsl(NUMBER_MAX_CHAR+1)),_SC("%d"),_integer(o));
		v->Push(SQString::Create(_ss(v),_ss(v)->GetScratchPad(-1)));
		break;
	case OT_FLOAT:
		scsprintf(_ss(v)->GetScratchPad(rsl(NUMBER_MAX_CHAR+1)),_SC("%.14g"),_float(o));
		v->Push(SQString::Create(_ss(v),_ss(v)->GetScratchPad(-1)));
		break;
	case OT_BOOL:
		v->Push(SQObjectPtr(SQString::Create(_ss(v),_integer(o)?_SC("true"):_SC("false"))));
		break;
	default:
		v->Push(_null_);
		break;
	}
	return 1;
}

static int number_delegate_tochar(HSQUIRRELVM v)
{
	SQObject &o=stack_get(v,1);
	SQChar c=tointeger(o);
	v->Push(SQString::Create(_ss(v),(const SQChar *)&c,1));
	return 1;
}


/////////////////////////////////////////////////////////////////
//TABLE DEFAULT DELEGATE

static int table_rawdelete(HSQUIRRELVM v)
{
	if(SQ_FAILED(sq_rawdeleteslot(v,1,SQTrue)))
		return SQ_ERROR;
	return 1;
}


static int container_rawexists(HSQUIRRELVM v)
{
	if(SQ_SUCCEEDED(sq_rawget(v,-2))) {
		sq_pushbool(v,SQTrue);
		return 1;
	}
	sq_pushbool(v,SQFalse);
	return 1;
}

static int table_rawset(HSQUIRRELVM v)
{
	return sq_rawset(v,-3);
}


static int table_rawget(HSQUIRRELVM v)
{
	return SQ_SUCCEEDED(sq_rawget(v,-2))?1:SQ_ERROR;
}

SQRegFunction SQSharedState::_table_default_delegate_funcz[]={
	{_SC("len"),default_delegate_len,1, _SC("t")},
	{_SC("rawget"),table_rawget,2, _SC("t")},
	{_SC("rawset"),table_rawset,3, _SC("t")},
	{_SC("rawdelete"),table_rawdelete,2, _SC("t")},
	{_SC("rawin"),container_rawexists,2, _SC("t")},
	{0,0}
};

//ARRAY DEFAULT DELEGATE///////////////////////////////////////

static int array_append(HSQUIRRELVM v)
{
	return sq_arrayappend(v,-2);
}

static int array_extend(HSQUIRRELVM v)
{
	_array(stack_get(v,1))->Extend(_array(stack_get(v,2)));
	return 0;
}

static int array_reverse(HSQUIRRELVM v)
{
	return sq_arrayreverse(v,-1);
}

static int array_pop(HSQUIRRELVM v)
{
	return SQ_SUCCEEDED(sq_arraypop(v,1,SQTrue))?1:SQ_ERROR;
}

static int array_top(HSQUIRRELVM v)
{
	SQObject &o=stack_get(v,1);
	if(_array(o)->Size()>0){
		v->Push(_array(o)->Top());
		return 1;
	}
	else return sq_throwerror(v,_SC("top() on a empty array"));
}

static int array_insert(HSQUIRRELVM v)
{
	SQObject &o=stack_get(v,1);
	SQObject &idx=stack_get(v,2);
	SQObject &val=stack_get(v,3);
	_array(o)->Insert(idx,val);
	return 0;
}

static int array_remove(HSQUIRRELVM v)
{
	SQObject &o = stack_get(v, 1);
	SQObject &idx = stack_get(v, 2);
	if(!sq_isnumeric(idx)) return sq_throwerror(v, _SC("wrong type"));
	SQObjectPtr val;
	if(_array(o)->Get(tointeger(idx), val)) {
		_array(o)->Remove(tointeger(idx));
		v->Push(val);
		return 1;
	}
	return sq_throwerror(v, _SC("idx out of range"));
}

static int array_resize(HSQUIRRELVM v)
{
	SQObject &o = stack_get(v, 1);
	SQObject &nsize = stack_get(v, 2);
	SQObjectPtr fill;
	if(sq_isnumeric(nsize)) {
		if(sq_gettop(v) > 2)
			fill = stack_get(v, 3);
		_array(o)->Resize(tointeger(nsize),fill);
		return 0;
	}
	return sq_throwerror(v, _SC("size must be a number"));
}


//QSORT ala Sedgewick
bool _qsort_compare(HSQUIRRELVM v,SQObjectPtr &arr,SQObjectPtr &a,SQObjectPtr &b,int func,int &ret)
{
	if(func < 0) {
		if(!v->ObjCmp(a,b,ret)) return false;
	}
	else {
		int top = sq_gettop(v);
		sq_push(v, func);
		sq_pushroottable(v);
		v->Push(a);
		v->Push(b);
		if(SQ_FAILED(sq_call(v, 3, SQTrue))) {
			v->Raise_Error(_SC("compare func failed"));
			return false;
		}
		sq_getinteger(v, -1, &ret);
		sq_settop(v, top);
		return true;
	}
	return true;
}
//QSORT ala Sedgewick
bool _qsort(HSQUIRRELVM v,SQObjectPtr &arr, int l, int r,int func)
{
	int i, j;
	SQArray *a=_array(arr);
	SQObjectPtr pivot,t;
	if( l < r ){
		pivot = a->_values[l];
		i = l; j = r+1;
		while(1){
			int ret;
			do { 
				++i; 
				if(i > r) break;
				if(!_qsort_compare(v,arr,a->_values[i],pivot,func,ret))
					return false;
			} while( ret <= 0);
			do {
				--j; 
				if(!_qsort_compare(v,arr,a->_values[j],pivot,func,ret))
					return false;
			}
			while( ret > 0 );
			if( i >= j ) break;
			t = a->_values[i]; a->_values[i] = a->_values[j]; a->_values[j] = t;
		}
		t = a->_values[l]; a->_values[l] = a->_values[j]; a->_values[j] = t;
		if(!_qsort( v, arr, l, j-1,func)) return false;
		if(!_qsort( v, arr, j+1, r,func)) return false;
	}
	return true;
}

static int array_sort(HSQUIRRELVM v)
{
	//SQ_TRY {
	int func = -1;
	SQObjectPtr &o = stack_get(v,1);
	SQObject &funcobj = stack_get(v,2);
	if(_array(o)->Size() > 1) {
		if(type(funcobj) == OT_CLOSURE || type(funcobj) == OT_NATIVECLOSURE) func = 2;
		if(!_qsort(v, o, 0, _array(o)->Size()-1, func))
			return SQ_ERROR;

	}
	return 0;
}
static int array_slice(HSQUIRRELVM v)
{
	int sidx,eidx;
	SQObjectPtr o;
	if(get_slice_params(v,sidx,eidx,o)==-1)return -1;
	if(sidx<0)sidx=_array(o)->Size()+sidx;
	if(eidx<0)eidx=_array(o)->Size()+eidx;
	if(eidx <= sidx)return sq_throwerror(v,_SC("wrong indexes"));
	SQArray *arr=SQArray::Create(_ss(v),eidx-sidx);
	SQObjectPtr t;
	int count=0;
	for(int i=sidx;i<eidx;i++){
		_array(o)->Get(i,t);
		arr->Set(count++,t);
	}
	v->Push(arr);
	return 1;
	
}

SQRegFunction SQSharedState::_array_default_delegate_funcz[]={
	{_SC("len"),default_delegate_len,1, _SC("a")},
	{_SC("append"),array_append,2, _SC("a")},
	{_SC("extend"),array_extend,2, _SC("aa")},
	{_SC("push"),array_append,2, _SC("a")},
	{_SC("pop"),array_pop,1, _SC("a")},
	{_SC("top"),array_top,1, _SC("a")},
	{_SC("insert"),array_insert,3, _SC("an")},
	{_SC("remove"),array_remove,2, _SC("an")},
	{_SC("resize"),array_resize,-2, _SC("an")},
	{_SC("reverse"),array_reverse,1, _SC("a")},
	{_SC("sort"),array_sort,-1, _SC("ac")},
	{_SC("slice"),array_slice,-1, _SC("ann")},
	{0,0}
};

//STRING DEFAULT DELEGATE//////////////////////////
static int string_slice(HSQUIRRELVM v)
{
	int sidx,eidx;
	SQObjectPtr o;
	if(SQ_FAILED(get_slice_params(v,sidx,eidx,o)))return -1;
	if(sidx<0)sidx=_string(o)->_len+sidx;
	if(eidx<0)eidx=_string(o)->_len+eidx;
	if(eidx<sidx)
		return sq_throwerror(v,_SC("wrong indexes"));
	v->Push(SQString::Create(_ss(v),&_stringval(o)[sidx],eidx-sidx));
	return 1;
}

static int string_find(HSQUIRRELVM v)
{
	int top,start_idx=0;
	const SQChar *str,*substr,*ret;
	if(((top=sq_gettop(v))>1) && SQ_SUCCEEDED(sq_getstring(v,1,&str)) && SQ_SUCCEEDED(sq_getstring(v,2,&substr))){
		if(top>2)sq_getinteger(v,3,&start_idx);
		if((sq_getsize(v,1)>start_idx) && (start_idx>=0)){
			ret=scstrstr(&str[start_idx],substr);
			if(ret){
				sq_pushinteger(v,(int)(ret-str));
				return 1;
			}
		}
		return 0;
	}
	return sq_throwerror(v,_SC("invalid param"));
}

#define STRING_TOFUNCZ(func) static int string_##func(HSQUIRRELVM v) \
{ \
	SQObject str=stack_get(v,1); \
	int len=_string(str)->_len; \
	const SQChar *sThis=_stringval(str); \
	SQChar *sNew=(_ss(v)->GetScratchPad(rsl(len))); \
	for(int i=0;i<len;i++) sNew[i]=func(sThis[i]); \
	v->Push(SQString::Create(_ss(v),sNew,len)); \
	return 1; \
}


STRING_TOFUNCZ(tolower)
STRING_TOFUNCZ(toupper)

SQRegFunction SQSharedState::_string_default_delegate_funcz[]={
	{_SC("len"),default_delegate_len,1, _SC("s")},
	{_SC("tointeger"),default_delegate_tointeger,1, _SC("s")},
	{_SC("tofloat"),default_delegate_tofloat,1, _SC("s")},
	{_SC("tostring"),default_delegate_tostring,1, _SC("s")},
	{_SC("slice"),string_slice,-1, _SC(" s n  n")},
	{_SC("find"),string_find,-2, _SC("s s n ")},
	{_SC("tolower"),string_tolower,1, _SC("s")},
	{_SC("toupper"),string_toupper,1, _SC("s")},
	{0,0}
};

//INTEGER DEFAULT DELEGATE//////////////////////////
SQRegFunction SQSharedState::_number_default_delegate_funcz[]={
	{_SC("tointeger"),default_delegate_tointeger,1, _SC("n|b")},
	{_SC("tofloat"),default_delegate_tofloat,1, _SC("n|b")},
	{_SC("tostring"),default_delegate_tostring,1, _SC("n|b")},
	{_SC("tochar"),number_delegate_tochar,1, _SC("n|b")},
	{0,0}
};

//CLOSURE DEFAULT DELEGATE//////////////////////////
static int closure_call(HSQUIRRELVM v)
{
	return SQ_SUCCEEDED(sq_call(v,sq_gettop(v)-1,SQTrue))?1:SQ_ERROR;
}

static int closure_acall(HSQUIRRELVM v)
{
	SQArray *aparams=_array(stack_get(v,2));
	int nparams=aparams->Size();
	v->Push(stack_get(v,1));
	for(int i=0;i<nparams;i++)v->Push(aparams->_values[i]);
	return SQ_SUCCEEDED(sq_call(v,nparams,SQTrue))?1:SQ_ERROR;
}

SQRegFunction SQSharedState::_closure_default_delegate_funcz[]={
	{_SC("call"),closure_call,-1, _SC("c")},
	{_SC("acall"),closure_acall,2, _SC("ca")},
	{0,0}
};

//GENERATOR DEFAULT DELEGATE
static int generator_getstatus(HSQUIRRELVM v)
{
	SQObject &o=stack_get(v,1);
	switch(_generator(o)->_state){
		case SQGenerator::eSuspended:v->Push(SQString::Create(_ss(v),_SC("suspended")));break;
		case SQGenerator::eRunning:v->Push(SQString::Create(_ss(v),_SC("running")));break;
		case SQGenerator::eDead:v->Push(SQString::Create(_ss(v),_SC("dead")));break;
	}
	return 1;
}

SQRegFunction SQSharedState::_generator_default_delegate_funcz[]={
	{_SC("getstatus"),generator_getstatus,1, _SC("g")},
	{0,0}
};

//THREAD DEFAULT DELEGATE

static int thread_call(HSQUIRRELVM v)
{
	SQObjectPtr o = stack_get(v,1);
	if(type(o) == OT_THREAD) {
		int nparams = sq_gettop(v);
		_thread(o)->Push(_thread(o)->_roottable);
		for(int i = 2; i<(nparams+1); i++)
			sq_move(_thread(o),v,i);
		if(SQ_SUCCEEDED(sq_call(_thread(o),nparams,SQTrue))) {
			sq_move(v,_thread(o),-1);
			return 1;
		}
		return SQ_ERROR;
	}
	return sq_throwerror(v,_SC("wrong parameter"));
}

static int thread_wakeup(HSQUIRRELVM v)
{
	SQObjectPtr o = stack_get(v,1);
	if(type(o) == OT_THREAD) {
		SQVM *thread = _thread(o);
		int state = sq_getvmstate(thread);
		if(state != SQ_VMSTATE_SUSPENDED) {
			switch(state) {
				case SQ_VMSTATE_IDLE:
					return sq_throwerror(v,_SC("cannot wakeup a idle thread"));
				break;
				case SQ_VMSTATE_RUNNING:
					return sq_throwerror(v,_SC("cannot wakeup a running thread"));
				break;
			}
		}
			
		int wakeupret = sq_gettop(v)>1?1:0;
		if(wakeupret) {
			sq_move(thread,v,2);
		}
		if(SQ_SUCCEEDED(sq_wakeupvm(thread,wakeupret,1))) {
			sq_move(v,thread,-1);
			sq_pop(thread,1);
			if(sq_getvmstate(thread) == SQ_VMSTATE_IDLE) {
				sq_pop(thread,1);
			}
			return 1;
		}
		return SQ_ERROR;
	}
	return sq_throwerror(v,_SC("wrong parameter"));
}

static int thread_getstatus(HSQUIRRELVM v)
{
	SQObjectPtr &o = stack_get(v,1);
	switch(sq_getvmstate(_thread(o))) {
		case SQ_VMSTATE_IDLE:
			sq_pushstring(v,_SC("idle"),-1);
		break;
		case SQ_VMSTATE_RUNNING:
			sq_pushstring(v,_SC("running"),-1);
		break;
		case SQ_VMSTATE_SUSPENDED:
			sq_pushstring(v,_SC("suspended"),-1);
		break;
		default:
			return sq_throwerror(v,_SC("internal VM error"));
	}
	return 1;
}

SQRegFunction SQSharedState::_thread_default_delegate_funcz[] = {
	{_SC("call"), thread_call, -1, _SC("v")},
	{_SC("wakeup"), thread_wakeup, -1, _SC("v")},
	{_SC("getstatus"), thread_getstatus, 1, _SC("v")},
	{0,0},
};

static int class_getattributes(HSQUIRRELVM v)
{
	if(SQ_SUCCEEDED(sq_getattributes(v,-2)))
		return 1;
	return SQ_ERROR;
}

static int class_setattributes(HSQUIRRELVM v)
{
	if(SQ_SUCCEEDED(sq_setattributes(v,-3)))
		return 1;
	return SQ_ERROR;
}

SQRegFunction SQSharedState::_class_default_delegate_funcz[] = {
	{_SC("getattributes"), class_getattributes, 2, _SC("y.")},
	{_SC("setattributes"), class_setattributes, 3, _SC("y..")},
	{_SC("rawin"),container_rawexists,2, _SC("y")},
	{0,0}
};

static int instance_getclass(HSQUIRRELVM v)
{
	if(SQ_SUCCEEDED(sq_getclass(v,1)))
		return 1;
	return SQ_ERROR;
}

SQRegFunction SQSharedState::_instance_default_delegate_funcz[] = {
	{_SC("getclass"), instance_getclass, 1, _SC("x")},
	{_SC("rawin"),container_rawexists,2, _SC("x")},
	{0,0}
};

