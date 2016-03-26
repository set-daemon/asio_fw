#include "data_interface.h"

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
		// 信号量等待，获取数据块
		DataBlock* data_block = worker->data_que.wait();
		if (data_block == NULL) {
			continue;
		}
		char* data = DATABLK_ADDR(data_block);
		DataSrc * data_src = (DataSrc*)data; // 数据源

		// ** http解析
		// *** 查看 

		worker->data_que.lease_data_block();
	}
}
