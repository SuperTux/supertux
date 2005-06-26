/*
	see copyright notice in squirrel.h
*/
#include "sqpcheader.h"
#include "sqvm.h"
#include "sqtable.h"
#include "sqclass.h"
#include "sqclosure.h"

SQClass::SQClass(SQSharedState *ss,SQClass *base)
{
	_uiRef=0;
	_base = base;
	_typetag = 0;
	_metamethods.resize(MT_LAST); //size it to max size
	if(_base) {
		_defaultvalues.copy(base->_defaultvalues);
		_methods.copy(base->_methods);
		_metamethods.copy(base->_metamethods);
		__ObjAddRef(_base);
	}
	_members = base?base->_members->Clone() : SQTable::Create(ss,0);
	__ObjAddRef(_members);
	_locked = false;
	INIT_CHAIN();
	ADD_TO_CHAIN(&_sharedstate->_gc_chain, this);
}

void SQClass::Finalize() { 
	_attributes = _null_;
	_defaultvalues.resize(0);
	_methods.resize(0);
	_metamethods.resize(0);
	__ObjRelease(_members);
	if(_base) {
		__ObjRelease(_base);
	}
}

SQClass::~SQClass()
{
	REMOVE_FROM_CHAIN(&_sharedstate->_gc_chain, this);
	Finalize();
}

bool SQClass::NewSlot(const SQObjectPtr &key,const SQObjectPtr &val)
{
	SQObjectPtr temp;
	if(_locked) 
		return false; //the slot already exists
	if(_members->Get(key,temp) && type(temp) == OT_INTEGER) //overrides the default value
	{
		_defaultvalues[_integer(temp)].val = val;
		return true;
	}
	if(type(val) == OT_CLOSURE || type(val) == OT_NATIVECLOSURE) {
		SQInteger mmidx;
		if((mmidx = _sharedstate->GetMetaMethodIdxByName(key)) != -1) {
			_metamethods[mmidx] = val;
		} 
		else {
			if(type(temp) == OT_NULL) {
				SQClassMemeber m;
				m.val = val;
				_members->NewSlot(key,SQObjectPtr((SQUserPointer)_methods.size()));
				_methods.push_back(m);
			}
			else {
				_methods[(int)_userpointer(temp)].val = val;
			}
		}
		return true;
	}
	SQClassMemeber m;
	m.val = val;
	_members->NewSlot(key,SQObjectPtr((SQInteger)_defaultvalues.size()));
	_defaultvalues.push_back(m);
	return true;
}

SQInstance *SQClass::CreateInstance()
{
	if(!_locked) Lock();
	return SQInstance::Create(_opt_ss(this),this);
}

int SQClass::Next(const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval)
{
	SQObjectPtr oval;
	int idx = _members->Next(refpos,outkey,oval);
	if(idx != -1) {
		if(type(oval) != OT_INTEGER) {
			outval = _methods[(int)_userpointer(oval)].val;
		}
		else {
			outval = _defaultvalues[_integer(oval)].val;
		}
	}
	return idx;
}

bool SQClass::SetAttributes(const SQObjectPtr &key,const SQObjectPtr &val)
{
	SQObjectPtr idx;
	if(_members->Get(key,idx)) {
		if(type(idx) == OT_INTEGER)
			_defaultvalues[_integer(idx)].attrs = val;
		else
			_methods[(int)_userpointer(idx)].attrs = val;
		return true;
	}
	return false;
}

bool SQClass::GetAttributes(const SQObjectPtr &key,SQObjectPtr &outval)
{
	SQObjectPtr idx;
	if(_members->Get(key,idx)) {
		outval = (type(idx) == OT_INTEGER?_defaultvalues[_integer(idx)].attrs:_methods[(int)_userpointer(idx)].attrs);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////
void SQInstance::Init(SQSharedState *ss)
{
	_uiRef = 0;
	_userpointer = NULL;
	_hook = NULL;
	__ObjAddRef(_class);
	_delegate = _class->_members;
	INIT_CHAIN();
	ADD_TO_CHAIN(&_sharedstate->_gc_chain, this);
}

SQInstance::SQInstance(SQSharedState *ss, SQClass *c)
{
	_class = c;
	_values.resize(_class->_defaultvalues.size());
	for(unsigned int i = 0; i < _class->_defaultvalues.size(); i++) {
		_values[i] = _class->_defaultvalues[i].val;
	}
	Init(ss);
}

SQInstance::SQInstance(SQSharedState *ss, SQInstance *i)
{
	_class = i->_class;
	_values.copy(i->_values);
	Init(ss);
}

void SQInstance::Finalize() 
{
	__ObjRelease(_class);
	_values.resize(0);
}

SQInstance::~SQInstance()
{
	REMOVE_FROM_CHAIN(&_sharedstate->_gc_chain, this);
	Finalize();
}

bool SQInstance::GetMetaMethod(SQMetaMethod mm,SQObjectPtr &res)
{
	if(type(_class->_metamethods[mm]) != OT_NULL) {
		res = _class->_metamethods[mm];
		return true;
	}
	return false;
}

bool SQInstance::InstanceOf(SQClass *trg)
{
	SQClass *parent = _class;
	while(parent != NULL) {
		if(parent == trg)
			return true;
		parent = parent->_base;
	}
	return false;
}
