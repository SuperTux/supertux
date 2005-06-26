/*
	see copyright notice in squirrel.h
*/
#include "sqpcheader.h"
#include "sqvm.h"
#include "sqstring.h"
#include "sqarray.h"
#include "sqtable.h"
#include "squserdata.h"
#include "sqfuncproto.h"
#include "sqclass.h"
#include "sqclosure.h"

SQString *SQString::Create(SQSharedState *ss,const SQChar *s,int len)
{
	SQString *str=ADD_STRING(ss,s,len);
	str->_sharedstate=ss;
	return str;
}

void SQString::Release()
{
	REMOVE_STRING(_sharedstate,this);
}

unsigned int TranslateIndex(const SQObjectPtr &idx)
{
	switch(type(idx)){
		case OT_NULL:
			return 0;
		case OT_INTEGER:
			return (unsigned int)_integer(idx);
	}
	assert(0);
	return 0;
}

bool SQDelegable::GetMetaMethod(SQMetaMethod mm,SQObjectPtr &res) {
	if(_delegate) {
		return _delegate->Get((*_ss(this)->_metamethods)[mm],res);
	}
	return false;
}

bool SQGenerator::Yield(SQVM *v)
{
	if(_state==eSuspended) { v->Raise_Error(_SC("internal vm error, yielding dead generator"));  return false;}
	if(_state==eDead) { v->Raise_Error(_SC("internal vm error, yielding a dead generator")); return false; }
	int size = v->_top-v->_stackbase;
	_ci=*v->ci;
	_stack.resize(size);
	for(int n =0; n<size; n++) {
		_stack._vals[n] = v->_stack[v->_stackbase+n];
		v->_stack[v->_stackbase+n] = _null_;
	}
	int nvargs = v->ci->_vargs.size;
	int vargsbase = v->ci->_vargs.base;
	for(int j = nvargs - 1; j >= 0; j--) {
		_vargsstack.push_back(v->_vargsstack[vargsbase+j]);
	}
	_ci._generator=_null_;
	for(int i=0;i<_ci._etraps;i++) {
		_etraps.push_back(v->_etraps.top());
		v->_etraps.pop_back();
	}
	_state=eSuspended;
	return true;
}

bool SQGenerator::Resume(SQVM *v,int target)
{
	int size=_stack.size();
	if(_state==eDead){ v->Raise_Error(_SC("resuming dead generator")); return false; }
	if(_state==eRunning){ v->Raise_Error(_SC("resuming active generator")); return false; }
	int prevtop=v->_top-v->_stackbase;
	PUSH_CALLINFO(v,_ci);
	int oldstackbase=v->_stackbase;
	v->_stackbase=v->_top;
	v->ci->_target=target;
	v->ci->_generator=SQObjectPtr(this);
	v->ci->_vargs.size = _vargsstack.size();
	
	for(int i=0;i<_ci._etraps;i++) {
		v->_etraps.push_back(_etraps.top());
		_etraps.pop_back();
	}
	for(int n =0; n<size; n++) {
		v->_stack[v->_stackbase+n] = _stack._vals[n];
		_stack._vals[0] = _null_;
	}
	while(_vargsstack.size()) {
		v->_vargsstack.push_back(_vargsstack.back());
		_vargsstack.pop_back();
	}
	v->ci->_vargs.base = v->_vargsstack.size() - v->ci->_vargs.size;
	v->_top=v->_stackbase+size;
	v->ci->_prevtop=prevtop;
	v->ci->_prevstkbase=v->_stackbase-oldstackbase;
	_state=eRunning;
	return true;
}

void SQArray::Extend(const SQArray *a){
	int xlen;
	if((xlen=a->Size()))
		for(int i=0;i<xlen;i++)
			Append(a->_values[i]);
}

const SQChar* SQFunctionProto::GetLocal(SQVM *vm,unsigned int stackbase,unsigned int nseq,unsigned int nop)
{
	unsigned int nvars=_localvarinfos.size();
	const SQChar *res=NULL; 
	if(nvars>=nseq){
 		for(unsigned int i=0;i<nvars;i++){
			if(_localvarinfos[i]._start_op<=nop && _localvarinfos[i]._end_op>=nop)
			{
				if(nseq==0){
					vm->Push(vm->_stack[stackbase+_localvarinfos[i]._pos]);
					res=_stringval(_localvarinfos[i]._name);
					break;
				}
				nseq--;
			}
		}
	}
	return res;
}

int SQFunctionProto::GetLine(SQInstruction *curr)
{
	int op=(curr-_instructions._vals);
	int line=_lineinfos[0]._line;
	for(unsigned int i=1;i<_lineinfos.size();i++){
		if(_lineinfos[i]._op>=op)
			return line;
		line=_lineinfos[i]._line;
	}
	return line;
}

