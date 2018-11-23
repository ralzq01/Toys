## Toys

A toy tool for C++ library.

### Components

* `Ring Buffer`: Data transfer between producer and consumer. Each item may have different memory size. Notice if you use this to transfer data size larger than maximum of `std::size_t`, you should make it not overflow by handling the pointers by yourself.