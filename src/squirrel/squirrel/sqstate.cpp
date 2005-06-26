/*
	see copyright notice in squirrel.h
*/
#include "sqpcheader.h"
#include "sqopcodes.h"
#include "sqvm.h"
#include "sqfuncproto.h"
#include "sqclosure.h"
#include "sqstring.h"
#include "sqtable.h"
#include "sqarray.h"
#include "squserdata.h"
#include "sqclass.h"

SQObjectPtr _null_;
SQObjectPtr _true_(true);
SQObjectPtr _false_(false);
SQObjectPtr _one_(1);
SQObjectPtr _minusone_(-1);

SQSharedState::SQSharedState()
{
	_compilererrorhandler = NULL;
	_printfunc = NULL;
	_debuginfo = false;
}

#define newsysstring(s) {	\
	_systemstrings->push_back(SQString::Create(this,s));	\
	}

#define newmetamethod(s) {	\
	_metamethods->push_back(SQString::Create(this,s));	\
	_table(_metamethodsmap)->NewSlot(_metamethods->back(),(SQInteger)(_metamethods->size()-1)); \
	}

bool CompileTypemask(SQIntVec &res,const SQChar *typemask)
{
	int i = 0;
	
	int mask = 0;
	while(typemask[i] != 0) {
		
		switch(typemask[i]){
				case 'o': mask |= _RT_NULL; break;
				case 'i': mask |= _RT_INTEGER; break;
				case 'f': mask |= _RT_FLOAT; break;
				case 'n': mask |= (_RT_FLOAT | _RT_INTEGER); break;
				case 's': mask |= _RT_STRING; break;
				case 't': mask |= _RT_TABLE; break;
				case 'a': mask |= _RT_ARRAY; break;
				case 'u': mask |= _RT_USERDATA; break;
				case 'c': mask |= (_RT_CLOSURE | _RT_NATIVECLOSURE); break;
				case 'b': mask |= _RT_BOOL; break;
				case 'g': mask |= _RT_GENERATOR; break;
				case 'p': mask |= _RT_USERPOINTER; break;
				case 'v': mask |= _RT_THREAD; break;
				case 'x': mask |= _RT_INSTANCE; break;
				case 'y': mask |= _RT_CLASS; break;
				case '.': mask = -1; res.push_back(mask); i++; mask = 0; continue;
				case ' ': i++; continue; //ignores spaces
				default:
					return false;
		}
		i++;
		if(typemask[i] == '|') { 
			i++; 
			if(typemask[i] == 0)
				return false;
			continue; 
		}
		res.push_back(mask);
		mask = 0;
		
	}
	return true;
}

SQTable *CreateDefaultDelegate(SQSharedState *ss,SQRegFunction *funcz)
{
	int i=0;
	SQTable *t=SQTable::Create(ss,0);
	while(funcz[i].name!=0){
		SQNativeClosure *nc = SQNativeClosure::Create(ss,funcz[i].f);
		nc->_nparamscheck = funcz[i].nparamscheck;
		nc->_name = SQString::Create(ss,funcz[i].name);
		if(funcz[i].typemask && !CompileTypemask(nc->_typecheck,funcz[i].typemask))
			return NULL;
		t->NewSlot(SQString::Create(ss,funcz[i].name),nc);
		i++;
	}
	return t;
}

void SQSharedState::Init()
{	
	_scratchpad=NULL;
	_scratchpadsize=0;
#ifndef NO_GARBAGE_COLLECTOR
	_gc_chain=NULL;
#endif
	sq_new(_stringtable,StringTable);
	sq_new(_metamethods,SQObjectPtrVec);
	sq_new(_systemstrings,SQObjectPtrVec);
	sq_new(_types,SQObjectPtrVec);
	_metamethodsmap = SQTable::Create(this,MT_LAST-1);
	//adding type strings to avoid memory trashing
	//types names
	newsysstring(_SC("null"));
	newsysstring(_SC("table"));
	newsysstring(_SC("array"));
	newsysstring(_SC("closure"));
	newsysstring(_SC("string"));
	newsysstring(_SC("userdata"));
	newsysstring(_SC("integer"));
	newsysstring(_SC("float"));
	newsysstring(_SC("userpointer"));
	newsysstring(_SC("function"));
	newsysstring(_SC("generator"));
	newsysstring(_SC("thread"));
	newsysstring(_SC("class"));
	newsysstring(_SC("instance"));
	newsysstring(_SC("bool"));
	//meta methods
	newmetamethod(MM_ADD);
	newmetamethod(MM_SUB);
	newmetamethod(MM_MUL);
	newmetamethod(MM_DIV);
	newmetamethod(MM_UNM);
	newmetamethod(MM_MODULO);
	newmetamethod(MM_SET);
	newmetamethod(MM_GET);
	newmetamethod(MM_TYPEOF);
	newmetamethod(MM_NEXTI);
	newmetamethod(MM_CMP);
	newmetamethod(MM_CALL);
	newmetamethod(MM_CLONED);
	newmetamethod(MM_NEWSLOT);
	newmetamethod(MM_DELSLOT);

	_constructoridx = SQString::Create(this,_SC("constructor"));
	_refs_table = SQTable::Create(this,0);
	_registry = SQTable::Create(this,0);
	_table_default_delegate=CreateDefaultDelegate(this,_table_default_delegate_funcz);
	_array_default_delegate=CreateDefaultDelegate(this,_array_default_delegate_funcz);
	_string_default_delegate=CreateDefaultDelegate(this,_string_default_delegate_funcz);
	_number_default_delegate=CreateDefaultDelegate(this,_number_default_delegate_funcz);
	_closure_default_delegate=CreateDefaultDelegate(this,_closure_default_delegate_funcz);
	_generator_default_delegate=CreateDefaultDelegate(this,_generator_default_delegate_funcz);
	_thread_default_delegate=CreateDefaultDelegate(this,_thread_default_delegate_funcz);
	_class_default_delegate=CreateDefaultDelegate(this,_class_default_delegate_funcz);
	_instance_default_delegate=CreateDefaultDelegate(this,_instance_default_delegate_funcz);

}

