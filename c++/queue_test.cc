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

class DataSet {
 public:
  DataSet(int size) {
    data_.resize(size);
    mark_.resize(size);
    for (int i = 0; i < size; i++) {
      data_[i] = i + 1;
    }
    std::shuffle(data_.begin(), data_.end(), std::random_device());
  }

  bool Produce(int& data) {
    int next = produced_.fetch_add(1);
    if (next < data_.size()) {
      data = data_[next];
      return true;
    }
    return false;
  }

  void Consume(int data) {
    mark_[data - 1] = data;
    consumed_.fetch_add(1);
  }

  bool Check() {
    if (consumed_ != mark_.size()) {
      return false;
    }
    for (int i = 0; i < mark_.size(); i++) {
      assert(mark_[i] == i + 1);
    }
    return true;
  }

 private:
  std::vector<int> data_;
  std::vector<int> mark_;
  std::atomic<int> produced_ {0};
  std::atomic<int> consumed_ {0};
};

void producer(Queue* q, DataSet* set) {
  int data;
  while (set->Produce(data)) {
    while (!q->Enqueue(data));
  }
}

void consumer(Queue* q, DataSet* set) {
  int data;
  while (!set->Check()) {
    if (q->Dequeue(data)) {
      set->Consume(data);
    }
  }
}

void test(Queue* q, int set_size, int num_producers, int num_consumers) {
  auto set = std::make_unique<DataSet>(set_size);
  std::vector<std::thread> threads;
  for (int i = 0; i < num_producers; i++) {
    threads.emplace_back(producer, q, set.get());
  }
  for (int i = 0; i < num_consumers; i++) {
    threads.emplace_back(consumer, q, set.get());
  }
  for (int i = 0; i < threads.size(); i++) {
    threads[i].join();
  }
}

int main(int argc, char* argv[]) {
  int size = 4096;
  {
    auto q = std::make_unique<bounded_mpmc::Queue>(size);
    test(q.get(), size, 10, 10);
  }
  {
    auto q = std::make_unique<non_intrusive_mpsc::Queue>();
    test(q.get(), size, 10, 1);
  }
  return 0;
}
