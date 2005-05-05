/*
see copyright notice in squirrel.h
*/
#include "sqpcheader.h"
#include "sqvm.h"
#include "sqtable.h"
#include "sqfuncproto.h"
#include "sqclosure.h"

SQTable::SQTable(SQSharedState *ss,int nInitialSize)
{
	int pow2size=MINPOWER2;
	while(nInitialSize>pow2size)pow2size=pow2size<<1;
	AllocNodes(pow2size);
	_uiRef = 0;
	_usednodes = 0;
	_delegate = NULL;
	INIT_CHAIN();
	ADD_TO_CHAIN(&_sharedstate->_gc_chain,this);
}

void SQTable::Remove(const SQObjectPtr &key)
{
	_HashNode *n = _Get(key, HashKey(key) & (_numofnodes - 1));
	if (n) {
		n->val = n->key = _null_;
		_usednodes--;
		Rehash(false);
	}
}

void SQTable::AllocNodes(int nSize)
{
	_HashNode *nodes=(_HashNode *)SQ_MALLOC(sizeof(_HashNode)*nSize);
	for(int i=0;i<nSize;i++){
		new (&nodes[i]) _HashNode;
		nodes[i].next=NULL;
	}
	_numofnodes=nSize;
	_nodes=nodes;
	_firstfree=&_nodes[_numofnodes-1];
}

int SQTable::CountUsed()
{
	/*int n=0;
	for(int i=0;i<_numofnodes;i++){
		if(type(_nodes[i].key)!=OT_NULL) n++;
	}*/
	return _usednodes;
}

void SQTable::Rehash(bool force)
{
	int oldsize=_numofnodes;
	//prevent problems with the integer division
	if(oldsize<4)oldsize=4;
	_HashNode *nold=_nodes;
	int nelems=CountUsed();
	if (nelems >= oldsize-oldsize/4)  /* using more than 3/4? */
		AllocNodes(oldsize*2);
	else if (nelems <= oldsize/4 &&  /* less than 1/4? */
		oldsize > MINPOWER2)
		AllocNodes(oldsize/2);
	else if(force)
		AllocNodes(oldsize);
	else
		return;
	_usednodes = 0;
	for (int i=0; i<oldsize; i++) {
		_HashNode *old = nold+i;
		if (type(old->key) != OT_NULL)
			NewSlot(old->key,old->val);
	}
	for(int k=0;k<oldsize;k++) 
		nold[k].~_HashNode();
	SQ_FREE(nold,oldsize*sizeof(_HashNode));
}

SQTable *SQTable::Clone()
{
	SQTable *nt=Create(_opt_ss(this),_numofnodes);
	SQInteger ridx=0;
	SQObjectPtr key,val;
	while((ridx=Next(ridx,key,val))!=-1){
		nt->NewSlot(key,val);
	}
	nt->SetDelegate(_delegate);
	return nt;
}

bool SQTable::Get(const SQObjectPtr &key,SQObjectPtr &val)
{
	_HashNode *n = _Get(key, HashKey(key) & (_numofnodes - 1));
	if (n) {
		val = n->val;
		return true;
	}
	return false;
}
bool SQTable::NewSlot(const SQObjectPtr &key,const SQObjectPtr &val)
{
	unsigned long h = HashKey(key) & (_numofnodes - 1);
	_HashNode *n = _Get(key, h);
	if (n) {
		n->val = val;
		return false;
	}
	_HashNode *mp = &_nodes[h];
	n = mp;

	//key not found I'll insert it
	//main pos is not free

	if(type(mp->key)!=OT_NULL) {

		_HashNode *othern;  /* main position of colliding node */
		n = _firstfree;  /* get a free place */
		if (mp > n && (othern = &_nodes[h]) != mp){
			/* yes; move colliding node into free position */
			while (othern->next != mp)
				othern = othern->next;  /* find previous */
			othern->next = n;  /* redo the chain with `n' in place of `mp' */
			*n = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
			mp->next = NULL;  /* now `mp' is free */
		}
		else{
			/* new node will go into free position */
			n->next = mp->next;  /* chain new position */
			mp->next = n;
			mp = n;
		}
	}
	mp->key = key;

	for (;;) {  /* correct `firstfree' */
		if (type(_firstfree->key) == OT_NULL) {
			mp->val = val;
			_usednodes++;
			return true;  /* OK; table still has a free place */
		}
		else if (_firstfree == _nodes) break;  /* cannot decrement from here */
		else (_firstfree)--;
	}
	Rehash(true);
	return NewSlot(key, val);
}

int SQTable::Next(const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval)
{
	int idx = (int)TranslateIndex(refpos);
	while (idx < _numofnodes) {
		if(type(_nodes[idx].key) != OT_NULL) {
			//first found
			outkey = _nodes[idx].key;
			outval = _nodes[idx].val;
			//return idx for the next iteration
			return ++idx;
		}
		++idx;
	}
	//nothing to iterate anymore
	return -1;
}

bool SQTable::SetDelegate(SQTable *mt)
{
	SQTable *temp = mt;
	while (temp) {
		if (temp->_delegate == this) return false; //cycle detected
		temp = temp->_delegate;
	}
	if (mt)	__ObjAddRef(mt);
	__ObjRelease(_delegate);
	_delegate = mt;
	return true;
}

bool SQTable::Set(const SQObjectPtr &key, const SQObjectPtr &val)
{
	_HashNode *n = _Get(key, HashKey(key) & (_numofnodes - 1));
	if (n) {
		n->val = val;
		return true;
	}
	return false;
}

void SQTable::Finalize()
{
	for(int i = 0;i < _numofnodes; i++) { _nodes[i].key = _null_; _nodes[i].val = _null_; }
		SetDelegate(NULL);
}
