#ifndef LOCKFREE_LINKED_LIST
#define LOCKFREE_LINKED_LIST

#include <atomic>

#include "list.h"

namespace linked_list {

class List : public interface::List {
 public:
  List() : head_(new Node(0)) {}

  ~List() {
    auto node = head_;
    while (node) {
      auto next = node->next.load(std::memory_order_relaxed);
      delete node;
      node = next;
    }
  }

  bool Insert(int data) override {
    auto node = new Node(data);
    while (true) {
      Node* prev;
      Node* next = Search(data, prev);
      if (next && next->data == data) {
        delete node;
        return false;
      }
      node->next.store(next, std::memory_order_relaxed);
      if (prev->next.compare_exchange_weak(next, node, std::memory_order_acq_rel)) {
        return true;
      }
    }
  }

  bool Contains(int data) override {
    Node* prev;
    Node* node = Search(data, prev);
    return node && node->data == data;
  }

  interface::List::Iterator* NewIterator() override {
    return new Iterator(head_);
  }

 private:
  struct Node {
    int data;
    std::atomic<Node*> next;
    Node(int v) : data(v), next(nullptr) {}
  };

  class Iterator : public interface::List::Iterator {
   public:
    Iterator(Node* head) : head_(head), node_(nullptr) {}

    void SeekToFirst() override {
      node_ = head_->next.load(std::memory_order_acquire);
    }

    void Next() override {
      node_ = node_->next.load(std::memory_order_acquire);
    }

    bool Peek(int& data) override {
      if (node_) {
        data = node_->data;
        return true;
      }
      return false;
    }

   private:
    Node* head_;
    Node* node_;
  };

  Node* Search(int data, Node*& prev) {
    auto node = head_;
    do {
      prev = node;
      node = node->next.load(std::memory_order_acquire);
    } while (node && node->data < data);
    return node;
  }

  Node* head_ {nullptr};
};

}

#endif  // LOCKFREE_LINKED_LIST
