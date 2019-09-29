// C++ implementation of a bounded MPMC queue.
// http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue

#ifndef LOCKFREE_BOUNDED_MPMC
#define LOCKFREE_BOUNDED_MPMC

#include <atomic>
#include <assert.h>

#include "queue.h"

namespace bounded_mpmc {

class Queue : public interface::Queue {
 public:
  Queue(int64_t size) {
    assert((size >= 2) && ((size & (size - 1)) == 0));
    buffer_mark_ = size - 1;
    buffer_ = new Node[size];
    for (int64_t i = 0; i < size; i++) {
      buffer_[i].sequence = i;
    }
  }

  ~Queue() { delete[] buffer_; }

  bool Enqueue(int data) override {
    do {
      auto pos = enqueue_pos_.load(std::memory_order_relaxed);
      auto node = &buffer_[pos & buffer_mark_];
      auto seq = node->sequence.load(std::memory_order_acquire);
      auto diff = seq - pos;
      if (diff == 0) {
        if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
          node->data = data;
          node->sequence.store(pos + 1, std::memory_order_release);
          return true;
        }
      } else if (diff < 0) {
        return false;
      }
    } while (true);
  }

  bool Dequeue(int& data) override {
    do {
      auto pos = dequeue_pos_.load(std::memory_order_relaxed);
      auto node = &buffer_[pos & buffer_mark_];
      auto seq = node->sequence.load(std::memory_order_acquire);
      auto diff = seq - (pos + 1);
      if (diff == 0) {
        if (dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
          data = node->data;
          node->sequence.store(pos + buffer_mark_ + 1, std::memory_order_release);
          return true;
        }
      } else if (diff < 0) {
        return false;
      }
    } while (true);
  }

 private:
  struct Node {
    int data;
    std::atomic<int64_t> sequence {0};
  };

  Node* buffer_;
  int64_t buffer_mark_;
  std::atomic<int64_t> enqueue_pos_ {0};
  std::atomic<int64_t> dequeue_pos_ {0};
};

}

#endif  // LOCKFREE_BOUNDED_MPMC
