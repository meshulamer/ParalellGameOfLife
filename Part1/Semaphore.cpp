#include "Semaphore.hpp"
//Implementation

//Constructors for Semaphore
Semaphore::Semaphore(){
    pthread_mutex_init(&cons_mutex, nullptr);
    pthread_cond_init(&cons_cond, nullptr);
	pthread_mutex_init(&prod_mutex, nullptr);
    pthread_cond_init(&prod_cond, nullptr);
    counter = 0;
	returned = 0;


}

Semaphore::Semaphore(unsigned int val): counter(val){
    pthread_mutex_init(&cons_mutex, nullptr);
    pthread_cond_init(&cons_cond, nullptr);
	pthread_mutex_init(&prod_mutex, nullptr);
    pthread_cond_init(&prod_cond, nullptr);
	returned = 0;
}


//Methods:

void Semaphore::up() {
	pthread_mutex_lock(&prod_mutex);
    producer_queue.push pthread_self());
    while(producer_queue.front() != pthread_self()){
        pthread_cond_wait(&prod_cond, &prod_mutex);
    }
    returned++;
    producer_queue.pop();
	if(!producer_queue.empty()){
		pthread_cond_broadcast(&prod_cond);
    }
	else{
		pthread_cond_broadcast(&cons_cond);	
	}
    pthread_mutex_unlock(&prod_mutex);
}

void Semaphore::down() {
    pthread_mutex_lock(&cons_mutex);
    consumer_queue.push(pthread_self());
    while(consumer_queue.front() != pthread_self()){
        pthread_cond_wait(&cons_cond, &cons_mutex);
    }
	if(counter == 0) {
        pthread_mutex_lock(&prod_mutex);
        while (returned != 0) {
            pthread_cond_wait(&prod_cond, &prod_mutex);
            counter = returned;
            returned = 0;
            pthread_cond_broadcast(&prod_cond);
            pthread_mutex_unlock(&prod_mutex);
        }
        counter--;
        consumer_queue.pop();
        if (!consumer_queue.empty()) {
            pthread_cond_broadcast(&cons_cond);
        }
        pthread_mutex_unlock(&cons_mutex);
    }

}