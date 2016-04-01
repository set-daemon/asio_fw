#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>
using namespace std;

#include "xxbuf_que.h"
#include "net_listener.h"
#include "session_worker.h"
#include "transaction_worker.h"
#include "transaction_tc_worker.h"
#include "app_worker.h"

int main(int argc, char *argv[]) {
	string cf = "";

	// 从上到下初始化
	vector<AppWorker*> app_workers(5);
	for (int i = 0; i < 5; ++i) {
		AppWorker* app_worker = new AppWorker();
		app_worker->init(cf);
		app_workers.push_back(app_worker);
	}

	TransactionWorker* t_worker = new TransactionWorker();
	t_worker->init(cf);

	TransactionTcWorker* tc_worker = new TransactionTcWorker();
	tc_worker->init(cf);

	SessionWorker *session_worker = new SessionWorker();
	session_worker->init(cf);

	NetListener *listener = new NetListener();
	listener->init(cf);

	// 通道打通
	Worker::join(*listener, *session_worker);
	Worker::join(*session_worker, *t_worker);
	Worker::join(*t_worker, *tc_worker);
	Worker::join(*tc_worker, *t_worker);
	for (int i = 0; i < app_workers.size(); ++i) {
		Worker::join(*t_worker, *app_workers[i]);
		Worker::join(*app_workers[i], *tc_worker);
	}

	string addr = "tcp:127.0.0.1:18981";	
	//string addr = "tcp:192.168.1.34:18981";	
	listener->roll_on(addr);

	return 0;
}
