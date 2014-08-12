#ifndef _SQ_DBGSERVER_H_
#define _SQ_DBGSERVER_H_

#define MAX_BP_PATH 512
#define MAX_MSG_LEN 2049

#include <set>
#include <map>
#include <string>
#include <vector>
#include <winsock.h>

typedef std::basic_string<SQChar> SQDBGString;

struct BreakPoint{
	BreakPoint(){_line=0;}
	BreakPoint(SQInteger line, const SQChar *src){ _line = line; _src = src; }
	BreakPoint(const BreakPoint& bp){ _line = bp._line; _src=bp._src; }
	bool operator<(const BreakPoint& bp) const
	{
		if(_line<bp._line)
			return true;
		if(_line==bp._line){
			if(_src<bp._src){
				return true;
			}
			return false;
		}
		return false;
	}
	bool operator==(const BreakPoint& other)
	{
		if(_line==other._line
			&& (_src==other._src))
			return true;
		return false;
	}
	SQInteger _line;
	SQDBGString _src;
};

struct Watch{
	Watch() { _id = 0; }
	Watch(SQInteger id,const SQChar *exp) { _id = id; _exp = exp; }
	Watch(const Watch &w) { _id = w._id; _exp = w._exp; }
	bool operator<(const Watch& w) const { return _id<w._id; }
	bool operator==(const Watch& w) const { return _id == w._id; }
	SQInteger _id;
	SQDBGString _exp;
};

struct VMState {
	VMState() { _nestedcalls = 0;}
	SQInteger _nestedcalls;
};
typedef std::map<HSQUIRRELVM,VMState*> VMStateMap;
typedef std::set<BreakPoint> BreakPointSet;
typedef BreakPointSet::iterator BreakPointSetItor;

typedef std::set<Watch> WatchSet;
typedef WatchSet::iterator WatchSetItor;

typedef std::vector<SQChar> SQCharVec;
struct SQDbgServer{
public:
	enum eDbgState{
		eDBG_Running,
		eDBG_StepOver,
		eDBG_StepInto,
		eDBG_StepReturn,
		eDBG_Suspended,
		eDBG_Disabled,
	};

	SQDbgServer(HSQUIRRELVM v);
	~SQDbgServer();
	bool Init();
	//returns true if a message has been received
	bool WaitForClient();
	bool ReadMsg();
	void BusyWait();
	void Hook(HSQUIRRELVM v,SQInteger type,SQInteger line,const SQChar *src,const SQChar *func);
	void ParseMsg(const char *msg);
	bool ParseBreakpoint(const char *msg,BreakPoint &out);
	bool ParseWatch(const char *msg,Watch &out);
	bool ParseRemoveWatch(const char *msg,SQInteger &id);
	void Terminated();
	//
	void BreakExecution();
	void Send(const SQChar *s,...);
	void SendChunk(const SQChar *chunk);
	void Break(HSQUIRRELVM v,SQInteger line,const SQChar *src,const SQChar *type,const SQChar *error=NULL);
	

	void SerializeState(HSQUIRRELVM v);
	//COMMANDS
	void AddBreakpoint(BreakPoint &bp);
	void AddWatch(Watch &w);
	void RemoveWatch(SQInteger id);
	void RemoveBreakpoint(BreakPoint &bp);

	//
	void SetErrorHandlers(HSQUIRRELVM v);
	VMState *GetVMState(HSQUIRRELVM v);

	//XML RELATED STUFF///////////////////////
	#define MAX_NESTING 10
	struct XMLElementState {
		SQChar name[256];
		bool haschildren;
	};

	XMLElementState xmlstate[MAX_NESTING];
	SQInteger _xmlcurrentement;

	void BeginDocument();
	void BeginElement(const SQChar *name);
	void Attribute(const SQChar *name, const SQChar *value);
	void EndElement(const SQChar *name);
	void EndDocument();

	const SQChar *escape_xml(const SQChar *x);
	//////////////////////////////////////////////
	HSQUIRRELVM _v;
	HSQOBJECT _debugroot;
	eDbgState _state;
	SOCKET _accept;
	SOCKET _endpoint;
	BreakPointSet _breakpoints;
	WatchSet _watches;
	//int _recursionlevel; 
	//int _maxrecursion;
	
	bool _ready;
	bool _autoupdate;
	HSQOBJECT _serializefunc;
	SQCharVec _scratchstring;

	SQInteger _line;
	SQDBGString _src;
	SQDBGString _break_type;
	VMStateMap _vmstate;	
};

#ifdef _WIN32
#define sqdbg_closesocket(x) closesocket((x))
#else
#define sqdbg_closesocket(x) close((x))
#endif

#endif //_SQ_DBGSERVER_H_ 