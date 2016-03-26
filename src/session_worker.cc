
#include "session_worker.h"


int SessionWorker::init(const string& cfg) {

	return 0;
}

int SessionWorker::run() {

	if (0 != pthread_create(&worker_pthread, NULL, SessionWorker::worker_cb, this)) {
		return -1;
	}

	return 0;
}

void* SessionWorker::worker_cb(void* arg) {
	SessionWorker* worker = (SessionWorker*)arg;

	while (true) {
		// 信号量等待	
		DataBlock* data_block = worker->data_que.wait();
		if (data_block == NULL) {
			continue;
		}
		worker->data_que.lease_data_block();
	}
}
