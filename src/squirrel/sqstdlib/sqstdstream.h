/*	see copyright notice in squirrel.h */
#ifndef _SQSTD_STREAM_H_
#define _SQSTD_STREAM_H_

int _stream_readstr(HSQUIRRELVM v);
int _stream_readblob(HSQUIRRELVM v);
int _stream_readline(HSQUIRRELVM v);
int _stream_readn(HSQUIRRELVM v);
int _stream_writestr(HSQUIRRELVM v);
int _stream_writeblob(HSQUIRRELVM v);
int _stream_writen(HSQUIRRELVM v);
int _stream_seek(HSQUIRRELVM v);
int _stream_tell(HSQUIRRELVM v);
int _stream_len(HSQUIRRELVM v);
int _stream_eos(HSQUIRRELVM v);
int _stream_flush(HSQUIRRELVM v);

#define _DECL_STREAM_FUNC(name,nparams,typecheck) {_SC(#name),_stream_##name,nparams,typecheck}
SQRESULT declare_stream(HSQUIRRELVM v,SQChar* name,int typetag,SQChar* reg_name,SQRegFunction *methods,SQRegFunction *globals);
#endif /*_SQSTD_STREAM_H_*/
