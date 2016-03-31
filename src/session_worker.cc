#include "n0_string.h"
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

		//n0_string::hex_print((char*)data_block, worker->data_que.cfg_block_size(), "整块数据", 20);

		char* data = DATABLK_ADDR(data_block);
		DataSrc * data_src = (DataSrc*)data; // 数据源
		// TODO 数据变化通知：有新数据、通道断开
		DataMsg* msg = (DataMsg*)(data + sizeof(DataSrc));

		parser::HttpParseInfo* parse_info = NULL;
		SocketHttpParseInfoIter iter = worker->socket_httpinfo.find(data_src->fd);
		if (iter != worker->socket_httpinfo.end()) {
			parse_info = iter->second;
		}
		// TODO 如果是通道断开，则关闭
		if (msg->op == CHANNEL_LEASE) {
			// 清除
			if (iter != worker->socket_httpinfo.end()) {
				delete iter->second;
				worker->socket_httpinfo.erase(iter);
			}
			//fprintf(stdout, "获取消息：断开通知\n");
			worker->data_que.lease_data_block();
			continue;
		}

		data = data + sizeof(DataSrc) + sizeof(DataMsg); // 真正数据起点
		int data_size = data_block->size;
		//fprintf(stdout, "在SESSION中, %d 读取到%d字节，地址:%p,%p\n", data_src->fd, data_size, data, data_block);
		//n0_string::hex_print(data, data_size, "接收的数据", 20);
		if (parse_info == NULL) {
			// 未找到，创建新的
			//fprintf(stderr, "未找到socket历史数据\n");
			parse_info = new parser::HttpParseInfo();
			worker->socket_httpinfo[data_src->fd] = parse_info;
		} else {
			//fprintf(stderr, "找到socket历史数据 %d,%d,%d\n", parse_info->status.stage, parse_info->status.step, parse_info->status.offset);
		}
		if (parse_info == NULL) {
			fprintf(stderr, "SESSION-WORKER 未能找到存储socket%d的缓存\n", data_src->fd);
			worker->data_que.lease_data_block();
			continue;
		}

		// 拷贝数据
		int cap = parse_info->http_data.size - parse_info->http_data.len;
		if (cap < data_size) {
			fprintf(stderr, "SESSION-WORKER 没有足够的空间%d\n", cap);
			worker->data_que.lease_data_block();
			continue;
		}
		memcpy((char*)parse_info->http_data.data+parse_info->http_data.len, data, data_size);
		parse_info->http_data.len += data_size;
		// 释放数据块
		worker->data_que.lease_data_block();

		// ** http解析
		int ret = parser::http_req_parse(*parse_info);
		if (0 != ret) {
			fprintf(stderr, "解析失败\n");
		}

		//fprintf(stdout, "status=%d\n", parse_info->status.status);
		if (parse_info->status.status == parser::PARSE_COMPLETED) {
			//http_req_meta_print(parse_info->http_data.meta);
			// TODO 生成事务数据并传递给事务处理层
			//fprintf(stdout, "ok....%d,%d\n", parse_info->http_data.len, parse_info->status.offset);

			// 将剩余的数据挪至最前面
			int unparsed_data_len = parse_info->http_data.len - parse_info->status.offset + 1;
			if (unparsed_data_len > 0) {
				memcpy((char*)parse_info->http_data.data, parse_info->status.offset + parse_info->http_data.data, unparsed_data_len);
				parse_info->http_data.len = unparsed_data_len;
			} else {
				parse_info->http_data.len = 0;
			}
			// 重置状态
			parse_info->status.offset = 0;
			parse_info->status.stage = parser::CMD_LINE_STAGE;
			parse_info->status.step = parser::METHOD_STEP;
		}
		// 特别的：重置状态为PARSE_WAIT
		parse_info->status.status = parser::PARSE_WAIT;
	}
}
