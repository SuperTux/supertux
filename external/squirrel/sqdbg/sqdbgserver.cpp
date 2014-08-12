#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <squirrel.h>
#include <assert.h>
#include <sqstdblob.h>
#include "sqrdbg.h"
#include "sqdbgserver.h"


#ifndef _UNICODE
#define scstrcpy strcpy
#else
#define scstrcpy wcscpy
#endif
struct XMLEscape{
	const SQChar c;
	const SQChar *esc;
};

#define SQDBG_DEBUG_HOOK _SC("_sqdbg_debug_hook_")
#define SQDBG_ERROR_HANDLER _SC("_sqdbg_error_handler_")

XMLEscape g_escapes[]={
	{_SC('<'),_SC("&lt;")},{'>',_SC("&gt;")},{_SC('&'),_SC("&amp;")},{_SC('\''),_SC("&apos;")},{_SC('\"'),_SC("&quot;")},{_SC('\n'),_SC("&quot;n")},{_SC('\r'),_SC("&quot;r")},{0, NULL}
};

const SQChar *IntToString(int n)
{
	static SQChar temp[256];
	scsprintf(temp,_SC("%d"),n);
	return temp;
}

SQInteger debug_hook(HSQUIRRELVM v);
SQInteger error_handler(HSQUIRRELVM v);

SQInteger beginelement(HSQUIRRELVM v)
{
	SQUserPointer up;
	const SQChar *name;
	sq_getuserpointer(v,-1,&up);
	SQDbgServer *self = (SQDbgServer*)up;
	sq_getuserpointer(v,-1,&up);
	sq_getstring(v,2,&name);
	self->BeginElement(name);
	return 0;
}

SQInteger endelement(HSQUIRRELVM v)
{
	SQUserPointer up;
	const SQChar *name;
	sq_getuserpointer(v,-1,&up);
	SQDbgServer *self = (SQDbgServer*)up;
	sq_getuserpointer(v,-1,&up);
	sq_getstring(v,2,&name);
	self->EndElement(name);
	return 0;
}

SQInteger attribute(HSQUIRRELVM v)
{
	SQUserPointer up;
	const SQChar *name,*value;
	sq_getuserpointer(v,-1,&up);
	SQDbgServer *self = (SQDbgServer*)up;
	sq_getuserpointer(v,-1,&up);
	sq_getstring(v,2,&name);
	sq_getstring(v,3,&value);
	self->Attribute(name,value);
	return 0;
}

const SQChar *EscapeXMLString(HSQUIRRELVM v,const SQChar *s)
{

	SQChar *temp=sq_getscratchpad(v,((int)scstrlen(s)*6) + sizeof(SQChar));
	SQChar *dest=temp;
	while(*s!=_SC('\0')){
		int i=0;
		bool escaped=false;
		while(g_escapes[i].esc!=NULL){
			if(*s==g_escapes[i].c){
				scstrcpy(dest,g_escapes[i].esc);
				dest+=scstrlen(g_escapes[i].esc);
				escaped=true;
				break;
			}
			i++;
		}
		if(!escaped){*dest=*s;*dest++;}
		*s++;
	}
	*dest=_SC('\0');
	return temp;
}

SQDbgServer::SQDbgServer(HSQUIRRELVM v)
{
	_ready = false;
	_nestedcalls = 0;
	_autoupdate = false;
	_v = v;
	_state = eDBG_Running;
	_accept = INVALID_SOCKET;
	_endpoint = INVALID_SOCKET;
	_maxrecursion = 10;
	sq_resetobject(&_debugroot);
}

SQDbgServer::~SQDbgServer()
{
	sq_release(_v,&_debugroot);
	if(_accept != INVALID_SOCKET)
		sqdbg_closesocket(_accept);
	if(_endpoint != INVALID_SOCKET)
		sqdbg_closesocket(_endpoint);
}

