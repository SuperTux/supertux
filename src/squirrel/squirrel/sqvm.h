/*	see copyright notice in squirrel.h */
#ifndef _SQVM_H_
#define _SQVM_H_

#include "sqopcodes.h"
#include "sqobject.h"
#define MAX_NATIVE_CALLS 100
#define MIN_STACK_OVERHEAD 10

#define SQ_SUSPEND_FLAG -666
//base lib
void sq_base_register(HSQUIRRELVM v);

struct SQExceptionTrap{
	SQExceptionTrap() {}
	SQExceptionTrap(SQInteger ss, SQInteger stackbase,SQInstruction *ip, SQInteger ex_target){ _stacksize = ss; _stackbase = stackbase; _ip = ip; _extarget = ex_target;}
	SQExceptionTrap(const SQExceptionTrap &et) { (*this) = et;	}
	SQInteger _stackbase;
	SQInteger _stacksize;
	SQInstruction *_ip;
	SQInteger _extarget;
};


#define STK(a) _stack._vals[_stackbase+(a)]
#define TARGET _stack._vals[_stackbase+arg0]

typedef sqvector<SQExceptionTrap> ExceptionsTraps;

struct SQVM : public CHAINABLE_OBJ
{
	struct VarArgs {
		VarArgs() { size = 0; base = 0; }
		SQInteger size;
		SQInteger base;
	};

	struct CallInfo{
		CallInfo() { _generator._type = OT_NULL;}
		//CallInfo(const CallInfo& ci) {  }
		SQInstructionVec *_iv;
		SQObjectPtrVec *_literals;
		SQObject _closure;
		SQObject _generator;
		SQInteger _etraps;
		SQInteger _prevstkbase;
		SQInteger _prevtop;
		SQInteger _target;
		SQInstruction *_ip;
		SQInteger _ncalls;
		SQBool _root;
		VarArgs _vargs;
	};

typedef sqvector<CallInfo> CallInfoVec;
public:
	enum ExecutionType { ET_CALL, ET_RESUME_GENERATOR, ET_RESUME_VM };
	SQVM(SQSharedState *ss);
	~SQVM();
	bool Init(SQVM *friendvm, SQInteger stacksize);
	bool Execute(SQObjectPtr &func, SQInteger target, SQInteger nargs, SQInteger stackbase, SQObjectPtr &outres, ExecutionType et = ET_CALL);
	//start a native call return when the NATIVE closure returns(returns true if the vm has been suspended)
	bool CallNative(SQNativeClosure *nclosure, SQInteger nargs, SQInteger stackbase, bool tailcall, SQObjectPtr &retval,bool &suspend);
	//start a SQUIRREL call in the same "Execution loop"
	bool StartCall(SQClosure *closure, SQInteger target, SQInteger nargs, SQInteger stackbase, bool tailcall);
	bool CreateClassInstance(SQClass *theclass, SQInteger nargs, SQInteger stackbase, SQObjectPtr &retval);
	//call a generic closure pure SQUIRREL or NATIVE
	bool Call(SQObjectPtr &closure, SQInteger nparams, SQInteger stackbase, SQObjectPtr &outres);
	SQRESULT Suspend();

	void CallDebugHook(SQInteger type,SQInteger forcedline=0);
	void CallErrorHandler(SQObjectPtr &e);
	bool Get(const SQObjectPtr &self, const SQObjectPtr &key, SQObjectPtr &dest, bool raw, bool fetchroot);
	bool FallBackGet(const SQObjectPtr &self,const SQObjectPtr &key,SQObjectPtr &dest,bool raw);
	bool Set(const SQObjectPtr &self, const SQObjectPtr &key, const SQObjectPtr &val, bool fetchroot);
	bool NewSlot(const SQObjectPtr &self, const SQObjectPtr &key, const SQObjectPtr &val);
	bool DeleteSlot(const SQObjectPtr &self, const SQObjectPtr &key, SQObjectPtr &res);
	bool Clone(const SQObjectPtr &self, SQObjectPtr &target);
	bool ObjCmp(const SQObjectPtr &o1, const SQObjectPtr &o2,SQInteger &res);
	bool StringCat(const SQObjectPtr &str, const SQObjectPtr &obj, SQObjectPtr &dest);
	bool IsEqual(SQObjectPtr &o1,SQObjectPtr &o2,bool &res);
	void ToString(const SQObjectPtr &o,SQObjectPtr &res);
	SQString *PrintObjVal(const SQObject &o);

 
	void Raise_Error(const SQChar *s, ...);
	void Raise_Error(SQObjectPtr &desc);
	void Raise_IdxError(SQObject &o);
	void Raise_CompareError(const SQObject &o1, const SQObject &o2);
	void Raise_ParamTypeError(SQInteger nparam,SQInteger typemask,SQInteger type);

