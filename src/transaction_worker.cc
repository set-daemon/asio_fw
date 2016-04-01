
#include "transaction_worker.h"

int TransactionWorker::init(const string& cf) {
	return 0;
}

int TransactionWorker::run() {

	if (0 != pthread_create(&worker_pthread, NULL, TransactionWorker::worker_cb, this)) {
		return -1;
	}

	return 0;
}

void* TransactionWorker::worker_cb(void* arg) {
	TransactionWorker* worker = (TransactionWorker*)arg;

	while (true) {
		// 获取来自SESSION的数据
		XxbufQue* in_que_sess = worker->get_inque(SESSION_LAYER);
		if (NULL == in_que_sess) {
			return NULL;
		}
		DataBlock* block = in_que_sess->get_data_block();
		if (NULL != block) {
			// 处理数据
			in_que_sess->lease_data_block(block);
		}

		// 获取来自TC的数据
		XxbufQue* in_que_tc = worker->get_inque(TRANSACTION_CONTROLLER_LAYER);
		if (NULL == in_que_tc) {
			return NULL;
		}
		block = in_que_tc->get_data_block();
		if (NULL != block) {
			// 处理数据
			in_que_tc->lease_data_block(block);
		}

	}

	return arg;
}
