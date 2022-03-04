/*
 * consumers.h
 *
 *  Created on: Sep 12, 2013
 *      Author: zlp
 */

#ifndef CONSUMERS_H_
#define CONSUMERS_H_
#include "logic_operate.h"

class consumers{

struct DSQueue{
	/* An array of elements in the queue. */
	std::string *buf;

	/* The position of the first element in the queue. */
	uint32_t pos;

	/* The number of items currently in the queue.
	 * When `length` = 0, ds_queue_get will block.
	 * When `length` = `capacity`, ds_queue_put will block. */
	uint32_t length;

	/* The total number of allowable items in the queue */
	uint32_t capacity;

	/* When true, the queue has been closed. A run-time error will occur
	 * if a value is sent to a closed queue. */
	bool closed;

	/* Guards the modification of `length` (a condition variable) and `pos`. */
	pthread_mutex_t mutate;

	/* A condition variable that is pinged whenever `length` has changed or
	 * when the queue has been closed. */
	pthread_cond_t cond_length;
};
public:
	consumers(int consumer_num, int queue_capacity);
	void runAllConsumers();
	void putData(const std::string &item);
	std::string getData(void);
	virtual ~consumers();

private:

	struct DSQueue c_queue;
	int c_num;
	pthread_t *consumers_t;
	static void* consumerRun(void *);
};

#endif /* CONSUMERS_H_ */