bool SQDbgServer::Init()
{
	//creates  an environment table for the debugger

	sq_newtable(_v);
	sq_getstackobj(_v,-1,&_debugroot);
	sq_addref(_v,&_debugroot);

	//creates a emptyslot to store the watches
	sq_pushstring(_v,_SC("watches"),-1);
	sq_pushnull(_v);
	sq_createslot(_v,-3);

	sq_pushstring(_v,_SC("beginelement"),-1);
	sq_pushuserpointer(_v,this);
	sq_newclosure(_v,beginelement,1);
	sq_setparamscheck(_v,2,_SC(".s"));
	sq_createslot(_v,-3);

	sq_pushstring(_v,_SC("endelement"),-1);
	sq_pushuserpointer(_v,this);
	sq_newclosure(_v,endelement,1);
	sq_setparamscheck(_v,2,_SC(".s"));
	sq_createslot(_v,-3);

	sq_pushstring(_v,_SC("attribute"),-1);
	sq_pushuserpointer(_v,this);
	sq_newclosure(_v,attribute,1);
	sq_setparamscheck(_v,3,_SC(".ss"));
	sq_createslot(_v,-3);

	sq_pop(_v,1);

	//stores debug hook and error handler in the registry
	sq_pushregistrytable(_v);

	sq_pushstring(_v,SQDBG_DEBUG_HOOK,-1);
	sq_pushuserpointer(_v,this);
	sq_newclosure(_v,debug_hook,1);
	sq_createslot(_v,-3);

	sq_pushstring(_v,SQDBG_ERROR_HANDLER,-1);
	sq_pushuserpointer(_v,this);
	sq_newclosure(_v,error_handler,1);
	sq_createslot(_v,-3);


	sq_pop(_v,1);

	//sets the error handlers
	SetErrorHandlers();
	return true;
}

bool SQDbgServer::ReadMsg()
{
	return false;
}

void SQDbgServer::BusyWait()
{
	while( !ReadMsg() )
		sleep(0);
}

void SQDbgServer::SendChunk(const SQChar *chunk)
{
	char *buf=NULL;
	int buf_len=0;
#ifdef _UNICODE
	buf_len=(int)scstrlen(chunk)+1;
	buf=(char *)sq_getscratchpad(_v,(buf_len)*3);
	wcstombs((char *)buf,chunk,buf_len);
#else
	buf_len=(int)scstrlen(chunk);
	buf=(char *)chunk;
#endif
	send(_endpoint,(const char*)buf,(int)strlen((const char *)buf),0);
}


void SQDbgServer::Terminated()
{
	BeginElement(_SC("terminated"));
	EndElement(_SC("terminated"));
	::usleep(200);
}

void SQDbgServer::Hook(int type,int line,const SQChar *src,const SQChar *func)
{
	switch(_state){
	case eDBG_Running:
		if(type==_SC('l') && _breakpoints.size()) {
			BreakPointSetItor itr = _breakpoints.find(BreakPoint(line,src));
			if(itr != _breakpoints.end()) {
				Break(line,src,_SC("breakpoint"));
				BreakExecution();
			}
		}
		break;
	case eDBG_Suspended:
		_nestedcalls=0;
	case eDBG_StepOver:
		switch(type){
		case _SC('l'):
			if(_nestedcalls==0) {
				Break(line,src,_SC("step"));
				BreakExecution();
			}
			break;
		case _SC('c'):
			_nestedcalls++;
			break;
		case _SC('r'):
			if(_nestedcalls==0){
				_nestedcalls=0;

			}else{
				_nestedcalls--;
			}
			break;
		}
		break;
	case eDBG_StepInto:
		switch(type){
		case _SC('l'):
			_nestedcalls=0;
			Break(line,src,_SC("step"));
			BreakExecution();
			break;

		}
		break;
	case eDBG_StepReturn:
		switch(type){
		case _SC('l'):
			break;
		case _SC('c'):
			_nestedcalls++;
			break;
		case _SC('r'):
			if(_nestedcalls==0){
				_nestedcalls=0;
				_state=eDBG_StepOver;
			}else{
				_nestedcalls--;
			}

			break;
		}
		break;
	case eDBG_Disabled:
		break;
	}
}


