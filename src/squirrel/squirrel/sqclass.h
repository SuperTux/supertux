/*	see copyright notice in squirrel.h */
#ifndef _SQCLASS_H_
#define _SQCLASS_H_

struct SQInstance;

struct SQClassMemeber {
	SQClassMemeber(){}
	SQClassMemeber(const SQClassMemeber &o) {
		val = o.val;
		attrs = o.attrs;
	}
	SQObjectPtr val;
	SQObjectPtr attrs;
};

typedef sqvector<SQClassMemeber> SQClassMemeberVec;

struct SQClass : public CHAINABLE_OBJ
{
	SQClass(SQSharedState *ss,SQClass *base);
public:
	static SQClass* Create(SQSharedState *ss,SQClass *base) {
		SQClass *newclass = (SQClass *)SQ_MALLOC(sizeof(SQClass));
		new (newclass) SQClass(ss, base);
		return newclass;
	}
	~SQClass();
	bool NewSlot(const SQObjectPtr &key,const SQObjectPtr &val);
	bool Get(const SQObjectPtr &key,SQObjectPtr &val) {
		if(_members->Get(key,val)) {
			val = (type(val) == OT_INTEGER?_defaultvalues[_integer(val)].val:_methods[(int)_userpointer(val)].val);
			return true;
		}
		return false;
	}
	bool SetAttributes(const SQObjectPtr &key,const SQObjectPtr &val);
	bool GetAttributes(const SQObjectPtr &key,SQObjectPtr &outval);
	void Lock() { _locked = true; if(_base) _base->Lock(); }
	void Release() { sq_delete(this, SQClass);	}
	void Finalize();
	void Mark(SQCollectable ** );
	int Next(const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval);
	SQInstance *CreateInstance();
	SQTable *_members;
	//SQTable *_properties;
	SQClass *_base;
	SQClassMemeberVec _defaultvalues;
	SQClassMemeberVec _methods;
	SQObjectPtrVec _metamethods;
	SQObjectPtr _attributes;
	unsigned int _typetag;
	bool _locked;
};

struct SQInstance : public SQDelegable 
{
	void Init(SQSharedState *ss);
	SQInstance(SQSharedState *ss, SQClass *c);
	SQInstance(SQSharedState *ss, SQInstance *c);
public:
	static SQInstance* Create(SQSharedState *ss,SQClass *theclass) {
		SQInstance *newinst = (SQInstance *)SQ_MALLOC(sizeof(SQInstance));
		new (newinst) SQInstance(ss, theclass);
		return newinst;
	}
	SQInstance *Clone(SQSharedState *ss)
	{
		SQInstance *newinst = (SQInstance *)SQ_MALLOC(sizeof(SQInstance));
		new (newinst) SQInstance(ss, this);
		return newinst;
	}
	~SQInstance();
	bool Get(const SQObjectPtr &key,SQObjectPtr &val)  {
		if(_class->_members->Get(key,val)) {
			val = (type(val) == OT_INTEGER?_values[_integer(val)]:_class->_methods[(int)_userpointer(val)].val);
			return true;
		}
		return false;
	}
	bool Set(const SQObjectPtr &key,const SQObjectPtr &val) {
		SQObjectPtr idx;
		if(_class->_members->Get(key,idx) && type(idx) == OT_INTEGER) {
            _values[_integer(idx)] = val;
			return true;
		}
		return false;
	}
	void Release() { 
		if (_hook) { _hook(_userpointer,0);}
		sq_delete(this, SQInstance);
	}
	void Finalize();
	void Mark(SQCollectable ** );
	bool InstanceOf(SQClass *trg);
	bool GetMetaMethod(SQMetaMethod mm,SQObjectPtr &res);

	SQClass *_class;
	SQUserPointer _userpointer;
	SQRELEASEHOOK _hook;
	SQObjectPtrVec _values;
};

#endif //_SQCLASS_H_
