#include "RingBuff.hpp"

RingBuffer::RingBuffer(std::size_t buffer_size)
    : buffer_size_(buffer_size),
      ofs_consumer_(0),
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
  while(ofs_writer_ + size - ofs_consumer_ > buffer_size_){
    // full
    std::size_t distance = ofs_writer_ - ofs_consumer_;
    ofs_consumer_ = ofs_consumer_ % buffer_size_;
    ofs_writer_ = ofs_consumer_ + distance;
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
  // update
  ofs_writer_ += size;
  wait_read_.push(size);
  not_empty_.notify_all();
  lock.unlock();
}

void RingBuffer::read(void** buffer, std::size_t& size){
  while(wait_read_.empty()){}
  size = wait_read_.front();
  ofs_reader_ = ofs_reader_ % buffer_size_;
  // if buffer is continous, return the buffer address
  if(buffer_size_ - ofs_reader_ >= size){
    *buffer = (void*)((char*)buffer_ + ofs_reader_);
  }
  // if buffer is not continous, allocate a new buffer and 
  // copy data into new buffer to make it continous.
  // I think there exists a more elegant way to do this
  else{
    void* temp = nullptr;
    temp = static_cast<void*>(std::malloc(size));
    std::size_t first_part = buffer_size_ - ofs_reader_;
    std::size_t second_part = size - first_part;
    if(temp){
      memcpy(temp, (void*)((char*)buffer_ + ofs_reader_), first_part);
      memcpy((void*)((char*)temp + first_part), buffer_, second_part);
      alloc_buffer_.push(temp);
      *buffer = temp;
    }
    else{
      *buffer = nullptr;
      throw std::bad_alloc();
    }
  }
  // update
  wait_read_.pop();
  wait_consume_.push(size);
  ofs_reader_ += size;
}

void RingBuffer::consume() {
  std::unique_lock<std::mutex> lock(mtx_);
  // consume can only be called after `read`
  // `consume` and `read` shoule be called same times.
  if(wait_consume_.empty()){
    std::cerr << "Error: consume call and read call number should match" << std::endl;
    return;
  }
  std::size_t size = wait_consume_.front();
  std::size_t real_ofs = ofs_consumer_ % buffer_size_;
  if(real_ofs + size > buffer_size_){
    void* allocated = alloc_buffer_.front();
    std::free(allocated);
    alloc_buffer_.pop();
  }
  ofs_consumer_ += size;
  wait_consume_.pop();
  not_full_.notify_all();
  lock.unlock();
}