//#define _ERROR_TRAP() error_trap:
#define _CHECK_IO(exp)  { if(!exp)return false; }
bool SafeWrite(HSQUIRRELVM v,SQWRITEFUNC write,SQUserPointer up,SQUserPointer dest,int size)
{
	if(write(up,dest,size) != size) {
		v->Raise_Error(_SC("io error (write function failure)"));
		return false;
	}
	return true;
}

bool SafeRead(HSQUIRRELVM v,SQWRITEFUNC read,SQUserPointer up,SQUserPointer dest,int size)
{
	if(size && read(up,dest,size) != size) {
		v->Raise_Error(_SC("io error, read function failure, the origin stream could be corrupted/trucated"));
		return false;
	}
	return true;
}

bool WriteTag(HSQUIRRELVM v,SQWRITEFUNC write,SQUserPointer up,int tag)
{
	return SafeWrite(v,write,up,&tag,sizeof(tag));
}

bool CheckTag(HSQUIRRELVM v,SQWRITEFUNC read,SQUserPointer up,int tag)
{
	int t;
	_CHECK_IO(SafeRead(v,read,up,&t,sizeof(t)));
	if(t != tag){
		v->Raise_Error(_SC("invalid or corrupted closure stream"));
		return false;
	}
	return true;
}

bool WriteObject(HSQUIRRELVM v,SQUserPointer up,SQWRITEFUNC write,SQObjectPtr &o)
{
	_CHECK_IO(SafeWrite(v,write,up,&type(o),sizeof(SQObjectType)));
	switch(type(o)){
	case OT_STRING:
		_CHECK_IO(SafeWrite(v,write,up,&_string(o)->_len,sizeof(SQInteger)));
		_CHECK_IO(SafeWrite(v,write,up,_stringval(o),rsl(_string(o)->_len)));
		break;
	case OT_INTEGER:
		_CHECK_IO(SafeWrite(v,write,up,&_integer(o),sizeof(SQInteger)));break;
	case OT_FLOAT:
		_CHECK_IO(SafeWrite(v,write,up,&_float(o),sizeof(SQFloat)));break;
	case OT_NULL:
		break;
	default:
		v->Raise_Error(_SC("cannot serialize a %s"),GetTypeName(o));
		return false;
	}
	return true;
}

bool ReadObject(HSQUIRRELVM v,SQUserPointer up,SQREADFUNC read,SQObjectPtr &o)
{
	SQObjectType t;
	_CHECK_IO(SafeRead(v,read,up,&t,sizeof(SQObjectType)));
	switch(t){
	case OT_STRING:{
		int len;
		_CHECK_IO(SafeRead(v,read,up,&len,sizeof(SQInteger)));
		_CHECK_IO(SafeRead(v,read,up,_ss(v)->GetScratchPad(rsl(len)),rsl(len)));
		o=SQString::Create(_ss(v),_ss(v)->GetScratchPad(-1),len);
				   }
		break;
	case OT_INTEGER:{
		SQInteger i;
		_CHECK_IO(SafeRead(v,read,up,&i,sizeof(SQInteger))); o = i; break;
					}
	case OT_FLOAT:{
		SQFloat f;
		_CHECK_IO(SafeRead(v,read,up,&f,sizeof(SQFloat))); o = f; break;
				  }
	case OT_NULL:
		o=_null_;
		break;
	default:
		v->Raise_Error(_SC("cannot serialize a %s"),IdType2Name(t));
		return false;
	}
	return true;
}

bool SQClosure::Save(SQVM *v,SQUserPointer up,SQWRITEFUNC write)
{
	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_HEAD));
	_CHECK_IO(WriteTag(v,write,up,sizeof(SQChar)));
	_CHECK_IO(_funcproto(_function)->Save(v,up,write));
	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_TAIL));
	return true;
}

bool SQClosure::Load(SQVM *v,SQUserPointer up,SQREADFUNC read)
{
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_HEAD));
	_CHECK_IO(CheckTag(v,read,up,sizeof(SQChar)));
	_CHECK_IO(_funcproto(_function)->Load(v,up,read));
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_TAIL));
	return true;
}