	void TypeOf(const SQObjectPtr &obj1, SQObjectPtr &dest);
	bool CallMetaMethod(SQDelegable *del, SQMetaMethod mm, SQInteger nparams, SQObjectPtr &outres);
	bool ArithMetaMethod(SQInteger op, const SQObjectPtr &o1, const SQObjectPtr &o2, SQObjectPtr &dest);
	bool Return(SQInteger _arg0, SQInteger _arg1, SQObjectPtr &retval);
	//new stuff
	inline bool ARITH_OP(SQUnsignedInteger op,SQObjectPtr &trg,const SQObjectPtr &o1,const SQObjectPtr &o2);
	inline bool BW_OP(SQUnsignedInteger op,SQObjectPtr &trg,const SQObjectPtr &o1,const SQObjectPtr &o2);
	inline bool NEG_OP(SQObjectPtr &trg,const SQObjectPtr &o1);
	inline bool CMP_OP(CmpOP op, const SQObjectPtr &o1,const SQObjectPtr &o2,SQObjectPtr &res);
	bool CLOSURE_OP(SQObjectPtr &target, SQFunctionProto *func);
	bool GETVARGV_OP(SQObjectPtr &target,SQObjectPtr &idx,CallInfo *ci);
	bool CLASS_OP(SQObjectPtr &target,SQInteger base,SQInteger attrs);
	//return true if the loop is finished
	bool FOREACH_OP(SQObjectPtr &o1,SQObjectPtr &o2,SQObjectPtr &o3,SQObjectPtr &o4,SQInteger arg_2,bool &finished);
	bool DELEGATE_OP(SQObjectPtr &trg,SQObjectPtr &o1,SQObjectPtr &o2);
	inline bool LOCAL_INC(SQInteger op,SQObjectPtr &target, SQObjectPtr &a, SQObjectPtr &incr);
	inline bool PLOCAL_INC(SQInteger op,SQObjectPtr &target, SQObjectPtr &a, SQObjectPtr &incr);
	inline bool DerefInc(SQInteger op,SQObjectPtr &target, SQObjectPtr &self, SQObjectPtr &key, SQObjectPtr &incr, bool postfix);
	void PopVarArgs(VarArgs &vargs);
#ifdef _DEBUG_DUMP
	void dumpstack(SQInteger stackbase=-1, bool dumpall = false);
#endif

#ifndef NO_GARBAGE_COLLECTOR
	void Mark(SQCollectable **chain);
#endif
	void Finalize();

	void Release(){ sq_delete(this,SQVM); } //does nothing
////////////////////////////////////////////////////////////////////////////
	//stack functions for the api
	void Remove(SQInteger n);

	inline bool IsFalse(SQObjectPtr &o)
	{
		if((type(o) & SQOBJECT_CANBEFALSE) && ( (type(o) == OT_FLOAT) && (_float(o) == SQFloat(0.0)) )
			|| (_integer(o) == 0) ) { //OT_NULL|OT_INTEGER|OT_BOOL
			return true;
		}
		return false;
	}
	inline void Pop() {
		_stack[--_top] = _null_;
	}

	inline void Pop(SQInteger n) {
		for(SQInteger i = 0; i < n; i++){
			_stack[--_top] = _null_;
		}
	}

	inline void Push(const SQObjectPtr &o) { _stack[_top++] = o; }
	inline SQObjectPtr &Top() { return _stack[_top-1]; }
	inline SQObjectPtr &PopGet() { return _stack[--_top]; }
	inline SQObjectPtr &GetUp(SQInteger n) { return _stack[_top+n]; }
	inline SQObjectPtr &GetAt(SQInteger n) { return _stack[n]; }

	SQObjectPtrVec _stack;
	SQObjectPtrVec _vargsstack;
	SQInteger _top;
	SQInteger _stackbase;
	SQObjectPtr _roottable;
	//SQObjectPtr _thrownerror;
	SQObjectPtr _lasterror;
	SQObjectPtr _errorhandler;
	SQObjectPtr _debughook;

	SQObjectPtr temp_reg;
	CallInfoVec _callsstack;
	ExceptionsTraps _etraps;
	CallInfo *ci;
	void *_foreignptr;
	//VMs sharing the same state
	SQSharedState *_sharedstate;
	SQInteger _nnativecalls;
	//suspend infos
	SQBool _suspended;
	SQBool _suspended_root;
	SQInteger _suspended_target;
	SQInteger _suspended_traps;
};

struct AutoDec{
	AutoDec(SQInteger *n) { _n = n; }
	~AutoDec() { (*_n)--; }
	SQInteger *_n;
};

inline SQObjectPtr &stack_get(HSQUIRRELVM v,SQInteger idx){return ((idx>=0)?(v->GetAt(idx+v->_stackbase-1)):(v->GetUp(idx)));}
const SQChar *GetTypeName(const SQObjectPtr &obj1);
const SQChar *IdType2Name(SQObjectType type);

#define _ss(_vm_) (_vm_)->_sharedstate

#ifndef NO_GARBAGE_COLLECTOR
#define _opt_ss(_vm_) (_vm_)->_sharedstate
#else
#define _opt_ss(_vm_) NULL
#endif

#define PUSH_CALLINFO(v,nci){ \
	v->ci = &v->_callsstack.push_back(nci); \
}

#define POP_CALLINFO(v){ \
	v->_callsstack.pop_back(); \
	if(v->_callsstack.size())	\
		v->ci = &v->_callsstack.back() ; \
	else	\
		v->ci = NULL; \
}
#endif //_SQVM_H_
