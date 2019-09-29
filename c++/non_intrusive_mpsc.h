// C++ implementation of a non-intrusive MPSC queue.
// http://www.1024cores.net/home/lock-free-algorithms/queues/non-intrusive-mpsc-node-based-queue

#ifndef LOCKFREE_NON_INTRUSIVE_MPSC
#define LOCKFREE_NON_INTRUSIVE_MPSC

#include <atomic>

#include "queue.h"

namespace non_intrusive_mpsc {

class Queue : public interface::Queue {
 public:
  Queue() : stub_(new Node(0)), head_(stub_), tail_(stub_) {}

  ~Queue() {
    auto head = head_.load(std::memory_order_relaxed);
    while (head) {
      auto node = head;
      head = head->next.load(std::memory_order_relaxed);
      delete node;
    }
  }

  bool Enqueue(int data) override {
    auto node = new Node(data);
    auto prev = tail_.exchange(node, std::memory_order_acq_rel);
    prev->next.store(node, std::memory_order_release);
    return true;
  }

  bool Dequeue(int& data) override {
    auto head = head_.load(std::memory_order_relaxed);
    auto next = head->next.load(std::memory_order_acquire);
    if (next) {
      head_.store(next, std::memory_order_relaxed);
      data = next->data;
      delete head;
      return true;
    }
    return false;
  }

 private:
  struct Node {
    int data;
    std::atomic<Node*> next;
    Node(int v) : data(v), next(nullptr) {}
  };

  Node* stub_;
  std::atomic<Node*> head_;
  std::atomic<Node*> tail_;
};

}

#endif  // LOCKFREE_NON_INTRUSIVE_MPSC
