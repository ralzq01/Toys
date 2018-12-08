#include <porter.hpp>
#include <time.h>
#include <thread>

const std::size_t kBufferSize = 1 << 25; // set to 32MB
std::queue<void*> gen_buffer;
std::queue<std::size_t> send_size;
std::queue<void*> recv_buffer;
Porter ring;

void producer(){
    std::size_t total = 0;
    // randomly generate data
    while(total < (1 << 30)){
        std::size_t random_size = (rand() % (1 << 21)) + 1;
        char* random_buff = static_cast<char*>(std::malloc(random_size));
        for(std::size_t i = 0; i < random_size; ++i){
            random_buff[i] = rand() % 128;
        }

        ring.write((void*)random_buff, random_size);
        gen_buffer.push((void*)random_buff);
        printf("[Producer]: Sending buffer size of %zu bytes\n", random_size);

        total += random_size;
        send_size.push(random_size);
    }
    char end = 0;
    ring.write((void*)(&end), 0);
    printf("[Producer]: Total Sent Data: %zu bytes\n", total);
}

void consumer(){
    ring.resize(kBufferSize);
    std::size_t total = 0;
    void* buffer = nullptr;
    std::size_t recv = 0;
    while(true){
        // recving part
        ring.read(&buffer, recv);
        if(recv == 0){
            break;
        }
        printf("[Consumer]: Recving buffer size of %zu bytes\n", recv);
        // data handle part
        char* buff = static_cast<char*>(std::malloc(recv));
        if(buffer == nullptr){
            std::cerr << "error" << std::endl;
        }
        memcpy(buff, buffer, recv);
        recv_buffer.push((void*)buff);
        // consume
        ring.consume();
        total += recv;
    }
    printf("[Consumer]: Total Recved Data: %zu bytes\n", total);
}

void verify(){
    std::size_t gen_size = gen_buffer.size();
    std::size_t recv_size = recv_buffer.size();

    printf("\nVerify...\n");
    printf("Sending num: %zu\n", gen_size);
    printf("Recving num: %zu\n", recv_size);

    if(gen_size != recv_size){
        std::cerr << "Error: sending num != recv num" << std::endl;
    }
    std::size_t check_size = gen_size < recv_size ? gen_size : recv_size;
    for(std::size_t idx = 0; idx < check_size; ++idx){
        void* p_gen = gen_buffer.front();
        void* p_recv = recv_buffer.front();
        int diff = memcmp(p_gen, p_recv, send_size.front());
        if(diff != 0){
            printf("Error: %zu-th buffer is different\n", idx + 1);
            exit(-2);
        }
        std::free(p_gen);
        std::free(p_recv);
        send_size.pop();
        gen_buffer.pop();
        recv_buffer.pop();
    }
    printf("All buffer has been verified correct\n");
}

int main(){
    srand((unsigned)time(NULL));
    std::thread prod(producer);
    std::thread cons(consumer);
    prod.join();
    cons.join();
    verify();
    return 0;
}