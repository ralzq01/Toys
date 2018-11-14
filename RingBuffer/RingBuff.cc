#include "RingBuff.hpp"

RingBuffer::RingBuffer(std::size_t buffer_size)
    : buffer_size_(buffer_size),
      ofs_reader_(0),
      ofs_writer_(0),
      buffer_(nullptr) {
  assert(buffer_size_ >= 2);
  buffer_ = static_cast<void*>(std::malloc(buffer_size_));
  if(!buffer_){
    throw std::bad_alloc();
  }
}

RingBuffer::~RingBuffer(){
  if(buffer_){
    std::free(buffer_);
  }
}

void RingBuffer::write(void* buffer, std::size_t size){
  if(size > buffer_size_){
    std::cerr << "Error: buffer size too large" << std::endl;
    return;
  }
  std::unique_lock<std::mutex> lock(mtx_);
  while(ofs_writer_ + size - ofs_reader_ > buffer_size_){
    // full
    std::size_t distance = ofs_writer_ - ofs_reader_;
    ofs_reader_ = ofs_reader_ % buffer_size_;
    ofs_writer_ = ofs_reader_ + distance;
    not_full_.wait(lock);
  }
  std::size_t real_ofs = ofs_writer_ % buffer_size_;
  if(buffer_size_ - real_ofs >= size){
    memcpy((void*)((char*)buffer_ + real_ofs), buffer, size);
  }
  else{
    std::size_t first_part = buffer_size_ - real_ofs;
    std::size_t second_part = size - first_part;
    memcpy((void*)((char*)buffer_ + real_ofs), buffer, first_part);
    memcpy(buffer_, (void*)((char*)buffer + first_part), second_part);
  }
  ofs_writer_ += size;
  items_.push(size);
  not_empty_.notify_all();
  lock.unlock();
}

void RingBuffer::read(void** buffer, std::size_t& size){
  std::unique_lock<std::mutex> lock(mtx_);
  while(items_.empty()){
    // empty
    not_empty_.wait(lock);
  }
  size = items_.front();
  std::size_t real_ofs = ofs_reader_ % buffer_size_;
  if(buffer_size_ - real_ofs >= size){
    *buffer = (void*)((char*)buffer_ + real_ofs);
  }
  else{
    void* temp = nullptr;
    temp = static_cast<void*>(std::malloc(size));
    std::size_t first_part = buffer_size_ - real_ofs;
    std::size_t second_part = size - first_part;
    if(temp){
      memcpy(temp, (void*)((char*)buffer_ + real_ofs), first_part);
      memcpy((void*)((char*)temp + first_part), buffer_, second_part);
      alloc_buffer_.push(temp);
      *buffer = temp;
    }
    else{
      *buffer = nullptr;
      throw std::bad_alloc();
    }
  }
  lock.unlock();
}

void RingBuffer::consume() {
  std::unique_lock<std::mutex> lock(mtx_);
  while(items_.empty()){
    not_empty_.wait(lock);
  }
  std::size_t size = items_.front();
  std::size_t real_ofs = ofs_reader_ % buffer_size_;
  if(real_ofs + size > buffer_size_){
    void* allocated = alloc_buffer_.front();
    std::free(allocated);
    alloc_buffer_.pop();
  }
  ofs_reader_ += size;
  items_.pop();
  not_full_.notify_all();
  lock.unlock();
}