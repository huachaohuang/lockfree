#include <algorithm>
#include <atomic>
#include <random>
#include <thread>
#include <vector>
#include <assert.h>

#include "queue.h"
#include "bounded_mpmc.h"
#include "non_intrusive_mpsc.h"

using interface::Queue;

class TestSet {
 public:
  TestSet(int size) {
    data_.resize(size);
    mark_.resize(size);
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

  void Mark(int data) {
    mark_[data - 1] = data;
    marked_++;
  }

  bool Done() {
    return marked_ == mark_.size();
  }

  void Check() {
    for (int i = 0; i < mark_.size(); i++) {
      assert(mark_[i] == i + 1);
    }
  }

 private:
  std::vector<int> data_;
  std::atomic<int> next_ {0};
  std::vector<int> mark_;
  std::atomic<int> marked_ {0};
};

void producer(Queue* q, TestSet* t) {
  int data;
  while (t->Peek(data)) {
    while (!q->Enqueue(data));
  }
}

void consumer(Queue* q, TestSet* t) {
  int data;
  while (!t->Done()) {
    if (q->Dequeue(data)) {
      t->Mark(data);
    }
  }
}

void run_test(Queue* q, int test_size, int num_producers, int num_consumers) {
  auto t = std::make_unique<TestSet>(test_size);
  std::vector<std::thread> threads;
  for (int i = 0; i < num_producers; i++) {
    threads.emplace_back(producer, q, t.get());
  }
  for (int i = 0; i < num_consumers; i++) {
    threads.emplace_back(consumer, q, t.get());
  }
  for (int i = 0; i < threads.size(); i++) {
    threads[i].join();
  }
  t->Check();
}

int main(int argc, char* argv[]) {
  int size = 4096;
  {
    auto q = std::make_unique<bounded_mpmc::Queue>(size);
    run_test(q.get(), size, 10, 10);
  }
  {
    auto q = std::make_unique<non_intrusive_mpsc::Queue>();
    run_test(q.get(), size, 10, 1);
  }
  return 0;
}
