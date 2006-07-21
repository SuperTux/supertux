/*	see copyright notice in squirrel.h */
#ifndef _SQFUNCTION_H_
#define _SQFUNCTION_H_

#include "sqopcodes.h"

enum SQOuterType {
	otLOCAL = 0,
	otSYMBOL = 1,
	otOUTER = 2
};

struct SQOuterVar
{

	SQOuterVar(){}
	SQOuterVar(const SQObjectPtr &name,const SQObjectPtr &src,SQOuterType t)
	{
		_name = name;
		_src=src;
		_type=t;
	}
	SQOuterVar(const SQOuterVar &ov)
	{
		_type=ov._type;
		_src=ov._src;
		_name=ov._name;
	}
	SQOuterType _type;
	SQObjectPtr _name;
	SQObjectPtr _src;
};

struct SQLocalVarInfo
{
	SQLocalVarInfo():_start_op(0),_end_op(0){}
	SQLocalVarInfo(const SQLocalVarInfo &lvi)
	{
		_name=lvi._name;
		_start_op=lvi._start_op;
		_end_op=lvi._end_op;
		_pos=lvi._pos;
	}
	SQObjectPtr _name;
	SQUnsignedInteger _start_op;
	SQUnsignedInteger _end_op;
	SQUnsignedInteger _pos;
};

struct SQLineInfo { SQInteger _line;SQInteger _op; };

typedef sqvector<SQOuterVar> SQOuterVarVec;
typedef sqvector<SQLocalVarInfo> SQLocalVarInfoVec;
typedef sqvector<SQLineInfo> SQLineInfoVec;

struct SQFunctionProto : public SQRefCounted
{
private:
	SQFunctionProto(){
	_stacksize=0;
	_bgenerator=false;}
public:
	static SQFunctionProto *Create()
	{
		SQFunctionProto *f;
		sq_new(f,SQFunctionProto);
		return f;
	}
	void Release(){ sq_delete(this,SQFunctionProto);}
	const SQChar* GetLocal(SQVM *v,SQUnsignedInteger stackbase,SQUnsignedInteger nseq,SQUnsignedInteger nop);
	SQInteger GetLine(SQInstruction *curr);
	bool Save(SQVM *v,SQUserPointer up,SQWRITEFUNC write);
	bool Load(SQVM *v,SQUserPointer up,SQREADFUNC read);
	SQObjectPtrVec _literals;
	SQObjectPtrVec _functions;
	SQObjectPtrVec _parameters;
	SQOuterVarVec _outervalues;
	SQInstructionVec _instructions;
	SQObjectPtr _sourcename;
	SQObjectPtr _name;
	SQLocalVarInfoVec _localvarinfos;
	SQLineInfoVec _lineinfos;
    SQInteger _stacksize;
	bool _bgenerator;
	bool _varparams;
};

#endif //_SQFUNCTION_H_
