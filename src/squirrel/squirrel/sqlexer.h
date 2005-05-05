/*	see copyright notice in squirrel.h */
#ifndef _SQLEXER_H_
#define _SQLEXER_H_

#define MAX_STRING 2024

struct SQLexer
{
	SQLexer();
	~SQLexer();
	void Init(SQSharedState *ss,SQLEXREADFUNC rg,SQUserPointer up);
	int Lex();
	SQObjectPtr Tok2Str(int tok);
private:
	int GetIDType(SQChar *s);
	int ReadString(int ndelim,bool verbatim);
	int ReadNumber();
	void LexBlockComment();
	int ReadID();
	void Next();
	int _curtoken;
	SQTable *_keywords;
public:
	int _prevtoken;
	int _currentline;
	int _lasttokenline;
	int _currentcolumn;
	const SQChar *_svalue;
	SQInteger _nvalue;
	SQFloat _fvalue;
	SQLEXREADFUNC _readf;
	SQUserPointer _up;
#ifdef _UNICODE
	SQChar _currdata;
#else
	unsigned char _currdata;
#endif
	SQSharedState *_sharedstate;
	sqvector<SQChar> _longstr;
};

#endif
