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


/*! brief RingBuffer for transfering data between two threads
 *  Only support 1 consumer and 1 producer
 *  3 operations:
 *    `write`: for producer putting data into buffer
 *    `read`: for consumer getting data from buffer (withou copy)
 *    `consume`: indicate previous data have been consumed and can
 *               be covered.
 *  Notice: number of `read` call and number of `consume` call
 *          should be equal. You can first call multiple `read` then
 *          call multiple consume. But be sure continues `read` may
 *          cause a dead lock because the buffer is full (not consumed)
 */
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
  std::size_t ofs_consumer_;
  std::size_t ofs_writer_;

  std::queue<std::size_t> wait_read_;
  std::queue<std::size_t> wait_consume_;
  std::queue<void*> alloc_buffer_;

  void* buffer_;

};



#endif