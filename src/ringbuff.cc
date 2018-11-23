#include <ringbuff.hpp>

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

void RingBuffer::write(const void* buffer, std::size_t size){
  if(size > buffer_size_){
    std::cerr << "Error: buffer size too large" << std::endl;
    return;
  }

  std::unique_lock<std::mutex> lock(mtx_);
  while(ofs_writer_ + size - ofs_consumer_ > buffer_size_){
    // full
    not_full_.wait(lock);
  }
  lock.unlock();

  std::size_t real_ofs = ofs_writer_ % buffer_size_;
  std::size_t remain = buffer_size_ - real_ofs;
  if(remain <= size){
    ofs_writer_ += remain;
    real_ofs = 0;
    wait_read_.push(remain);
  }
  ofs_writer_ += size;
  memcpy((void*)((char*)buffer_ + real_ofs), buffer, size);
  wait_read_.push(size);
}

void RingBuffer::read(void** buffer, std::size_t& size){
  
  wait_read_.fpop(size);
  ofs_reader_ = ofs_reader_ % buffer_size_;

  if(ofs_reader_ + size == buffer_size_){
    // detect empty buffer
    ofs_reader_ = 0;

    std::unique_lock<std::mutex> lock(mtx_);
    ofs_consumer_ += size;
    lock.unlock();

    not_full_.notify_one();
    wait_read_.fpop(size);
  }

  // if buffer is continous, return the buffer address
  *buffer = (void*)((char*)buffer_ + ofs_reader_);
  
  // update
  wait_consume_.push(size);
  ofs_reader_ += size;
}

void RingBuffer::consume() {
  // consume can only be called after `read`
  // `consume` and `read` shoule be called same times.
  if(wait_consume_.empty()){
    std::cerr << "Error: consume call and read call number should match" << std::endl;
    return;
  }
  std::size_t size = wait_consume_.front();

  std::unique_lock<std::mutex> lock(mtx_);
  ofs_consumer_ += size;
  not_full_.notify_one();
  lock.unlock();

  wait_consume_.pop();
}