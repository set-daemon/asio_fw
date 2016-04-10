/*
 * file name: session_worker.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2016-03-26
 * history  :
 */
#ifndef __SESSION_WORKER_H__
#define __SESSION_WORKER_H__

#include <pthread.h>

#include <string>
#include <map>
using namespace std;

#include "xxbuf_que.h"
#include "http_meta.h"
#include "http_parser.h"
#include "worker.h"


typedef map<int,parser::HttpParseInfo* > SocketHttpParseInfo;
typedef SocketHttpParseInfo::iterator SocketHttpParseInfoIter;

class SessionWorker : public Worker {
public:
	SessionWorker() {
		type = SESSION_LAYER;
	}
	~SessionWorker() {
	}

	int init(const string& cfg);

	int run();

	virtual XxbufQue* generate_channel(LayerType _type) {
		fprintf(stdout, "SessionWorker %s %d\n", __FUNCTION__, __LINE__);
		XxbufQue* que = NULL;
		switch (_type) {
			case LISTENER_LAYER: {
				que = new XxbufQue(4096, 60960);
				add_in_channel(_type, que);
			}
			break;
		}

		return que;
	}

private:
	static void* worker_cb(void* arg);

private:
	pthread_t 			worker_pthread;
	SocketHttpParseInfo socket_httpinfo;
};

#endif // __SESSION_WORKER_H__
