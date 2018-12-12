
#include <porter.hpp>

void Porter::write(const void* buffer, std::size_t size){
  
  std::unique_lock<std::mutex> lock(size_mtx_);
  while(current_size_ + size > max_size_){
    not_full_.wait(lock);
  }
  current_size_ += size;
  lock.unlock();

  void* buff = static_cast<void*>(std::malloc(size));
  if(buff == nullptr){
    std::cerr << "Error: Memory allocation failed\n";
    return;
  }
  memcpy(buff, buffer, size);

  logs_.push(Item(buff, size));
}

void Porter::read(void** buffer, std::size_t& size){

  Item item(nullptr, 0);
  logs_.fpop(item);
  // get buffer ptr
  *buffer = item.buffer;
  last_read_ = *buffer;
  // get size
  size = item.size;
  last_size_ = size;
  
  wait_consume_.push(item);
}

void Porter::lastRead(void** buffer, std::size_t& size){
  if(!last_read_){
    std::cerr << "Error: Last Item has been consumed.\n";
  }
  *buffer = last_read_;
  size = last_size_;
}

void Porter::consume(){
  Item item = wait_consume_.front();
  wait_consume_.pop();
  if(item.buffer == last_read_){
    last_read_ = nullptr;
    last_size_ = 0;
  }
  // free buffer
  std::free(item.buffer);
  // update current size
  std::lock_guard<std::mutex> lock(size_mtx_);
  current_size_ -= item.size;
  not_full_.notify_one();
}

bool Porter::resize(std::size_t size){
  std::lock_guard<std::mutex> lock(size_mtx_);
  if(size < current_size_){
    std::cerr << "Error: Can't resize ringbuffer.\n";
    return false;
  }
  max_size_ = size;
  not_full_.notify_one();
  return true;
}

Porter::~Porter(){
  Item item(nullptr, 0);
  while(!logs_.empty()){
    logs_.fpop(item);
    std::free(item.buffer);
  }
  while(!wait_consume_.empty()){
    consume();
  }
}

