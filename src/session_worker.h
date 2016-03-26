/*
 * file name: session_worker.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2016-03-26
 * history  :
 */
#ifndef __SESSION_WORKER_H__
#define __SESSION_WORKER_H__

#include <pthread.h>

#include "xxbuf_que.h"

#include <string>
using namespace std;

class SessionWorker {
public:
	SessionWorker(XxbufQue &_data_que) : data_que(_data_que){
	}
	~SessionWorker(){
	}

	int init(const string& cfg);

	int run();

private:
	static void* worker_cb(void* arg);

private:
	XxbufQue 	&data_que;
	pthread_t 	worker_pthread;
};

#endif // __SESSION_WORKER_H__
