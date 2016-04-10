/*
 * file name: transaction_worker.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2016-03-26
 * history  :
 */
#ifndef __TRANSACTION_WORKER_H__
#define __TRANSACTION_WORKER_H__

#include <string>
using namespace std;

#include "xxbuf_que.h"
#include "mempool.h"
#include "data_interface.h"

#include "worker.h"

typedef int (*pre_processor_fun)(HttpReqMeta* http_meta, char*http_data, int http_data_len, BufInfo& write_buf, void* ctx);

struct PreProcessor {
	pre_processor_fun f;
	void *ctx;
};

class TransIdCmp {
public:
	bool operator()(const TransId& t1, const TransId& t2) const {
		int ret = memcmp(t1, t2, sizeof(TransId));
		if (ret > 0) {
			return true; 
		}
		return false;
	}
};

class TransactionWorker : public Worker {
public:
	TransactionWorker() : pool(10240, 50000){
		type = TRANSACTION_LAYER;
		pre_processor.f = (pre_processor_fun)0;
		pre_processor.ctx = NULL;
		pool.init();
	}
	~TransactionWorker() {
	}

	int init(const string& cf);
	int run();

	void set_preprocessor(pre_processor_fun f, void *ctx) {
		pre_processor.f = f;
		pre_processor.ctx = ctx;
	}

	// 与Session、Transaction Control层有交互
	XxbufQue* generate_channel(LayerType _type) {
		XxbufQue* que = NULL;
		switch (_type) {
		case SESSION_LAYER:
			que = new XxbufQue(8192, 66960);
		break;
		case TRANSACTION_CONTROLLER_LAYER:
			que = new XxbufQue(sizeof(TcTrans), 20480);
		break;
		default:
			que = new XxbufQue(sizeof(TransApp), 10240);	
		break;
		}

		add_in_channel(_type, que);

		return que;
	}

private:
	static void* worker_cb(void* arg);

private:
	pthread_t 					worker_pthread;
	MemPool						pool; //用于存放事务数据：由数据源信息(32)、httpMeta信息(300)、原数据(4096)、待响应(1024)、APP响应块(5120)组成
	map<string, DataBlock*> 	trans;
	//
	PreProcessor				pre_processor;
};

#endif // __TRANSACTION_WORKER_H__