bool SQFunctionProto::Save(SQVM *v,SQUserPointer up,SQWRITEFUNC write)
{
	int i,nsize=_literals.size();
	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART));
	_CHECK_IO(WriteObject(v,up,write,_sourcename));
	_CHECK_IO(WriteObject(v,up,write,_name));
	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART));
	_CHECK_IO(SafeWrite(v,write,up,&nsize,sizeof(nsize)));
	for(i=0;i<nsize;i++){
		_CHECK_IO(WriteObject(v,up,write,_literals[i]));
	}
	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART));
	nsize=_parameters.size();
	_CHECK_IO(SafeWrite(v,write,up,&nsize,sizeof(nsize)));
	for(i=0;i<nsize;i++){
		_CHECK_IO(WriteObject(v,up,write,_parameters[i]));
	}
	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART));
	nsize=_outervalues.size();
	_CHECK_IO(SafeWrite(v,write,up,&nsize,sizeof(nsize)));
	for(i=0;i<nsize;i++){
		_CHECK_IO(SafeWrite(v,write,up,&_outervalues[i]._type,sizeof(unsigned int)));
		_CHECK_IO(WriteObject(v,up,write,_outervalues[i]._src));
		_CHECK_IO(WriteObject(v,up,write,_outervalues[i]._name));
	}
	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART));
	nsize=_localvarinfos.size();
	_CHECK_IO(SafeWrite(v,write,up,&nsize,sizeof(nsize)));
	for(i=0;i<nsize;i++){
		SQLocalVarInfo &lvi=_localvarinfos[i];
		_CHECK_IO(WriteObject(v,up,write,lvi._name));
		_CHECK_IO(SafeWrite(v,write,up,&lvi._pos,sizeof(unsigned int)));
		_CHECK_IO(SafeWrite(v,write,up,&lvi._start_op,sizeof(unsigned int)));
		_CHECK_IO(SafeWrite(v,write,up,&lvi._end_op,sizeof(unsigned int)));
	}
	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART));
	nsize=_lineinfos.size();
	_CHECK_IO(SafeWrite(v,write,up,&nsize,sizeof(nsize)));
	_CHECK_IO(SafeWrite(v,write,up,&_lineinfos[0],sizeof(SQLineInfo)*nsize));
	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART));
	nsize=_instructions.size();
	_CHECK_IO(SafeWrite(v,write,up,&nsize,sizeof(nsize)));
	_CHECK_IO(SafeWrite(v,write,up,&_instructions[0],sizeof(SQInstruction)*nsize));
	_CHECK_IO(WriteTag(v,write,up,SQ_CLOSURESTREAM_PART));
	nsize=_functions.size();
	_CHECK_IO(SafeWrite(v,write,up,&nsize,sizeof(nsize)));
	for(i=0;i<nsize;i++){
		_CHECK_IO(_funcproto(_functions[i])->Save(v,up,write));
	}
	_CHECK_IO(SafeWrite(v,write,up,&_stacksize,sizeof(_stacksize)));
	_CHECK_IO(SafeWrite(v,write,up,&_bgenerator,sizeof(_bgenerator)));
	_CHECK_IO(SafeWrite(v,write,up,&_varparams,sizeof(_varparams)));
	return true;
}

bool SQFunctionProto::Load(SQVM *v,SQUserPointer up,SQREADFUNC read)
{
	int i, nsize = _literals.size();
	SQObjectPtr o;
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
	_CHECK_IO(ReadObject(v, up, read, _sourcename));
	_CHECK_IO(ReadObject(v, up, read, _name));
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
	_CHECK_IO(SafeRead(v,read,up, &nsize, sizeof(nsize)));
	for(i = 0;i < nsize; i++){
		_CHECK_IO(ReadObject(v, up, read, o));
		_literals.push_back(o);
	}
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
	_CHECK_IO(SafeRead(v,read,up, &nsize, sizeof(nsize)));
	for(i = 0; i < nsize; i++){
		_CHECK_IO(ReadObject(v, up, read, o));
		_parameters.push_back(o);
	}
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
	_CHECK_IO(SafeRead(v,read,up,&nsize,sizeof(nsize)));
	for(i = 0; i < nsize; i++){
		unsigned int type;
		SQObjectPtr name;
		_CHECK_IO(SafeRead(v,read,up, &type, sizeof(unsigned int)));
		_CHECK_IO(ReadObject(v, up, read, o));
		_CHECK_IO(ReadObject(v, up, read, name));
		_outervalues.push_back(SQOuterVar(name,o, (SQOuterType)type));
	}
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
	_CHECK_IO(SafeRead(v,read,up,&nsize, sizeof(nsize)));
	for(i = 0; i < nsize; i++){
		SQLocalVarInfo lvi;
		_CHECK_IO(ReadObject(v, up, read, lvi._name));
		_CHECK_IO(SafeRead(v,read,up, &lvi._pos, sizeof(unsigned int)));
		_CHECK_IO(SafeRead(v,read,up, &lvi._start_op, sizeof(unsigned int)));
		_CHECK_IO(SafeRead(v,read,up, &lvi._end_op, sizeof(unsigned int)));
		_localvarinfos.push_back(lvi);
	}
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
	_CHECK_IO(SafeRead(v,read,up, &nsize,sizeof(nsize)));
	_lineinfos.resize(nsize);
	_CHECK_IO(SafeRead(v,read,up, &_lineinfos[0], sizeof(SQLineInfo)*nsize));
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
	_CHECK_IO(SafeRead(v,read,up, &nsize, sizeof(nsize)));
	_instructions.resize(nsize);
	_CHECK_IO(SafeRead(v,read,up, &_instructions[0], sizeof(SQInstruction)*nsize));
	_CHECK_IO(CheckTag(v,read,up,SQ_CLOSURESTREAM_PART));
	_CHECK_IO(SafeRead(v,read,up, &nsize, sizeof(nsize)));
	for(i = 0; i < nsize; i++){
		o = SQFunctionProto::Create();
		_CHECK_IO(_funcproto(o)->Load(v, up, read));
		_functions.push_back(o);
	}
	_CHECK_IO(SafeRead(v,read,up, &_stacksize, sizeof(_stacksize)));
	_CHECK_IO(SafeRead(v,read,up, &_bgenerator, sizeof(_bgenerator)));
	_CHECK_IO(SafeRead(v,read,up, &_varparams, sizeof(_varparams)));
	return true;
}

