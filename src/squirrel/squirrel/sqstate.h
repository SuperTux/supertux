/*	see copyright notice in squirrel.h */
#ifndef _SQSTATE_H_
#define _SQSTATE_H_

#include "squtils.h"
#include "sqobject.h"
struct SQString;
struct SQTable;
//max number of character for a printed number
#define NUMBER_MAX_CHAR 50

struct StringTable
{
	StringTable();
	~StringTable();
	//return a string obj if exists
	//so when there is a table query, if the string doesn't exists in the global state
	//it cannot be in a table so the result will be always null
	//SQString *get(const SQChar *news);
	SQString *Add(const SQChar *,int len);
	void Remove(SQString *);
private:
	void Resize(int size);
	void AllocNodes(int size);
	SQString **_strings;
	unsigned int _numofslots;
	unsigned int _slotused;
};

#define ADD_STRING(ss,str,len) ss->_stringtable->Add(str,len)
#define REMOVE_STRING(ss,bstr) ss->_stringtable->Remove(bstr)

struct SQObjectPtr;

struct SQSharedState
{
	SQSharedState();
	~SQSharedState();
	void Init();
public:
	SQChar* GetScratchPad(int size);
	SQInteger GetMetaMethodIdxByName(const SQObjectPtr &name);
#ifndef NO_GARBAGE_COLLECTOR
	int CollectGarbage(SQVM *vm); 
	static void MarkObject(SQObjectPtr &o,SQCollectable **chain);
#endif
	SQObjectPtrVec *_metamethods;
	SQObjectPtr _metamethodsmap;
	SQObjectPtrVec *_systemstrings;
	SQObjectPtrVec *_types;
	StringTable *_stringtable;
	SQObjectPtr _refs_table;
	SQObjectPtr _registry;
	SQObjectPtr _constructoridx;
#ifndef NO_GARBAGE_COLLECTOR
	SQCollectable *_gc_chain;
#endif
	SQObjectPtr _root_vm;
	SQObjectPtr _table_default_delegate;
	static SQRegFunction _table_default_delegate_funcz[];
	SQObjectPtr _array_default_delegate;
	static SQRegFunction _array_default_delegate_funcz[];
	SQObjectPtr _string_default_delegate;
	static SQRegFunction _string_default_delegate_funcz[];
	SQObjectPtr _number_default_delegate;
	static SQRegFunction _number_default_delegate_funcz[];
	SQObjectPtr _generator_default_delegate;
	static SQRegFunction _generator_default_delegate_funcz[];
	SQObjectPtr _closure_default_delegate;
	static SQRegFunction _closure_default_delegate_funcz[];
	SQObjectPtr _thread_default_delegate;
	static SQRegFunction _thread_default_delegate_funcz[];
	SQObjectPtr _class_default_delegate;
	static SQRegFunction _class_default_delegate_funcz[];
	SQObjectPtr _instance_default_delegate;
	static SQRegFunction _instance_default_delegate_funcz[];
	
	SQCOMPILERERROR _compilererrorhandler;
	SQPRINTFUNCTION _printfunc;
	bool _debuginfo;
private:
	SQChar *_scratchpad;
	int _scratchpadsize;
};

#define _sp(s) (_sharedstate->GetScratchPad(s))
#define _spval (_sharedstate->GetScratchPad(-1))

#define _table_ddel		_table(_sharedstate->_table_default_delegate) 
#define _array_ddel		_table(_sharedstate->_array_default_delegate) 
#define _string_ddel	_table(_sharedstate->_string_default_delegate) 
#define _number_ddel	_table(_sharedstate->_number_default_delegate) 
#define _generator_ddel	_table(_sharedstate->_generator_default_delegate) 
#define _closure_ddel	_table(_sharedstate->_closure_default_delegate) 
#define _thread_ddel	_table(_sharedstate->_thread_default_delegate) 
#define _class_ddel		_table(_sharedstate->_class_default_delegate) 
#define _instance_ddel	_table(_sharedstate->_instance_default_delegate) 

#ifdef SQUNICODE //rsl REAL STRING LEN
#define rsl(l) ((l)<<1)
#else
#define rsl(l) (l)
#endif

extern SQObjectPtr _null_;
extern SQObjectPtr _true_;
extern SQObjectPtr _false_;
extern SQObjectPtr _one_;
extern SQObjectPtr _minusone_;

bool CompileTypemask(SQIntVec &res,const SQChar *typemask);

void *sq_vm_malloc(unsigned int size);
void *sq_vm_realloc(void *p,unsigned int oldsize,unsigned int size);
void sq_vm_free(void *p,unsigned int size);
#endif //_SQSTATE_H_
