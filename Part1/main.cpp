#include <iostream>
#include "Semaphore.hpp"
#include <pthread.h>


bool up() {
    pthread_t m_thread;
    if (pthread_create(&m_thread, nullptr, upsema, nullptr) == 0) return true;
    return false;
}

bool down() {
    pthread_t m_thread;
    if (pthread_create(&m_thread, nullptr, downsema, nullptr) == 0) return true;
    return false;
}

void upsema(){

}
void downsema(){

}
int main() {
    Semaphore sema(3);

}