#define MSG_ID(x,y) ((y<<8)|x)
//ab Add Breakpoint
//rb Remove Breakpoint
//sp Suspend
void SQDbgServer::ParseMsg(const char *msg)
{

	switch(*((unsigned short *)msg)){
		case MSG_ID('a','b'): {
			BreakPoint bp;
			if(ParseBreakpoint(msg+3,bp)){
				AddBreakpoint(bp);
				scprintf(_SC("added bp %d %s\n"),bp._line,bp._src.c_str());
			}
			else
				scprintf(_SC("error parsing add breakpoint"));
							 }
			break;
		case MSG_ID('r','b'): {
			BreakPoint bp;
			if(ParseBreakpoint(msg+3,bp)){
				RemoveBreakpoint(bp);
				scprintf(_SC("removed bp %d %s\n"),bp._line,bp._src.c_str());
			}else
				scprintf(_SC("error parsing remove breakpoint"));
							}
			break;
		case MSG_ID('g','o'):
			if(_state!=eDBG_Running){
				_state=eDBG_Running;
				BeginDocument();
					BeginElement(_SC("resumed"));
					EndElement(_SC("resumed"));
				EndDocument();
//				Send(_SC("<resumed/>\r\n"));
				scprintf(_SC("go (execution resumed)\n"));
			}
			break;
		case MSG_ID('s','p'):
			if(_state!=eDBG_Suspended){
				_state=eDBG_Suspended;
				scprintf(_SC("suspend\n"));
			}
			break;
		case MSG_ID('s','o'):
			if(_state==eDBG_Suspended){
				_state=eDBG_StepOver;
			}
			break;
		case MSG_ID('s','i'):
			if(_state==eDBG_Suspended){
				_state=eDBG_StepInto;
				scprintf(_SC("step into\n"));
			}
			break;
		case MSG_ID('s','r'):
			if(_state==eDBG_Suspended){
				_state=eDBG_StepReturn;
				scprintf(_SC("step return\n"));
			}
			break;
		case MSG_ID('d','i'):
			if(_state!=eDBG_Disabled){
				_state=eDBG_Disabled;
				scprintf(_SC("disabled\n"));
			}
			break;
		case MSG_ID('a','w'): {
			Watch w;
			if(ParseWatch(msg+3,w))
			{
				AddWatch(w);
				scprintf(_SC("added watch %d %s\n"),w._id,w._exp.c_str());
			}
			else
				scprintf(_SC("error parsing add watch"));
								}
			break;
		case MSG_ID('r','w'): {
			int id;
			if(ParseRemoveWatch(msg+3,id))
			{
				RemoveWatch(id);
				scprintf(_SC("added watch %d\n"),id);
			}
			else
				scprintf(_SC("error parsing remove watch"));
								}
			break;
		case MSG_ID('t','r'):
			scprintf(_SC("terminate from user\n"));
			break;
		case MSG_ID('r','d'):
			scprintf(_SC("ready\n"));
			_ready=true;
			break;
		default:
			scprintf(_SC("unknown packet"));

	}
}

/*
	see copyright notice in sqrdbg.h
*/
bool SQDbgServer::ParseBreakpoint(const char *msg,BreakPoint &out)
{
	static char stemp[MAX_BP_PATH];
	char *ep=NULL;
	out._line=strtoul(msg,&ep,16);
	if(ep==msg || (*ep)!=':')return false;

	char *dest=stemp;
	ep++;
	while((*ep)!='\n' && (*ep)!='\0')
	{
		*dest=*ep;
		*dest++;*ep++;
	}
	*dest='\0';
	*dest++;
	*dest='\0';
#ifdef _UNICODE
	int len=(int)strlen(stemp);
	SQChar *p=sq_getscratchpad(_v,(SQInteger)(mbstowcs(NULL,stemp,len)+2)*sizeof(SQChar));
	size_t destlen=mbstowcs(p,stemp,len);
	p[destlen]=_SC('\0');
	out._src=p;
#else
	out._src=stemp;
#endif
	return true;
}

