
#ifndef _SFAEQUEUE_H_
#define _SFAEQUEUE_H_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>

template <typename T>
class SafeQueue {

 public:

  SafeQueue()
    : q_()
    , qmtx_()
    , empty_()
  {}

  ~SafeQueue() {}

  void push(T&& item) {
    std::lock_guard<std::mutex> lock(qmtx_);
    q_.push(item);
    empty_.notify_one();
  }

  void push(T& item) {
    std::lock_guard<std::mutex> lock(qmtx_);
    q_.push(item);
    empty_.notify_one();
  }

  void front(T& res) {
    std::lock_guard<std::mutex> lock(qmtx_);
    res = std::move(q_.front());
  }

  void pop() {
    std::unique_lock<std::mutex> lock(qmtx_);
    while(q_.empty()){
      empty_.wait(lock);
    }
    q_.pop();
    lock.unlock();
  }

  bool try_pop(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(qmtx_);
    if(empty_.wait_for(lock, timeout, [this] {return !q_.empty(); })){
      return false;
    }
    q_.pop();
    return true;
  }

  void fpop(T& res){
    std::unique_lock<std::mutex> lock(qmtx_);
    while(q_.empty()){
      empty_.wait(lock);
    }
    res = std::move(q_.front());
    q_.pop();
    lock.unlock();
  }

  bool try_fpop(T& res, std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(qmtx_);
    if(empty_.wait_for(lock, timeout, [this] {return !q_.empty(); })){
      return false;
    }
    res = std::move(q_.front());
    q_.pop();
    return true;
  }

  bool empty() {
    std::lock_guard<std::mutex> lock(qmtx_);
    return q_.empty();
  }

 private:
  std::queue<T> q_;
  mutable std::mutex qmtx_;
  std::condition_variable empty_;
};

#endif