SQSharedState::~SQSharedState()
{
	_constructoridx = _null_;
	_table(_refs_table)->Finalize();
	_table(_registry)->Finalize();
	_table(_metamethodsmap)->Finalize();
	_refs_table = _null_;
	_registry = _null_;
	_metamethodsmap = _null_;
	while(!_systemstrings->empty()){
		_systemstrings->back()=_null_;
		_systemstrings->pop_back();
	}
	_thread(_root_vm)->Finalize();
	_root_vm = _null_;
	_table_default_delegate=_null_;
	_array_default_delegate=_null_;
	_string_default_delegate=_null_;
	_number_default_delegate=_null_;
	_closure_default_delegate=_null_;
	_generator_default_delegate=_null_;
	_thread_default_delegate=_null_;
	_class_default_delegate=_null_;
	_instance_default_delegate=_null_;
	
#ifndef NO_GARBAGE_COLLECTOR
	
	
	SQCollectable *t=_gc_chain;
	SQCollectable *nx=NULL;
	while(t){
		t->_uiRef++;
		t->Finalize();
		nx=t->_next;
		if(--t->_uiRef==0)
			t->Release();
		t=nx;
	}
	assert(_gc_chain==NULL); //just to proove a theory
	while(_gc_chain){
		_gc_chain->_uiRef++;
		_gc_chain->Release();
	}
#endif
	sq_delete(_types,SQObjectPtrVec);
	sq_delete(_systemstrings,SQObjectPtrVec);
	sq_delete(_metamethods,SQObjectPtrVec);
	sq_delete(_stringtable,StringTable);
	if(_scratchpad)SQ_FREE(_scratchpad,_scratchpadsize);
}


SQInteger SQSharedState::GetMetaMethodIdxByName(const SQObjectPtr &name)
{
	if(type(name) != OT_STRING)
		return -1;
	SQObjectPtr ret;
	if(_table(_metamethodsmap)->Get(name,ret)) {
		return _integer(ret);
	}
	return -1;
}

#ifndef NO_GARBAGE_COLLECTOR

void SQSharedState::MarkObject(SQObjectPtr &o,SQCollectable **chain)
{
	switch(type(o)){
	case OT_TABLE:_table(o)->Mark(chain);break;
	case OT_ARRAY:_array(o)->Mark(chain);break;
	case OT_USERDATA:_userdata(o)->Mark(chain);break;
	case OT_CLOSURE:_closure(o)->Mark(chain);break;
	case OT_NATIVECLOSURE:_nativeclosure(o)->Mark(chain);break;
	case OT_GENERATOR:_generator(o)->Mark(chain);break;
	case OT_THREAD:_thread(o)->Mark(chain);break;
	case OT_CLASS:_class(o)->Mark(chain);break;
	case OT_INSTANCE:_instance(o)->Mark(chain);break;
	}
}


int SQSharedState::CollectGarbage(SQVM *vm)
{
	int n=0;
	SQCollectable *tchain=NULL;
	SQVM *vms=_thread(_root_vm);
	
	vms->Mark(&tchain);
	int x = _table(_thread(_root_vm)->_roottable)->CountUsed();
	MarkObject(_refs_table,&tchain);
	MarkObject(_registry,&tchain);
	MarkObject(_metamethodsmap,&tchain);
	MarkObject(_table_default_delegate,&tchain);
	MarkObject(_array_default_delegate,&tchain);
	MarkObject(_string_default_delegate,&tchain);
	MarkObject(_number_default_delegate,&tchain);
	MarkObject(_generator_default_delegate,&tchain);
	MarkObject(_thread_default_delegate,&tchain);
	MarkObject(_closure_default_delegate,&tchain);
	MarkObject(_class_default_delegate,&tchain);
	MarkObject(_instance_default_delegate,&tchain);
	
	SQCollectable *t=_gc_chain;
	SQCollectable *nx=NULL;
	while(t){
		t->_uiRef++;
		t->Finalize();
		nx=t->_next;
		if(--t->_uiRef==0)
			t->Release();
		t=nx;
		n++;
	}

	t=tchain;
	while(t){
		t->UnMark();
		t=t->_next;
	}
	_gc_chain=tchain;
	int z = _table(_thread(_root_vm)->_roottable)->CountUsed();
	assert(z == x);
	return n;
}
#endif

