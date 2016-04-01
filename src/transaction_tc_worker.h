/*
 * file name: transaction_tc_worker.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2016-04-01
 * history  :
 */
#ifndef __TRANSACTION_TC_WORKER_H__
#define __TRANSACTION_TC_WORKER_H__

#include "worker.h"

class TransactionTcWorker : public Worker {
public:
	TransactionTcWorker() {
		type = TRANSACTION_CONTROLLER_LAYER;
	}
	~TransactionTcWorker() {
	}

	XxbufQue* generate_channel(LayerType _type) {
		XxbufQue* que = NULL;
		switch (_type) {
		case TRANSACTION_LAYER:
			que = new XxbufQue(sizeof(TransTc), 20480);
		break;
		case APP_LAYER:
			que = new XxbufQue(sizeof(AppTc), 20480);
		break;
		}
		add_in_channel(_type, que);

		return que;
	}

	XxbufQue* get_channel(LayerType _type) {
		// APP层通常工作在多线程模式，所以每次获取都需要新生成，可定制
		XxbufQue* que = NULL;
		switch (_type) {
			case APP_LAYER: {
				map<LayerType,vector<XxbufQue*> >::iterator iter = in_channels.find(_type);
				if (in_channels.end() == iter) {
					vector<XxbufQue*> vec;
					que = new XxbufQue(sizeof(AppTc),10240);
					vec.push_back(que);
					in_channels.insert(pair<LayerType,vector<XxbufQue*> >(_type, vec));
					iter = in_channels.find(_type);
				} else {
					que = new XxbufQue(sizeof(AppTc),10240);
					iter->second.push_back(que);
				}
			}
			break;
			default: {
				map<LayerType,vector<XxbufQue*> >::iterator iter = in_channels.find(_type);
				if (in_channels.end() == iter) {
					vector<XxbufQue*> vec;
					que = new XxbufQue(1024,1024);
					vec.push_back(que);
					in_channels.insert(pair<LayerType,vector<XxbufQue*> >(_type, vec));
				} else {
					que = iter->second[0];
				}
			}
		}
		return que;
	}
};

#endif // __TRANSACTION_TC_WORKER_H__

