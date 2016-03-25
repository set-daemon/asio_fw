#include <errno.h>

#include "net_listener.h"

void NetListener::ev_accept_proc(int lis_fd, short ev, void *arg) {
	NetListener* listener = (NetListener*)arg;

	int cli_fd = 0;
	struct sockaddr_in cli_addr;
	socklen_t addr_len = sizeof(cli_addr);
	cli_fd = accept(lis_fd, (struct sockaddr*)&cli_addr, &addr_len);
	if (cli_fd < 0) {
		return;
	}

	SockEvent* sock_evs = listener->find_sock_event(cli_fd);
	sock_evs->rd_ev = event_new(listener->ev_base, cli_fd, EV_READ|EV_PERSIST, NetListener::ev_read_proc, arg);
	event_add(sock_evs->rd_ev, NULL);
}

void NetListener::ev_read_proc(int lis_fd, short ev, void *arg) {
	NetListener* listener = (NetListener*)arg;

	XxbufQue& data_que = listener->data_que;
	DataBlock *data_blk = data_que.get_free_block();	
	if (NULL == data_blk) {
		fprintf(stdout, "data_que is full\n");
		return;
	}

	char* buf = DATABLK_ADDR(data_blk);
	int to_read = data_blk->size-1;
	int total_read = 0;

	// 读取数据
	int r_len = 0;
	char *p = buf;
	do {
		r_len = recv(lis_fd, p, to_read, MSG_DONTWAIT);
		if (r_len <= 0) {
			//fprintf(stdout, "{EAGAIN=%d,EBADF=%d,EINTR=%d}errno = %d\n, errstr = %s\n", EAGAIN,EBADF,EINTR,errno, strerror(errno));
			break;
		}
		fprintf(stdout, "read...\n");
		total_read += r_len;
		to_read -= total_read;
		p += r_len;
	} while (r_len > 0 && to_read > 0);
	if (r_len == -1 && (errno != EAGAIN)) {
		fprintf(stdout, "读异常\n");
		listener->del_sock_event(lis_fd);
		return;
	} else if (r_len == 0) {
		fprintf(stdout, "断开\n");
		listener->del_sock_event(lis_fd);
		return;
	}

	if (total_read > 0) {
		*(buf+total_read) = '\0';
		data_blk->size = total_read;
		data_que.add_data_block(data_blk);
		fprintf(stdout, "read [%d], %p %p [%s]\n", data_blk->size, data_blk, buf, buf);
		//fprintf(stdout, "111[");
		for (int i = 0; i < total_read; ++i) {
			//fprintf(stdout, "{%02x %c }", *(unsigned char*)(buf+i), (char)buf[i]);
			//fprintf(stdout, "{%02x }", *(unsigned char*)(buf+i));
		}
		//fprintf(stdout, "]\n");
	}

	SockEvent* sock_evs = listener->find_sock_event(lis_fd);
	if (sock_evs->wr_ev == NULL) {
		sock_evs->wr_ev = event_new(listener->ev_base, lis_fd, EV_WRITE|EV_ET, NetListener::ev_write_proc, arg);
		fprintf(stdout, "设置写事件\n");
	}
	event_add(sock_evs->wr_ev, NULL);
	//event_base_set(listener->ev_base, &session->ev);
}

//static const char *test_rsp = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: 0\r\nDate: Fri, 25 Mar 2016 09:12:29 GMT\r\nServer: nginx/1.8.0\r\n\r\n";
static const char *rsp_body = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"></meta><title>测试</title></head><body></body></html>";
static const char *test_rsp = "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n%s";

void NetListener::ev_write_proc(int lis_fd, short ev, void *arg) {
	NetListener* listener = (NetListener*)arg;
	char buf[1024];
	int n = sprintf(buf, test_rsp, strlen(rsp_body), rsp_body);
	// 获取写数据
	fprintf(stdout, "写数据\n");
	int ret = write(lis_fd, buf, n);
	if (ret == -1) {
		fprintf(stdout, "写失败\n");
		listener->del_sock_event(lis_fd);
	}
#if 0
	// 不能设置读，否则会进入该事件会一直被触发
	event_set(session->ev, lis_fd, EV_READ|EV_WRITE|EV_PERSIST, NetListener::ev_read_proc, arg);
	event_base_set(listener->ev_base, &session->ev);
	event_add(&session->ev, NULL);
#endif
}
