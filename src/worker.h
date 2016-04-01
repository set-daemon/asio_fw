/*
 * file name: worker.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2016-04-01
 * history  :
 *    20160401 作为所有分层的基类
 */

#ifndef __WORKER_H__
#define __WORKER_H__


#include <string>
#include <vector>
#include <map>
using namespace std;

#include "data_interface.h"
#include "xxbuf_que.h"

typedef int WorkerId;

class Worker {
public:
	Worker() {
		worker_id = ++count;
	}
	virtual ~Worker() {
	}

	virtual int init(const string& cf) {
		return 0;
	}

	virtual int run() {
		return 0;
	}

	static void join(Worker& w1, Worker& w2) {
		// 单向绑定：w2的输入为w1的输出
		w1.set_out_channel(w2);
	}

	XxbufQue* get_inque(LayerType _type) {
		map<LayerType,vector<XxbufQue*> >::iterator iter = in_channels.find(_type);
		if (iter == in_channels.end()) {
			return NULL;
		}

		return iter->second[0];
	}

	// 用于生成通道关系时使用
	virtual XxbufQue* generate_channel(LayerType _type) {
		fprintf(stdout, "Worker %s %d\n", __FUNCTION__, __LINE__);
		XxbufQue* que = new XxbufQue(1024, 1024);
		return que;
	}

	// 用于生成通道关系时使用
	virtual XxbufQue* get_channel(LayerType _type) {
		// APP层通常工作在多线程模式，所以每次获取都需要新生成，可定制
		map<LayerType,vector<XxbufQue*> >::iterator iter = in_channels.find(_type);
		if (in_channels.end() == iter) {
			return NULL;
		}

		return iter->second[0];
	}

	LayerType get_layertype() {
		return type;
	}
	WorkerId get_workerid() {
		return worker_id;
	}

protected:
	void set_out_channel(Worker& w) {
		// 如果w没有准备该layer的则临时创建
		XxbufQue* que = w.get_channel(type);
		if (NULL == que) {
			que = w.generate_channel(type);
		}
		LayerType w_layertype = w.get_layertype();
		WorkerId  w_worker_id = w.get_workerid();

		map<LayerType, map<WorkerId, XxbufQue*> >::iterator iter = out_channels.find(w_layertype);
		if (iter == out_channels.end()) {
			map<WorkerId, XxbufQue*> m;
			m[w_worker_id] = que;
			out_channels[w_layertype] = m;
		} else {
			iter->second[w_worker_id] = que;	
		}
	}

	void add_in_channel(LayerType _type, XxbufQue* que) {
		map<LayerType, vector<XxbufQue*> >::iterator iter = in_channels.find(_type);
		if (iter != in_channels.end()) {
			iter->second.push_back(que);
			return;
		}
		vector<XxbufQue*> vec;
		vec.push_back(que);
		in_channels[_type] = vec;
	}
protected:
	WorkerId 	worker_id;
	LayerType 	type;

	map<LayerType, vector<XxbufQue*> > in_channels;
	map<LayerType, map<WorkerId, XxbufQue*> > out_channels;

	static WorkerId count;
};


#endif // __WORKER_H__
