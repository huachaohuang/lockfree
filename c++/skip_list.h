#ifndef LOCKFREE_SKIP_LIST
#define LOCKFREE_SKIP_LIST

#include <atomic>
#include <vector>
#include <stdlib.h>

#include "list.h"

namespace skip_list {

class List : public interface::List {
 public:
  List(int max_height) : head_(new Node(0, max_height)), max_height_(max_height) {}

  ~List() {
    auto node = head_;
    while (node) {
      auto next = node->next[0].load(std::memory_order_relaxed);
      delete node;
      node = next;
    }
  }

  bool Insert(int data) override {
    Node* prev[max_height_];
    Node* next = FindGreaterOrEqual(data, prev);
    if (next && next->data == data) {
      return false;
    }

    auto new_height = RandomHeight();
    auto old_height = height_.load(std::memory_order_relaxed);
    if (new_height > old_height) {
      for (int i = old_height; i < new_height; i++) {
        prev[i] = head_;
      }
      height_.store(new_height, std::memory_order_relaxed);
    }

    auto node = new Node(data, new_height);
    for (int i = 0; i < new_height; i++) {
      next = prev[i]->next[i].load(std::memory_order_relaxed);
      node->next[i].store(next, std::memory_order_relaxed);
      prev[i]->next[i].store(node, std::memory_order_release);
    }
    return true;
  }

  bool Contains(int data) override {
    auto node = FindGreaterOrEqual(data, nullptr);
    return node && node->data == data;
  }

  interface::List::Iterator* NewIterator() override {
    return new Iterator(head_);
  }

 private:
  struct Node {
    int data;
    std::vector<std::atomic<Node*>> next;
    Node(int v, int height) : data(v), next(height) {}
  };

  class Iterator : public interface::List::Iterator {
   public:
    Iterator(Node* head) : head_(head), node_(nullptr) {}

    void SeekToFirst() {
      node_ = head_->next[0].load(std::memory_order_acquire);
    }

    void Next() {
      node_ = node_->next[0].load(std::memory_order_acquire);
    }

    bool Peek(int& data) {
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

  int RandomHeight() {
    int height = 1;
    while (height < max_height_) {
      if (rand() % 2 == 0) {
        height++;
      } else {
        break;
      }
    }
    return height;
  }

  Node* FindGreaterOrEqual(int data, Node** prev) {
    auto node = head_;
    auto height = height_.load(std::memory_order_relaxed) - 1;
    while (true) {
      auto next = node->next[height].load(std::memory_order_acquire);
      if (next && next->data < data) {
        node = next;
      } else {
        if (prev) {
          prev[height] = node;
        }
        if (height > 0) {
          height--;
        } else {
          return next;
        }
      }
    }
  }

  Node* head_;
  const int max_height_;
  std::atomic<int> height_ {1};
};

}

#endif  // LOCKFREE_SKIP_LIST
