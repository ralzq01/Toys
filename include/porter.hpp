
#ifndef _PORTER_H_
#define _PORTER_H_

#include <memory>
#include <cstring>
#include <iostream>
#include <safequeue.hpp>


/*! \brief Porter for transfering data between two threads
 *  Only support 1 consumer and 1 producer
 *  3 operations:
 *    `write`: for producer putting data into buffer
 *    `read`: for consumer getting data from buffer (withou copy)
 *    `consume`: indicate previous data have been consumed and can
 *               be covered.
 *  Notice: number of `read` call and number of `consume` call
 *          should be equal. You can first call multiple `read` then
 *          call multiple consume. But be aware continues `read` may
 *          cause a dead lock because the buffer is full (not consumed)
 */

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

  ~Porter();

  /*! \brief write a buffer
   */
  void write(const void* buffer, std::size_t size);

  /*! \brief read a buffer from RingBuffer
   *  Get a buffer ptr and it's size withou copy
   */
  void read(void** buffer, std::size_t& size);

  void lastRead(void** buffer, std::size_t& size);

  /*! \brief notification a buffer has been consumed
   *  Indicate that the content of read buffer is useless
   *  so this buffer's content can be covered.
   *  Make sure this func should be called after `read`
   *  and number of `consume` and `read` should be equal.
   */
  void consume();

  /*! \breif dynamically change the max allocate size
   *  may fail due to current allocation memory is
   *  larger than the required resized number
   */
  bool resize(std::size_t size);

 protected:
  
  std::size_t max_size_;
  std::size_t current_size_;
  void* last_read_;
  std::size_t last_size_;

  std::mutex size_mtx_;
  std::condition_variable not_full_;

  SafeQueue<Item> logs_;
  std::queue<Item> wait_consume_;
};

#endif