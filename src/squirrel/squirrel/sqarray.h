/*	see copyright notice in squirrel.h */
#ifndef _SQARRAY_H_
#define _SQARRAY_H_

struct SQArray : public CHAINABLE_OBJ
{
private:
	SQArray(SQSharedState *ss,int nsize){_values.resize(nsize);_uiRef=0;INIT_CHAIN();ADD_TO_CHAIN(&_ss(this)->_gc_chain,this);}
	~SQArray()
	{
		REMOVE_FROM_CHAIN(&_ss(this)->_gc_chain,this);
	}
public:
	static SQArray* Create(SQSharedState *ss,int nInitialSize){
		SQArray *newarray=(SQArray*)SQ_MALLOC(sizeof(SQArray));
		new (newarray) SQArray(ss,nInitialSize);
		return newarray;
	}
#ifndef NO_GARBAGE_COLLECTOR
	void Mark(SQCollectable **chain);
#endif
	void Finalize(){
		_values.resize(0);
	}
	bool Get(const int nidx,SQObjectPtr &val)
	{
		if(nidx>=0 && nidx<(int)_values.size()){
			val=_values[nidx];
			return true;
		}
		else return false;
	}
	bool Set(const int nidx,const SQObjectPtr &val)
	{
		if(nidx>=0 && nidx<(int)_values.size()){
			_values[nidx]=val;
			return true;
		}
		else return false;
	}
	int Next(const SQObjectPtr &refpos,SQObjectPtr &outkey,SQObjectPtr &outval)
	{
		unsigned int idx=TranslateIndex(refpos);
		while(idx<_values.size()){
			//first found
			outkey=(SQInteger)idx;
			outval=_values[idx];
			//return idx for the next iteration
			return ++idx;
		}
		//nothing to iterate anymore
		return -1;
	}
	SQArray *Clone(){SQArray *anew=Create(_opt_ss(this),Size()); anew->_values.copy(_values); return anew; }
	int Size() const {return _values.size();}
	void Resize(int size,SQObjectPtr &fill = _null_) { _values.resize(size,fill); ShrinkIfNeeded(); }
	void Reserve(int size) { _values.reserve(size); }
	void Append(const SQObject &o){_values.push_back(o);}
	void Extend(const SQArray *a);
	SQObjectPtr &Top(){return _values.top();}
	void Pop(){_values.pop_back(); ShrinkIfNeeded(); }
	void Insert(const SQObject& idx,const SQObject &val){_values.insert((unsigned int)tointeger(idx),val);}
	void ShrinkIfNeeded() {
		if(_values.size() <= _values.capacity()>>2) //shrink the array
			_values.shrinktofit();
	}
	void Remove(unsigned int idx){
		_values.remove(idx);
		ShrinkIfNeeded();
	}
	void Release()
	{
		sq_delete(this,SQArray);
	}
	SQObjectPtrVec _values;
};
#endif //_SQARRAY_H_