bool SQDbgServer::ParseWatch(const char *msg,Watch &out)
{
	char *ep=NULL;
	out._id=strtoul(msg,&ep,16);
	if(ep==msg || (*ep)!=':')return false;

	//char *dest=out._src;
	ep++;
	while((*ep)!='\n' && (*ep)!='\0')
	{
		out._exp.append(1,*ep);
		*ep++;
	}
	return true;
}

bool SQDbgServer::ParseRemoveWatch(const char *msg,int &id)
{
	char *ep=NULL;
	id=strtoul(msg,&ep,16);
	if(ep==msg)return false;
	return true;
}


void SQDbgServer::BreakExecution()
{
	_state=eDBG_Suspended;
	while(_state==eDBG_Suspended){
		if(SQ_FAILED(sq_rdbg_update(this)))
			exit(0);
		usleep(10);
	}
}

//COMMANDS
void SQDbgServer::AddBreakpoint(BreakPoint &bp)
{
	_breakpoints.insert(bp);
	BeginDocument();
		BeginElement(_SC("addbreakpoint"));
			Attribute(_SC("line"),IntToString(bp._line));
			Attribute(_SC("src"),bp._src.c_str());
		EndElement(_SC("addbreakpoint"));
	EndDocument();
}

void SQDbgServer::AddWatch(Watch &w)
{
	_watches.insert(w);
}

void SQDbgServer::RemoveWatch(int id)
{
	WatchSetItor itor=_watches.find(Watch(id,_SC("")));
	if(itor==_watches.end()){
		BeginDocument();
		BeginElement(_SC("error"));
			Attribute(_SC("desc"),_SC("the watch does not exists"));
		EndElement(_SC("error"));
	EndDocument();
	}
	else{
		_watches.erase(itor);
		scprintf(_SC("removed watch %d\n"),id);
	}
}

void SQDbgServer::RemoveBreakpoint(BreakPoint &bp)
{
	BreakPointSetItor itor=_breakpoints.find(bp);
	if(itor==_breakpoints.end()){
		BeginDocument();
			BeginElement(_SC("break"));
				Attribute(_SC("desc"),_SC("the breakpoint doesn't exists"));
			EndElement(_SC("break"));
		EndDocument();
	}
	else{
		BeginDocument();
			BeginElement(_SC("removebreakpoint"));
				Attribute(_SC("line"),IntToString(bp._line));
				Attribute(_SC("src"),bp._src.c_str());
			EndElement(_SC("removebreakpoint"));
		EndDocument();
		_breakpoints.erase(itor);
	}
}

void SQDbgServer::Break(int line,const SQChar *src,const SQChar *type,const SQChar *error)
{
	if(!error){
		BeginDocument();
			BeginElement(_SC("break"));
				Attribute(_SC("line"),IntToString(line));
				Attribute(_SC("src"),src);
				Attribute(_SC("type"),type);
				SerializeState();
			EndElement(_SC("break"));
		EndDocument();
	}else{
		BeginDocument();
			BeginElement(_SC("break"));
				Attribute(_SC("line"),IntToString(line));
				Attribute(_SC("src"),src);
				Attribute(_SC("type"),type);
				Attribute(_SC("error"),error);
				SerializeState();
			EndElement(_SC("break"));
		EndDocument();
	}
}

