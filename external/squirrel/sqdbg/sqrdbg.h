#ifndef _SQ_RDBG_H_
#define _SQ_RDBG_H_

#ifdef __cplusplus
extern "C" {
#endif
#ifdef _WIN32
#pragma comment(lib, "WSOCK32.LIB")
#endif

struct SQDbgServer;
typedef SQDbgServer* HSQREMOTEDBG;

HSQREMOTEDBG sq_rdbg_init(HSQUIRRELVM v,unsigned short port,SQBool autoupdate);
SQRESULT sq_rdbg_waitforconnections(HSQREMOTEDBG rdbg);
SQRESULT sq_rdbg_shutdown(HSQREMOTEDBG rdbg);
SQRESULT sq_rdbg_update(HSQREMOTEDBG rdbg);
 
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //_SQ_RDBG_H_
