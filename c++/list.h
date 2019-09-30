#ifndef LOCKFREE_LIST
#define LOCKFREE_LIST

namespace interface {

class List {
 public:
  virtual ~List() {}

  virtual bool Insert(int data) = 0;

  virtual bool Contains(int data) = 0;

  class Iterator {
   public:
    virtual ~Iterator() {}
    virtual void SeekToFirst() = 0;
    virtual void Next() = 0;
    virtual bool Peek(int& data) = 0;
  };

  virtual Iterator* NewIterator() = 0;
};

}

#endif  // LOCKFREE_LIST
