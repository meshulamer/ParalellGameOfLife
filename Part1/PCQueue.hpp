#ifndef _QUEUEL_H
#define _QUEUEL_H
#include "Headers.hpp"
// Single Producer - Multiple Consumer queue
template <typename T>class PCQueue
{

public:
	PCQueue(){
		pushed_queue = new queue<T>();
		available_queue = new queue<T>();
		pthread_mutex_init(&cons_mutex, NULL);
		pthread_cond_init(&cons_cond, NULL);
		pthread_mutex_init(&prod_mutex, NULL);
		pthread_cond_init(&prod_cond, NULL);
	}
	// Blocks while queue is empty. When queue holds items, allows for a single
	// thread to enter and remove an item from the front of the queue and return it. 
	// Assumes multiple consumers.
	T pop(){
		pthread_mutex_lock(&cons_mutex);
		consumer_queue.push(pthread_self());
		while(consumer_queue.front() != pthread_self()){
			pthread_cond_wait(&cons_cond, &cons_mutex);
		}
		if(available_queue.empty()){
			pthread_mutex_lock(&prod_mutex);
			while(!(pushed_queue.empty()){
				pthread_cond_wait(&prod_cond, &prod_mutex);
			*queue<typename T> temp = pushed_queue;
			pushed_queue = available_queue;
			available_queue = temp;
			pthread_cond_broadcast(&prod_cond);
			pthread_mutex_unlock(&prod_mutex);
		}
		T poped_item = available_queue->pop();
		consumer_queue.pop();
		if(!consumer_queue.empty()){
			pthread_cond_broadcast(&cons_cond);
		}
		pthread_mutex_unlock(&cons_mutex);	
	}
    }
	// Allows for producer to enter with *minimal delay* and push items to back of the queue.
	// Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.  
	// Assumes single producer 
	void push(const T& item){
		pthread_mutex_lock(&prod_mutex);
		producer_queue.push(pthread_self());
		while(producer_queue.front() != pthread_self()){
				pthread_cond_wait(&prod_cond, &prod_mutex);
			}
			pushed_queue->push(item);
			producer_queue.pop();
			if(!producer_queue.empty()){
				pthread_cond_broadcast(&prod_cond);
			}
			else{
				pthread_cond_broadcast(&cons_cond);
			}
			pthread_mutex_unlock(&prod_mutex);
	}


private:
    unsigned int counter;
	pthread_mutex_t cons_mutex;
	pthread_cond_t cons_cond;
	pthread_mutex_t prod_mutex;
	pthread_cond_t prod_cond;
	queue<pthread_t> consumer_queue;
    queue<pthread_t> producer_queue;
	queue<T>* pushed_queue;
    queue<T>* available_queue;
	
};
// Recommendation: Use the implementation of the std::queue for this exercise
#endif