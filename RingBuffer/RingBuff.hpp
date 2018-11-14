#ifndef _RINGBUFF_H_
#define _RINGBUFF_H_

#include <iostream>
#include <cstdlib>
#include <memory>
#include <thread>
#include <stdexcept>
#include <condition_variable>
#include <queue>
#include <cstring>
#include <assert.h>

class RingBuffer{

 public:

  RingBuffer(const RingBuffer&) = delete;
  RingBuffer& operator=(const RingBuffer&) = delete;

  explicit RingBuffer(std::size_t buffer_size);

  /* \brief push a buffer into RingBuffer
   * Thread safe
   */
  void write(void* buffer, std::size_t size);

  /* \brief pop a bufer from RingBuffer
   * Thread safe
   */
  void read(void** buffer, std::size_t& size);

  void consume();

  ~RingBuffer();

 protected:

  std::size_t buffer_size_;

  std::mutex mtx_;
  std::condition_variable not_full_;
  std::condition_variable not_empty_;

  std::size_t ofs_reader_;
  std::size_t ofs_writer_;

  std::queue<std::size_t> items_;
  std::queue<void*> alloc_buffer_;

  void* buffer_;

};



#endif