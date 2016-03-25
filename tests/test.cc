#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xxbuf_que.h"
#include "net_listener.h"

int main(int argc, char *argv[]) {
	XxbufQue que(1024, 10240);

	if (!que.is_ok()) {
		fprintf(stderr, "que创建有问题\n");
		return -1;
	}

	NetListener listener(que);

	//string addr = "tcp:127.0.0.1:18981";	
	string addr = "tcp:192.168.1.34:18981";	
	listener.roll_on(addr);

	return 0;
}