#ifndef NO_GARBAGE_COLLECTOR

#define START_MARK() 	if(!(_uiRef&MARK_FLAG)){ \
		_uiRef|=MARK_FLAG;

#define END_MARK() RemoveFromChain(&_sharedstate->_gc_chain, this); \
		AddToChain(chain, this); }

void SQVM::Mark(SQCollectable **chain)
{
	START_MARK()
		SQSharedState::MarkObject(_lasterror,chain);
		SQSharedState::MarkObject(_errorhandler,chain);
		SQSharedState::MarkObject(_debughook,chain);
		SQSharedState::MarkObject(_roottable, chain);
		SQSharedState::MarkObject(temp_reg, chain);
		for(unsigned int i = 0; i < _stack.size(); i++) SQSharedState::MarkObject(_stack[i], chain);
		for(unsigned int j = 0; j < _vargsstack.size(); j++) SQSharedState::MarkObject(_vargsstack[j], chain);
	END_MARK()
}

void SQArray::Mark(SQCollectable **chain)
{
	START_MARK()
		int len = _values.size();
		for(int i = 0;i < len; i++) SQSharedState::MarkObject(_values[i], chain);
	END_MARK()
}
void SQTable::Mark(SQCollectable **chain)
{
	START_MARK()
		if(_delegate) _delegate->Mark(chain);
		int len = _numofnodes;
		for(int i = 0; i < len; i++){
			SQSharedState::MarkObject(_nodes[i].key, chain);
			SQSharedState::MarkObject(_nodes[i].val, chain);
		}
	END_MARK()
}

void SQClass::Mark(SQCollectable **chain)
{
	START_MARK()
		_members->Mark(chain);
		if(_base) _base->Mark(chain);
		SQSharedState::MarkObject(_attributes, chain);
		for(unsigned int i =0; i< _defaultvalues.size(); i++) {
			SQSharedState::MarkObject(_defaultvalues[i].val, chain);
			SQSharedState::MarkObject(_defaultvalues[i].attrs, chain);
		}
		for(unsigned int j =0; j< _methods.size(); j++) {
			SQSharedState::MarkObject(_methods[j].val, chain);
			SQSharedState::MarkObject(_methods[j].attrs, chain);
		}
		for(unsigned int k =0; k< _metamethods.size(); k++) {
			SQSharedState::MarkObject(_metamethods[k], chain);
		}
	END_MARK()
}

void SQInstance::Mark(SQCollectable **chain)
{
	START_MARK()
		_class->Mark(chain);
		for(unsigned int i =0; i< _values.size(); i++) {
			SQSharedState::MarkObject(_values[i], chain);
		}
	END_MARK()
}

void SQGenerator::Mark(SQCollectable **chain)
{
	START_MARK()
		for(unsigned int i = 0; i < _stack.size(); i++) SQSharedState::MarkObject(_stack[i], chain);
		for(unsigned int j = 0; j < _vargsstack.size(); j++) SQSharedState::MarkObject(_vargsstack[j], chain);
		SQSharedState::MarkObject(_closure, chain);
	END_MARK()
}

void SQClosure::Mark(SQCollectable **chain)
{
	START_MARK()
		for(unsigned int i = 0; i < _outervalues.size(); i++) SQSharedState::MarkObject(_outervalues[i], chain);
	END_MARK()
}

void SQNativeClosure::Mark(SQCollectable **chain)
{
	START_MARK()
		for(unsigned int i = 0; i < _outervalues.size(); i++) SQSharedState::MarkObject(_outervalues[i], chain);
	END_MARK()
}

void SQUserData::Mark(SQCollectable **chain){
	START_MARK()
		if(_delegate) _delegate->Mark(chain);
	END_MARK()
}

void SQCollectable::UnMark() { _uiRef&=~MARK_FLAG; }

#endif

