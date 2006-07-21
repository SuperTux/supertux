/*
Copyright (c) 2003-2005 Alberto Demichelis

This software is provided 'as-is', without any
express or implied warranty. In no event will the
authors be held liable for any damages arising from
the use of this software.

Permission is granted to anyone to use this software
for any purpose, including commercial applications,
and to alter it and redistribute it freely, subject
to the following restrictions:

		1. The origin of this software must not be
		misrepresented; you must not claim that
		you wrote the original software. If you
		use this software in a product, an
		acknowledgment in the product
		documentation would be appreciated but is
		not required.

		2. Altered source versions must be plainly
		marked as such, and must not be
		misrepresented as being the original
		software.

		3. This notice may not be removed or
		altered from any source distribution.

*/
#ifndef _SQ_RDBG_H_
#define _SQ_RDBG_H_

#ifdef __cplusplus
extern "C" {
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
