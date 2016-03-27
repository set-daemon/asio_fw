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
		// TODO 数据变化通知：有新数据、通道断开
		DataMsg* msg = (DataMsg*)(data + sizeof(DataSrc));

		// TODO 如果是通道断开，则关闭
		if (msg->op == CHANNEL_LEASE) {
			// 清除
			SocketHttpParseInfoIter iter = worker->socket_httpinfo.find(data_src->fd);
			if (iter != worker->socket_httpinfo.end()) {
				delete iter->second;
				worker->socket_httpinfo.erase(iter);
			}
			fprintf(stdout, "断开通知\n");
			continue;
		}

		data = data + sizeof(DataSrc); // 真正数据起点
		int data_size = data_block->size;
	
		parser::HttpParseInfo* parse_info = NULL;
		SocketHttpParseInfoIter iter = worker->socket_httpinfo.find(data_src->fd);
		if (iter == worker->socket_httpinfo.end()) {
			// 未找到，创建新的
			parse_info = new parser::HttpParseInfo();
			worker->socket_httpinfo[data_src->fd] = parse_info;
		} else {
			parse_info = iter->second;
		}

		// 拷贝数据
		memcpy((char*)parse_info->http_data.data, data, data_size);
		parse_info->http_data.len += data_size;

		// ** http解析
		int ret = parser::http_req_parse(*parse_info);
		if (0 != ret) {
		}
		worker->data_que.lease_data_block();
	}
}
