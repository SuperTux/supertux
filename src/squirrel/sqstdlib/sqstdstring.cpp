/* see copyright notice in squirrel.h */
#include <squirrel.h>
#include <sqstdstring.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#ifdef _UNICODE
#define scstrchr wcschr
#define scsnprintf wsnprintf
#define scatoi _wtoi
#else
#define scstrchr strchr
#define scsnprintf snprintf
#define scatoi atoi
#endif
#define MAX_FORMAT_LEN	20
#define MAX_WFORMAT_LEN	3
#define ADDITIONAL_FORMAT_SPACE (100*sizeof(SQChar))

static int validate_format(HSQUIRRELVM v, SQChar *fmt, const SQChar *src, int n,int &width)
{
	SQChar swidth[MAX_WFORMAT_LEN];
	int wc = 0;
	int start = n;
	fmt[0] = '%';
	while (scstrchr(_SC("-+ #0"), src[n])) n++;
	while (scisdigit(src[n])) {
		swidth[wc] = src[n];
		n++;
		wc++;
		if(wc>=MAX_WFORMAT_LEN)
			return sq_throwerror(v,_SC("width format too long"));
	}
	swidth[wc] = '\0';
	if(wc > 0) {
		width = scatoi(swidth);
	}
	else
		width = 0;
	if (src[n] == '.') {
	    n++;
    	
		wc = 0;
		while (scisdigit(src[n])) {
			swidth[wc] = src[n];
			n++;
			wc++;
			if(wc>=MAX_WFORMAT_LEN)
				return sq_throwerror(v,_SC("precision format too long"));
		}
		swidth[wc] = '\0';
		if(wc > 0) {
			width += scatoi(swidth);
		}
	}
	if (n-start > MAX_FORMAT_LEN )
		return sq_throwerror(v,_SC("format too long"));
	memcpy(&fmt[1],&src[start],((n-start)+1)*sizeof(SQChar));
	fmt[(n-start)+2] = '\0';
	return n;
}

static int _string_format(HSQUIRRELVM v)
{
	const SQChar *format;
	SQChar *dest;
	SQChar fmt[MAX_FORMAT_LEN];
	sq_getstring(v,2,&format);
	int allocated = (sq_getsize(v,2)+1)*sizeof(SQChar);
	dest = sq_getscratchpad(v,allocated);
	int n = 0,i = 0, nparam = 3, w;
	while(format[n] != '\0') {
		if(format[n] != '%') {
			dest[i++] = format[n];
			n++;
		}
		else if(format[++n] == '%') {
			dest[i++] = '%';
		}
		else {
			if( nparam > sq_gettop(v) )
				return sq_throwerror(v,_SC("not enough paramters for the given format string"));
			n = validate_format(v,fmt,format,n,w);
			if(n < 0) return -1;
			int addlen = 0;
			int valtype = 0;
			const SQChar *ts;
			SQInteger ti;
			SQFloat tf;
			switch(format[n]) {
			case 's':
				if(SQ_FAILED(sq_getstring(v,nparam,&ts))) 
					return sq_throwerror(v,_SC("string expected for the specified format"));
				addlen = (sq_getsize(v,nparam)*sizeof(SQChar))+((w+1)*sizeof(SQChar));
				valtype = 's';
				break;
			case 'i': case 'd': case 'c':case 'o':  case 'u':  case 'x':  case 'X':
				if(SQ_FAILED(sq_getinteger(v,nparam,&ti))) 
					return sq_throwerror(v,_SC("integer expected for the specified format"));
				addlen = (ADDITIONAL_FORMAT_SPACE)+((w+1)*sizeof(SQChar));
				valtype = 'i';
				break;
			case 'f': case 'g': case 'G': case 'e':  case 'E':
				if(SQ_FAILED(sq_getfloat(v,nparam,&tf))) 
					return sq_throwerror(v,_SC("float expected for the specified format"));
				addlen = (ADDITIONAL_FORMAT_SPACE)+((w+1)*sizeof(SQChar));
				valtype = 'f';
				break;
			default:
				return sq_throwerror(v,_SC("invalid format"));
			}
			n++;
			if((allocated-i) < addlen)
				allocated += addlen - (allocated-i);
			dest = sq_getscratchpad(v,allocated);
			switch(valtype) {
			case 's': i += scsprintf(&dest[i],fmt,ts); break;
			case 'i': i += scsprintf(&dest[i],fmt,ti); break;
			case 'f': i += scsprintf(&dest[i],fmt,tf); break;
			};
			nparam ++;
		}
	}
	sq_pushstring(v,dest,i);
	return 1;
}

#define SETUP_REX(v) \
	SQRex *self = NULL; \
	sq_getinstanceup(v,1,(SQUserPointer *)&self,0); 

static int _rexobj_releasehook(SQUserPointer p, int size)
{
	SQRex *self = ((SQRex *)p);
	sqstd_rex_free(self);
	return 1;
}

