/*
 * consumers.cc
 *
 *  Created on: Sep 12, 2013
 *      Author: zlp
 */
#include <cassert>
#include <iostream>
#include "consumers.h"

consumers::consumers(int consumer_num, int queue_capacity) {
	c_num = consumer_num;
	c_queue.capacity = queue_capacity;
	assert(c_num > 0);
	assert(c_queue.capacity > 0);
	int err;
	c_queue.pos = 0;
	c_queue.length = 0;
	c_queue.closed = false;

	c_queue.buf = new std::string[c_queue.capacity];
	assert(c_queue.buf);

	if (0 != (err = pthread_mutex_init(&c_queue.mutate, NULL))) {
		LOG4CXX_ERROR(g_logger, "Could not create mutex. Errno: " << strerror(errno));
	    exit(1);
	}
	if (0 != (err = pthread_cond_init(&c_queue.cond_length, NULL))) {
		LOG4CXX_ERROR(g_logger, "Could not create cond var. Errno: " << strerror(errno));
	    exit(1);
	}

	consumers_t = new pthread_t[c_num];
	if(consumers_t == NULL)
	{
		LOG4CXX_ERROR(g_logger, "consumers::consumers new pthread_t[c_num] failed");
		exit(1);
	}
}

std::string consumers::getData(void) {

	std::string item;
	pthread_mutex_lock(&c_queue.mutate);
	while (c_queue.length == 0) {
	    /* This is a bit tricky. It is possible that the queue has been closed
	     * *and* has become empty while `pthread_cond_wait` is blocking.
	     * Therefore, it is necessary to always check if the queue has been
	     * closed when the queue is empty, otherwise we will deadlock. */
	    if (c_queue.closed) {
	        pthread_mutex_unlock(&c_queue.mutate);
	        return std::string("");
	    }
	    pthread_cond_wait(&c_queue.cond_length, &c_queue.mutate);
	}

	assert(c_queue.length <= c_queue.capacity);
	assert(c_queue.length > 0);

	item = c_queue.buf[c_queue.pos];
	c_queue.buf[c_queue.pos] = std::string("");
	c_queue.pos = (c_queue.pos + 1) % c_queue.capacity;

	c_queue.length--;
	pthread_cond_broadcast(&c_queue.cond_length);

	pthread_mutex_unlock(&c_queue.mutate);

	return item;
}
void consumers::putData(const std::string &item) {

    pthread_mutex_lock(&c_queue.mutate);
    assert(!c_queue.closed);

    while (c_queue.length == c_queue.capacity)
        pthread_cond_wait(&c_queue.cond_length, &c_queue.mutate);

    assert(!c_queue.closed);
    assert(c_queue.length < c_queue.capacity);

    c_queue.buf[(c_queue.pos + c_queue.length) % c_queue.capacity] = item;
    c_queue.length++;
    pthread_cond_broadcast(&c_queue.cond_length);

    pthread_mutex_unlock(&c_queue.mutate);
}
void* consumers::consumerRun(void *p) {
	std::string message;
	consumers *anConsumer = (consumers *) p;
	while(true)
	{
		LOG4CXX_TRACE(g_logger, "consumers::consumerRun thread_id =  " << pthread_self());
		message=anConsumer->getData();
		if(message.length() > 0)
		{
			CAppOperate logic;
			logic.StartAppOpt(message);
		}
	}

	return NULL;
}

void consumers::runAllConsumers() {
	int i = 0;
	for(i = 0; i < c_num; i++)
	{
		 pthread_create(&consumers_t[i], NULL, consumerRun, (void*)this);
		 printf("printf consumer thread %d\n", i);
	}
}
consumers::~consumers() {
	delete [] c_queue.buf;
	delete [] consumers_t;
}

