#ifndef _SQ_DBGSERVER_H_
#define _SQ_DBGSERVER_H_

#define MAX_BP_PATH 512
#define MAX_MSG_LEN 2049

#include <set>
#include <string>
#include <vector>

#ifdef _WIN32
#include <winsock.h>
#define sqdbg_closesocket(x) closesocket((x))
typedef socklen_t int;
#else
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#define sqdbg_closesocket(x) close((x))
typedef int SOCKET;
typedef struct timeval TIMEVAL;
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#endif

typedef std::basic_string<SQChar> SQDBGString;

inline bool dbg_less(const SQChar *x,const SQChar *y)
{
	// [SuperTux] commented out to avoid compiler warning
	//int n = 0;
	do {
		int xl = *x == '\\' ? '/' : tolower(*x);
		int yl = *y == '\\' ? '/' : tolower(*y);
		int diff = xl - yl;
		if(diff != 0)
			return diff > 0?true:false;
		x++; y++;
	}while(*x != 0 && *y != 0);
	return false;
}

struct BreakPoint{
	BreakPoint(){_line=0;}
	BreakPoint(int line, const SQChar *src){ _line = line; _src = src; }
	BreakPoint(const BreakPoint& bp){ _line = bp._line; _src=bp._src; }
	inline bool operator<(const BreakPoint& bp) const
	{
		if(_line<bp._line)
			return true;
		if(_line==bp._line){
			return dbg_less(_src.c_str(),bp._src.c_str());
		}
		return false;
	}

	int _line;
	SQDBGString _src;
};

struct Watch{
	Watch() { _id = 0; }
	Watch(int id,const SQChar *exp) { _id = id; _exp = exp; }
	Watch(const Watch &w) { _id = w._id; _exp = w._exp; }
	bool operator<(const Watch& w) const { return _id<w._id; }
	bool operator==(const Watch& w) const { return _id == w._id; }
	int _id;
	SQDBGString _exp;
};

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
	void Hook(int type,int line,const SQChar *src,const SQChar *func);
	void ParseMsg(const char *msg);
	bool ParseBreakpoint(const char *msg,BreakPoint &out);
	bool ParseWatch(const char *msg,Watch &out);
	bool ParseRemoveWatch(const char *msg,int &id);
	void Terminated();
	//
	void BreakExecution();
	void Send(const SQChar *s,...);
	void SendChunk(const SQChar *chunk);
	void Break(int line,const SQChar *src,const SQChar *type,const SQChar *error=NULL);


	void SerializeState();
	//COMMANDS
	void AddBreakpoint(BreakPoint &bp);
	void AddWatch(Watch &w);
	void RemoveWatch(int id);
	void RemoveBreakpoint(BreakPoint &bp);

	//
	void SetErrorHandlers();

	//XML RELATED STUFF///////////////////////
	#define MAX_NESTING 10
	struct XMLElementState {
		SQChar name[256];
		bool haschildren;
	};

	XMLElementState xmlstate[MAX_NESTING];
	int _xmlcurrentement;

	void BeginDocument() { _xmlcurrentement = -1; }
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
	int _recursionlevel;
	int _maxrecursion;
	int _nestedcalls;
	bool _ready;
	bool _autoupdate;
	HSQOBJECT _serializefunc;
	SQCharVec _scratchstring;

};

#endif //_SQ_DBGSERVER_H_