static int _regexp_match(HSQUIRRELVM v)
{
	SETUP_REX(v);
	const SQChar *str;
	sq_getstring(v,2,&str);
	if(sqstd_rex_match(self,str) == SQTrue)
	{
		sq_pushinteger(v,1);
		return 1;
	}
	return 0;
}

static void _addrexmatch(HSQUIRRELVM v,const SQChar *str,const SQChar *begin,const SQChar *end)
{
	sq_newtable(v);
	sq_pushstring(v,_SC("begin"),-1);
	sq_pushinteger(v,begin - str);
	sq_rawset(v,-3);
	sq_pushstring(v,_SC("end"),-1);
	sq_pushinteger(v,end - str);
	sq_rawset(v,-3);
}

static int _regexp_search(HSQUIRRELVM v)
{
	SETUP_REX(v);
	const SQChar *str,*begin,*end;
	SQInteger start = 0;
	sq_getstring(v,2,&str);
	if(sq_gettop(v) > 2) sq_getinteger(v,3,&start);
	if(sqstd_rex_search(self,str+start,&begin,&end) == SQTrue) {
		_addrexmatch(v,str,begin,end);
		return 1;
	}
	return 0;
}

static int _regexp_capture(HSQUIRRELVM v)
{
	SETUP_REX(v);
	const SQChar *str,*begin,*end;
	SQInteger start = 0;
	sq_getstring(v,2,&str);
	if(sq_gettop(v) > 2) sq_getinteger(v,3,&start);
	if(sqstd_rex_search(self,str+start,&begin,&end) == SQTrue) {
		SQInteger n = sqstd_rex_getsubexpcount(self);
		SQRexMatch match;
		sq_newarray(v,0);
		for(SQInteger i = 0;i < n; i++) {
			sqstd_rex_getsubexp(self,i,&match);
			if(match.len > 0)
				_addrexmatch(v,str,match.begin,match.begin+match.len);
			else
				_addrexmatch(v,str,str,str); //empty match
			sq_arrayappend(v,-2);
		}
		return 1;
	}
	return 0;
}

static int _regexp_subexpcount(HSQUIRRELVM v)
{
	SETUP_REX(v);
	sq_pushinteger(v,sqstd_rex_getsubexpcount(self));
	return 1;
}

static int _regexp_constructor(HSQUIRRELVM v)
{
	const SQChar *error,*pattern;
	sq_getstring(v,2,&pattern);
	SQRex *rex = sqstd_rex_compile(pattern,&error);
	if(!rex) return sq_throwerror(v,error);
	sq_setinstanceup(v,1,rex);
	sq_setreleasehook(v,1,_rexobj_releasehook);
	return 0;
}

static int _regexp__typeof(HSQUIRRELVM v)
{
	sq_pushstring(v,_SC("regexp"),-1);
	return 1;
}

#define _DECL_REX_FUNC(name,nparams,pmask) {_SC(#name),_regexp_##name,nparams,pmask}
static SQRegFunction rexobj_funcs[]={
	_DECL_REX_FUNC(constructor,2,_SC(".s")),
	_DECL_REX_FUNC(search,-2,_SC("xsn")),
	_DECL_REX_FUNC(match,2,_SC("xs")),
	_DECL_REX_FUNC(capture,-2,_SC("xsn")),
	_DECL_REX_FUNC(subexpcount,1,_SC("x")),
	_DECL_REX_FUNC(_typeof,1,_SC("x")),
	{0,0}
};

#define _DECL_FUNC(name,nparams,pmask) {_SC(#name),_string_##name,nparams,pmask}
static SQRegFunction stringlib_funcs[]={
	_DECL_FUNC(format,-2,_SC(".s")),
	{0,0}
};


int sqstd_register_stringlib(HSQUIRRELVM v)
{
	sq_pushstring(v,_SC("regexp"),-1);
	sq_newclass(v,SQFalse);
	int i = 0;
	while(rexobj_funcs[i].name != 0) {
		SQRegFunction &f = rexobj_funcs[i];
		sq_pushstring(v,f.name,-1);
		sq_newclosure(v,f.f,0);
		sq_setparamscheck(v,f.nparamscheck,f.typemask);
		sq_setnativeclosurename(v,-1,f.name);
		sq_createslot(v,-3);
		i++;
	}
	sq_createslot(v,-3);

	i = 0;
	while(stringlib_funcs[i].name!=0)
	{
		sq_pushstring(v,stringlib_funcs[i].name,-1);
		sq_newclosure(v,stringlib_funcs[i].f,0);
		sq_setparamscheck(v,stringlib_funcs[i].nparamscheck,stringlib_funcs[i].typemask);
		sq_setnativeclosurename(v,-1,stringlib_funcs[i].name);
		sq_createslot(v,-3);
		i++;
	}
	return 1;
}
