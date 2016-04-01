/*
 * file name: transaction_worker.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2016-03-26
 * history  :
 */
#ifndef __TRANSACTION_WORKER_H__
#define __TRANSACTION_WORKER_H__

#include "data_interface.h"
#include "xxbuf_que.h"

#include "worker.h"

class TransactionWorker : public Worker {
public:
	TransactionWorker() {
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

};

#endif // __TRANSACTION_WORKER_H__
