/*
 * file name: app_worker.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2016-04-01
 * history  :
 */
#ifndef __APP_WORKER_H__
#define __APP_WORKER_H__

#include "data_interface.h"

#include "worker.h"

class AppWorker : public Worker {
public:
	AppWorker() {
		type = APP_LAYER;
	}
	~AppWorker() {
	}

	XxbufQue* generate_channel(LayerType _type) {
		XxbufQue* que = NULL;
		switch (_type) {
		default:
			que = new XxbufQue(sizeof(TransApp), 10240);	
		break;
		}

		add_in_channel(_type, que);

		return que;
	}

};

#endif // __APP_WORKER_H__