#ifndef NO_GARBAGE_COLLECTOR
void SQCollectable::AddToChain(SQCollectable **chain,SQCollectable *c)
{
    c->_prev=NULL;
	c->_next=*chain;
	if(*chain) (*chain)->_prev=c;
	*chain=c;
}

void SQCollectable::RemoveFromChain(SQCollectable **chain,SQCollectable *c)
{
	if(c->_prev) c->_prev->_next=c->_next;
	else *chain=c->_next;
	if(c->_next)
		c->_next->_prev=c->_prev;
	c->_next=NULL;
	c->_prev=NULL;
}
#endif

SQChar* SQSharedState::GetScratchPad(int size)
{
	int newsize;
	if(size>0){
		if(_scratchpadsize<size){
			newsize=size+(size>>1);
			_scratchpad=(SQChar *)SQ_REALLOC(_scratchpad,_scratchpadsize,newsize);
			_scratchpadsize=newsize;

		}else if(_scratchpadsize>=(size<<5)){
			newsize=_scratchpadsize>>1;
			_scratchpad=(SQChar *)SQ_REALLOC(_scratchpad,_scratchpadsize,newsize);
			_scratchpadsize=newsize;
		}
	}
	return _scratchpad;
}

//////////////////////////////////////////////////////////////////////////
//StringTable
/*
* The following code is based on Lua 4.0 (Copyright 1994-2002 Tecgraf, PUC-Rio.)
* http://www.lua.org/copyright.html#4
* http://www.lua.org/source/4.0.1/src_lstring.c.html
*/

int SQString::Next(const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval)
{
	int idx = (int)TranslateIndex(refpos);
	while(idx < _len){
		outkey = (SQInteger)idx;
		outval = SQInteger(_val[idx]);
		//return idx for the next iteration
		return ++idx;
	}
	//nothing to iterate anymore
	return -1;
}

StringTable::StringTable()
{
	AllocNodes(4);
	_slotused = 0;
}

StringTable::~StringTable()
{
	SQ_FREE(_strings,sizeof(SQString*)*_numofslots);
	_strings=NULL;
}

void StringTable::AllocNodes(int size)
{
	_numofslots=size;
	//_slotused=0;
	_strings=(SQString**)SQ_MALLOC(sizeof(SQString*)*_numofslots);
	memset(_strings,0,sizeof(SQString*)*_numofslots);
}

SQString *StringTable::Add(const SQChar *news,int len)
{
	if(len<0)
		len=scstrlen(news);
	unsigned int h=::_hashstr(news,len)&(_numofslots-1);
	SQString *s;
	for (s = _strings[h]; s; s = s->_next){
		if(s->_len == len && (!memcmp(news,s->_val,rsl(len))))
			return s; //found
	}

	SQString *t=(SQString *)SQ_MALLOC(rsl(len)+sizeof(SQString));
	new (t) SQString;
	memcpy(t->_val,news,rsl(len));
	t->_val[len]=_SC('\0');
	t->_len=len;
	t->_hash=::_hashstr(news,len);
	t->_next=_strings[h];
	t->_uiRef=0;
	_strings[h]=t;
	_slotused++;
	if (_slotused > _numofslots)  /* too crowded? */
		Resize(_numofslots*2);
	return t;
}

void StringTable::Resize(int size)
{
	int oldsize=_numofslots;
	SQString **oldtable=_strings;
	AllocNodes(size);
	for (int i=0; i<oldsize; i++){
		SQString *p = oldtable[i];
		while(p){
			SQString *next = p->_next;
			unsigned int h=p->_hash&(_numofslots-1);
			p->_next=_strings[h];
			_strings[h] = p;
			p=next;
		}
	}
	SQ_FREE(oldtable,oldsize*sizeof(SQString*));
}

void StringTable::Remove(SQString *bs)
{
	SQString *s;
	SQString *prev=NULL;
	unsigned int h=bs->_hash&(_numofslots-1);
	
	for (s = _strings[h]; s; ){
		if(s == bs){
			if(prev)
				prev->_next = s->_next;
			else
				_strings[h] = s->_next;
			_slotused--;
			int slen=s->_len;
			s->~SQString();
			SQ_FREE(s,sizeof(SQString)+rsl(slen));
			return;
		}
		prev = s;
		s = s->_next;
	}
	assert(0);//if this fail something is wrong
}