void SQDbgServer::SerializeState()
{
	sq_pushnull(_v);
	sq_setdebughook(_v);
	sq_pushnull(_v);
	sq_seterrorhandler(_v);
	const SQChar *sz;
	sq_pushobject(_v,_serializefunc);
	sq_pushobject(_v,_debugroot);
	sq_pushstring(_v,_SC("watches"),-1);
	sq_newtable(_v);
	for(WatchSetItor i=_watches.begin(); i!=_watches.end(); ++i)
	{
		sq_pushinteger(_v,i->_id);
		sq_pushstring(_v,i->_exp.c_str(),(int)i->_exp.length());
		sq_createslot(_v,-3);
	}
	sq_rawset(_v,-3);
	if(SQ_SUCCEEDED(sq_call(_v,1,SQTrue,SQTrue))){
		if(SQ_SUCCEEDED(sqstd_getblob(_v,-1,(SQUserPointer*)&sz)))
			SendChunk(sz);
	}
	sq_pop(_v,2);

	SetErrorHandlers();
}


void SQDbgServer::SetErrorHandlers()
{
	sq_pushregistrytable(_v);
	sq_pushstring(_v,SQDBG_DEBUG_HOOK,-1);
	sq_rawget(_v,-2);
	sq_setdebughook(_v);
	sq_pushstring(_v,SQDBG_ERROR_HANDLER,-1);
	sq_rawget(_v,-2);
	sq_seterrorhandler(_v);
	sq_pop(_v,1);
}

void SQDbgServer::BeginElement(const SQChar *name)
{
	_xmlcurrentement++;
	XMLElementState *self = &xmlstate[_xmlcurrentement];
	scstrcpy(self->name,name);
	self->haschildren = false;
	if(_xmlcurrentement > 0) {
		XMLElementState *parent = &xmlstate[_xmlcurrentement-1];
		if(!parent->haschildren) {
			SendChunk(_SC(">")); // closes the parent tag
			parent->haschildren = true;
		}
	}
	_scratchstring.resize(2+scstrlen(name));
	scsprintf(&_scratchstring[0],_SC("<%s"),name);
	SendChunk(&_scratchstring[0]);
}

void SQDbgServer::Attribute(const SQChar *name,const SQChar *value)
{
	XMLElementState *self = &xmlstate[_xmlcurrentement];
	assert(!self->haschildren); //cannot have attributes if already has children
	const SQChar *escval = escape_xml(value);
	_scratchstring.resize(5+scstrlen(name)+scstrlen(escval));
	scsprintf(&_scratchstring[0],_SC(" %s=\"%s\""),name,escval);
	SendChunk(&_scratchstring[0]);
}

void SQDbgServer::EndElement(const SQChar *name)
{
	XMLElementState *self = &xmlstate[_xmlcurrentement];
	assert(scstrcmp(self->name,name) == 0);
	if(self->haschildren) {
		_scratchstring.resize(4+scstrlen(name));
		scsprintf(&_scratchstring[0],_SC("</%s>"),name);
		SendChunk(&_scratchstring[0]);

	}
	else {
		SendChunk(_SC("/>"));
	}
	_xmlcurrentement--;
}

void SQDbgServer::EndDocument()
{
	SendChunk(_SC("\r\n"));
}

//this can be done much better/faster(do we need that?)
const SQChar *SQDbgServer::escape_xml(const SQChar *s)
{
	SQChar *temp=sq_getscratchpad(_v,((int)scstrlen(s)*6) + sizeof(SQChar));
	SQChar *dest=temp;
	while(*s!=_SC('\0')){
		int i=0;
		bool escaped=false;
		while(g_escapes[i].esc!=NULL){
			if(*s==g_escapes[i].c){
				scstrcpy(dest,g_escapes[i].esc);
				dest+=scstrlen(g_escapes[i].esc);
				escaped=true;
				break;
			}
			i++;
		}
		if(!escaped){*dest=*s;*dest++;}
		*s++;
	}
	*dest=_SC('\0');
	return temp;

}
