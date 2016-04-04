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
		XxbufQue* in_que = worker->get_inque(LISTENER_LAYER);
		DataBlock* data_block = in_que->wait();
		if (data_block == NULL) {
			continue;
		}

		//n0_string::hex_print((char*)data_block, worker->in_que.cfg_block_size(), "整块数据", 20);
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
			in_que->lease_data_block();
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
		}
		if (parse_info == NULL) {
			fprintf(stderr, "SESSION-WORKER 未能找到存储socket%d的缓存\n", data_src->fd);
			in_que->lease_data_block();
			continue;
		}

		// 拷贝数据
		int cap = parse_info->http_data.size - parse_info->http_data.len;
		if (cap < data_size) {
			fprintf(stderr, "SESSION-WORKER 没有足够的空间%d\n", cap);
			in_que->lease_data_block();
			continue;
		}
		memcpy((char*)parse_info->http_data.data+parse_info->http_data.len, data, data_size);
		parse_info->http_data.len += data_size;
		// 释放数据块
		in_que->lease_data_block();

		// ** http解析
		int ret = parser::http_req_parse(*parse_info);
		if (0 != ret) {
			fprintf(stderr, "解析失败\n");
		}

		//fprintf(stdout, "status=%d\n", parse_info->status.status);
		if (parse_info->status.status == parser::PARSE_COMPLETED) {
			//http_req_meta_print(parse_info->http_data.meta);
			// TODO 生成事务数据并传递给事务处理层
			XxbufQue* t_que = worker->get_outque(TRANSACTION_LAYER);		
			DataBlock* t_block = t_que->get_free_block();
			if (NULL != t_block) {
				int data_len = parse_info->status.offset;
				char *data_addr = DATABLK_ADDR(t_block);
				int size = t_block->size;
				if (size > sizeof(SessTrans)) {
					SessTrans* st = (SessTrans*)(data_addr);
					// 拷贝
					memcpy(&st->h.ds, data_src, sizeof(DataSrc));
					st->h.dm.op = UPLOAD_DATA;
					memcpy(&st->http_meta, &parse_info->http_data.meta, sizeof(HttpReqMeta));
					data_addr = (char*)(data_addr + sizeof(SessTrans));
					st->http_meta.http_start = data_addr;
					st->httpdata_len = data_len;
					// 拷贝原始数据
					int remain_size = size - sizeof(SessTrans);
					if (remain_size >= parse_info->status.offset) {
						memcpy(data_addr, parse_info->http_data.data, data_len);
						t_block->size = data_len + sizeof(SessTrans);
						t_que->add_data_block(t_block);
					} else {
						fprintf(stderr, "无充足空间");
					}
				}
			}

			// 将剩余的数据挪至最前面
			int unparsed_data_len = parse_info->http_data.len - parse_info->status.offset + 1;
			if (unparsed_data_len > 0) {
				memcpy((char*)parse_info->http_data.data, parse_info->status.offset + parse_info->http_data.data, unparsed_data_len);
				parse_info->http_data.len = unparsed_data_len;
				memset(&parse_info->http_data.meta, 0x00, sizeof(HttpReqMeta));
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
