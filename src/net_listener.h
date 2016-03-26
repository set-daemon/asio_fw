/*
 * file name: net_listener.h
 * author   : set_daemon@126.com
 * date     : 2016-01-22
 */
#ifndef __NET_LISTENER_H__
#define __NET_LISTENER_H__

#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <event.h>

#include <string>
#include <map>
using namespace std;

#include "net_addr.h"
#include "xxbuf_que.h"

class SockEvent {
public:
	SockEvent(): rd_ev(NULL), wr_ev(NULL) {
	}

	~SockEvent() {
		if (rd_ev) {
			event_del(rd_ev);
			event_free(rd_ev);
		}
		if (wr_ev) {
			event_del(wr_ev);
			event_free(wr_ev);
		}
	}

public:
	struct event* rd_ev;
	struct event* wr_ev;
};

class NetListener {
public:
	NetListener(XxbufQue& buf_que): listen_fd(-1), ev_base(NULL),data_que(buf_que) {
	}
	~NetListener() {
	}

	int roll_on(const string& addr) {
		if (create(addr) != 0) {
			return -1;
		}

		event_base_dispatch(ev_base);

		return 0;
	}

private:

	static void ev_accept_proc(int lis_fd, short ev, void *arg);
	static void ev_read_proc(int lis_fd, short ev, void *arg);
	static void ev_write_proc(int lis_fd, short ev, void *arg);

	int create(const string& server_addr) {
		listen_addr.set(server_addr);
		listen_fd = listen_addr.create_server_socket();

		ev_base = event_base_new();
		struct event *ev = event_new(ev_base, listen_fd, EV_READ|EV_PERSIST, ev_accept_proc, this);
		//event_base_set(ev_base, ev);
		event_add(ev, NULL);

		return 0;
	}
	
	void del_sock_event(int fd) {
		map<int,SockEvent>::iterator iter = sock_events.find(fd);
		if (iter != sock_events.end()) {
			sock_events.erase(iter);
		}
		close(fd);
	}

	SockEvent* find_sock_event(int fd) {
		map<int,SockEvent>::iterator iter = sock_events.find(fd);
		if (iter != sock_events.end()) {
			return &iter->second;
		}
		sock_events.insert(pair<int,SockEvent>(fd, SockEvent()));

		return &sock_events[fd];
	}

private:
	map<int,SockEvent> 	sock_events;
	int						listen_fd;
	NetAddr					listen_addr;
	struct event_base 		*ev_base;
	XxbufQue&				data_que;
};

#endif // __NET_LISTENER_H__
