/*	see copyright notice in squirrel.h */
#ifndef _SQFUNCSTATE_H_
#define _SQFUNCSTATE_H_
///////////////////////////////////
#include "squtils.h"



struct SQFuncState
{
	SQFuncState(SQSharedState *ss,SQFunctionProto *func,SQFuncState *parent);
#ifdef _DEBUG_DUMP
	void Dump();
#endif
	void AddInstruction(SQOpcode _op,int arg0=0,int arg1=0,int arg2=0,int arg3=0){SQInstruction i(_op,arg0,arg1,arg2,arg3);AddInstruction(i);}
	void AddInstruction(SQInstruction &i);
	void SetIntructionParams(int pos,int arg0,int arg1,int arg2=0,int arg3=0);
	void SetIntructionParam(int pos,int arg,int val);
	SQInstruction &GetInstruction(int pos){return _instructions[pos];}
	void PopInstructions(int size){for(int i=0;i<size;i++)_instructions.pop_back();}
	void SetStackSize(int n);
	void SnoozeOpt(){_optimization=false;}
	int GetCurrentPos(){return _instructions.size()-1;}
	int GetStringConstant(const SQChar *cons);
	int GetNumericConstant(const SQInteger cons);
	int GetNumericConstant(const SQFloat cons);
	int PushLocalVariable(const SQObjectPtr &name);
	void AddParameter(const SQObjectPtr &name);
	void AddOuterValue(const SQObjectPtr &name);
	int GetLocalVariable(const SQObjectPtr &name);
	int GetOuterVariable(const SQObjectPtr &name);
	int GenerateCode();
	int GetStackSize();
	int CalcStackFrameSize();
	void AddLineInfos(int line,bool lineop,bool force=false);
	void Finalize();
	int AllocStackPos();
	int PushTarget(int n=-1);
	int PopTarget();
	int TopTarget();
	int GetUpTarget(int n);
	bool IsLocal(unsigned int stkpos);
	SQObject CreateString(const SQChar *s);
	int _returnexp;
	SQLocalVarInfoVec _vlocals;
	SQIntVec _targetstack;
	int _stacksize;
	bool _varparams;
	SQIntVec _unresolvedbreaks;
	SQIntVec _unresolvedcontinues;
	SQObjectPtrVec _functions;
	SQObjectPtrVec _parameters;
	SQObjectPtrVec _stringrefs;
	SQOuterVarVec _outervalues;
	SQInstructionVec _instructions;
	SQLocalVarInfoVec _localvarinfos;
	SQObjectPtr _literals;
	SQInteger _nliterals;
	SQLineInfoVec _lineinfos;
	SQObjectPtr _func;
	SQFuncState *_parent;
	SQIntVec _breaktargets; //contains number of nested exception traps
	SQIntVec _continuetargets;
	int _lastline;
	int _traps;
	bool _optimization;
	SQSharedState *_sharedstate;
private:
	int GetConstant(SQObjectPtr cons);
};


#endif //_SQFUNCSTATE_H_

