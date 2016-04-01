/*
 * file name: transaction_worker.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2016-03-26
 * history  :
 */
#ifndef __TRANSACTION_WORKER_H__
#define __TRANSACTION_WORKER_H__

#include "xxbuf_que.h"
#include "mempool.h"
#include "data_interface.h"

#include "worker.h"

class TransactionWorker : public Worker {
public:
	TransactionWorker() : pool(10240, 50000){
		type = TRANSACTION_LAYER;
	}
	~TransactionWorker() {
	}

	XxbufQue* generate_channel(LayerType _type) {
		XxbufQue* que = NULL;
		switch (_type) {
		case SESSION_LAYER:
			que = new XxbufQue(8192, 20480);
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
	map<TransId, BlockData*> 	trans;
};

#endif // __TRANSACTION_WORKER_H__
