#include <algorithm>
#include <atomic>
#include <random>
#include <thread>
#include <vector>
#include <assert.h>

#include "list.h"
#include "skip_list.h"
#include "linked_list.h"

using interface::List;

class TestSet {
 public:
  TestSet(int size) {
    data_.resize(size);
    for (int i = 0; i < size; i++) {
      data_[i] = i + 1;
    }
    std::shuffle(data_.begin(), data_.end(), std::random_device());
  }

  bool Peek(int& data) {
    int next = next_++;
    if (next < data_.size()) {
      data = data_[next];
      return true;
    }
    return false;
  }

  bool Done() {
    return next_ >= data_.size();
  }

  void Check(List* l) {
    std::unique_ptr<List::Iterator> it(l->NewIterator());
    int data, next = 0;
    for (it->SeekToFirst(); it->Peek(data); it->Next()) {
      next++;
      assert(data == next);
    }
    assert(next == data_.size());
  }

 private:
  std::vector<int> data_;
  std::atomic<int> next_ {0};
};

void writer(List* l, TestSet* t) {
  int data;
  while (t->Peek(data)) {
    assert(l->Insert(data));
    assert(l->Contains(data));
  }
}

void reader(List* l, TestSet* t) {
  std::unique_ptr<List::Iterator> it;
  while (!t->Done()) {
    it.reset(l->NewIterator());
    int data, prev = 0;
    for (it->SeekToFirst(); it->Peek(data); it->Next()) {
      assert(prev < data);
      prev = data;
    }
  }
}

void run_test(List* l, int test_size, int num_writers, int num_readers) {
  auto t = std::make_unique<TestSet>(test_size);
  std::vector<std::thread> threads;
  for (int i = 0; i < num_writers; i++) {
    threads.emplace_back(writer, l, t.get());
  }
  for (int i = 0; i < num_readers; i++) {
    threads.emplace_back(reader, l, t.get());
  }
  for (int i = 0; i < threads.size(); i++) {
    threads[i].join();
  }
  t->Check(l);
}

int main(int argc, char* argv[]) {
  int size = 4096;
  {
    auto l = std::make_unique<skip_list::List>(12);
    run_test(l.get(), size, 1, 10);
  }
  {
    auto l = std::make_unique<linked_list::List>();
    run_test(l.get(), size, 10, 10);
  }
  return 0;
}
