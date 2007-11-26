// btree.h

#pragma once

#include "../stdafx.h"
#include "jsobj.h"
#include "storage.h"
#include "pdfile.h"

#pragma pack(push)
#pragma pack(1)

struct _KeyNode {
	DiskLoc prevChildBucket;
	DiskLoc recordLoc;
	short keyDataOfs;
};

#pragma pack(pop)

class BucketBasics;

/* wrapper - this is our in memory representation of the key.  _KeyNode is the disk representation. */
class KeyNode {
public:
	KeyNode(BucketBasics& bb, _KeyNode &k);
	DiskLoc& prevChildBucket;
	DiskLoc& recordLoc;
	JSObj key;
};

#pragma pack(push)
#pragma pack(1)

/* this class is all about the storage management */
class BucketBasics {
	friend class KeyNode;
public:
	bool isHead() { return parent.isNull(); }
	void assertValid();
protected:
	DiskLoc& getChild(int pos) { 
		assert( pos >= 0 && pos <= n );
		return pos == n ? nextChild : k(pos).prevChildBucket;
	}
	KeyNode keyNode(int i) { 
		assert( i < n );
		return KeyNode(*this, k(i));
	}

	char * dataAt(short ofs) { return data + ofs; }

	void init(); // initialize a new node

	/* returns false if node is full and must be split 
	   keypos is where to insert -- inserted after that key #.  so keypos=0 is the leftmost one.
	*/
	bool basicInsert(int keypos, const DiskLoc& recordLoc, JSObj& key);
	void pushBack(const DiskLoc& recordLoc, JSObj& key, DiskLoc prevChild);
	void del(int keypos);

	/* !Packed means there is deleted fragment space within the bucket.
       We "repack" when we run out of space before considering the node
	   to be full. 
	   */
	enum Flags { Packed=1 };

	DiskLoc childForPos(int p) { 
		return p == n ? nextChild : k(p).prevChildBucket;
	}

	int totalDataSize() const;
	void pack(); void setNotPacked(); void setPacked();
	int _alloc(int bytes);
	void truncateTo(int N);

	DiskLoc parent;
	DiskLoc nextChild; // the next bucket
	int Size; // total size of this btree node in bytes. constant.
	int flags;
	int emptySize; // size of the empty region
	int topSize; // size of the data at the top of the bucket (keys are at the beginning or 'bottom')
	int n; // # of keys so far.
	int reserved;
	_KeyNode& k(int i) { return ((_KeyNode*)data)[i]; }
	char data[4];
};

class BtreeBucket : public BucketBasics { 
	friend class BtreeCursor;
public:
	void dump();
	/* rc: 0 = ok */
	static DiskLoc addHead(const char *ns); /* start a new index off, empty */
	int insert(const DiskLoc& thisLoc, const char *ns, const DiskLoc& recordLoc, 
		JSObj& key, bool dupsAllowed, IndexDetails& idx);
	void update(const DiskLoc& recordLoc, JSObj& key);
	bool unindex(JSObj& key);
	DiskLoc locate(const DiskLoc& thisLoc, JSObj& key, int& pos, bool& found, int direction=1);
	/* advance one key position in the index: */
	DiskLoc advance(const DiskLoc& thisLoc, int& keyOfs, int direction);
	DiskLoc getHead(const DiskLoc& thisLoc);
private:
	JSObj keyAt(int keyOfs) { return keyOfs >= n ? JSObj() : keyNode(keyOfs).key; }
	static BtreeBucket* allocTemp(); /* caller must release with free() */
	void insertHere(const DiskLoc& thisLoc, const char *ns, int keypos, 
		const DiskLoc& recordLoc, JSObj& key,
		DiskLoc lchild, DiskLoc rchild, IndexDetails&);
	int _insert(const DiskLoc& thisLoc, const char *ns, const DiskLoc& recordLoc, 
		JSObj& key, bool dupsAllowed,
		DiskLoc lChild, DiskLoc rChild, IndexDetails&);
	bool find(JSObj& key, int& pos);
};

class BtreeCursor : public Cursor {
public:
	BtreeCursor(DiskLoc head, JSObj startKey, int direction, bool stopmiss);
	virtual bool ok() { return !bucket.isNull(); }
	bool eof() { return !ok(); }
	virtual Record* _current() { return currLoc().rec(); }
	virtual JSObj current() { return JSObj(_current()); }
	virtual DiskLoc currLoc();
	virtual bool advance();
	virtual bool tempStopOnMiss() { return stopmiss; }
	virtual void noteLocation(); // updates keyAtKeyOfs...
	virtual void checkLocation();
private:
	DiskLoc bucket;
	int keyOfs;
	int direction; // 1=fwd,-1=reverse
	bool stopmiss;
	JSObj keyAtKeyOfs; // so we can tell if things moved around on us between the query and the getMore call
};

#pragma pack(pop)