
#ifndef _PORTER_H_
#define _PORTER_H_

#include <memory>
#include <cstring>
#include <iostream>
#include <safequeue.hpp>

typedef struct Item{
  void* buffer;
  std::size_t size;
  Item(void* buffer_, std::size_t size_): buffer(buffer_), size(size_) {}
}Item;

class Porter{

 public:

  Porter(const Porter&) = delete;
  Porter& operator=(const Porter&) = delete;

  Porter(): max_size_(0)
          , current_size_(0)
          , last_read_(nullptr)
          , last_size_(0) {}

  void write(const void* buffer, std::size_t size);

  void read(void** buffer, std::size_t& size);

  void lastRead(void** buffer);

  void consume();

  void resize(std::size_t size);

 protected:
  
  std::size_t max_size_;
  std::size_t current_size_;
  void* last_read_;
  std::size_t last_size_;

  std::mutex size_mtx_;
  std::condition_variable not_full_;

  SafeQueue<Item> logs_;
};

#endif