/*
 * file name: net_addr.h
 * author   : set_daemon@126.com
 * history  : 2016-01-22
 */
#ifndef __NET_ADDR_H__
#define __NET_ADDR_H__

#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <regex.h>
#include <errno.h>

#include <string>
#include <vector>
using namespace std;

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

class NetAddr {
public:
	typedef enum {
		UNKNOWN_ADDR = 0x00,
		UNIX_DOMAIN_ADDR,
		TCP_ADDR,
		UDP_ADDR
	}AddrType;

	NetAddr() :addr(""), type(UNKNOWN_ADDR), sock_addr(NULL), sock_addr_len(0) {
	}
	NetAddr(const string &_addr) :addr(_addr), type(TCP_ADDR), sock_addr(NULL), sock_addr_len(0) {
		set(addr);
	}

	// 仅支持IPv4
	void set(const string &addr) {
		// tcp:host:port或者udp:host:port或者/tmp/mysql.sock
		if (addr[0] == '/') {
			type = UNIX_DOMAIN_ADDR;
		} else {
			vector<string> v_segs;	
			boost::split(v_segs, addr, boost::is_any_of(":"), boost::token_compress_on);
			if (v_segs.size() == 3) {
				if (strncasecmp("tcp", v_segs[0].c_str(), v_segs[0].size()) == 0) {
					type = TCP_ADDR;
				} else if (strncasecmp("udp", v_segs[0].c_str(), v_segs[0].size()) == 0) {
					type = UDP_ADDR;	
				} else {
					type = UNKNOWN_ADDR;	
				}
				//if (check_ip(v_segs[1]) == 0) {
				//	// 匹配ip
				//	host = v_segs[1];
				//	ip = v_segs[1];
				//} else {
				host = v_segs[1];
				char buf[64];
				int ret = 0;
				struct hostent host_info, *p_h = NULL;
				gethostbyname_r(host.c_str(), &host_info, buf, sizeof(buf), &p_h, &ret);
				inet_ntop(host_info.h_addrtype, host_info.h_addr_list[0], buf, sizeof(buf));
				ip = buf;
				//}
				host = host_info.h_name;
				port = strtoul(v_segs[2].c_str(), NULL, 10);
				fprintf(stdout, "type:%d, host:%s, ip:%s, port:%d\n", type, host.c_str(), ip.c_str(), port);
			}
		}
	}

	~NetAddr() {
	}

	int check_ip(const string& ip) {
		// 暂时匹配出问题....需要再研究
		// 正则表达式：^((\\d{1,2}|1\\d{2}|2[0-4]\\d|25[0-5])\\.){3}(\\d{1,2}|1\\d{2}|2[0-4]\\d|25[0-5])$
		const char* ipv4_pattern = "^((\\d{1,2}|1\\d{2}|2[0-4]\\d|25[0-5])\\.){3}(\\d{1,2}|1\\d{2}|2[0-4]\\d|25[0-5])$";
		regex_t reg;
		if (regcomp(&reg, ipv4_pattern, 0)) {
			return -1;
		}
		if (regexec(&reg, ip.c_str(), 0, NULL, REG_NOTBOL)) {
			regfree(&reg);
			return -1;
		}
		regfree(&reg);

		return 0;
	}

	int create_socket() {
		int fd = -1;
		switch (type) {
			case UNIX_DOMAIN_ADDR:
				fd = socket(AF_UNIX, SOCK_STREAM, 0);
			break;
			case TCP_ADDR:
				fd = socket(AF_INET, SOCK_STREAM, 0);
			break;
			case UDP_ADDR:
				fd = socket(AF_INET, SOCK_DGRAM, 0);
			break;
			default:
				return -1;
			break;
		}

		int opt = 1;
		int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&opt, sizeof(opt));
		if (ret < 0) {
			fprintf(stderr, "setsockopt failed\n");
		}
		ret = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (const void*)&opt, sizeof(opt));
		if (ret < 0) {
			fprintf(stderr, "setsockopt failed\n");
		}

		fcntl(fd, F_SETFL,O_NONBLOCK);
		return fd;
	}

	int create_server_socket() {
		int fd = create_socket();
		fprintf(stdout, "%d\n", type);
		switch (type) {
			case UNIX_DOMAIN_ADDR: {
				unlink(addr.c_str());
				struct sockaddr_un un;
				memset(&un, 0x00, sizeof(struct sockaddr_un));
				un.sun_family = AF_UNIX;
				int ret = snprintf(un.sun_path, sizeof(un.sun_path)-1, "%s", addr.c_str());
				if (ret < 0 || ret >= sizeof(un.sun_path)-1) {
					return -1;
				}
				un.sun_path[ret] = '\0';
				int len = offsetof(struct sockaddr_un, sun_path) + ret;
				if (bind(fd, (struct sockaddr*)&un, len) < 0) {
					fprintf(stderr, "it can not bind fd %d{%s}\n", fd, strerror(errno));
					return -1;
				}
			}
			break;
			case TCP_ADDR: {
				struct sockaddr_in sock_addr;
				sock_addr.sin_family = AF_INET;
				sock_addr.sin_port = htons(port);
				inet_pton(AF_INET, ip.c_str(), &sock_addr.sin_addr.s_addr);
				if (bind(fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) < 0) {
					fprintf(stderr, "it can not bind fd %d{%s}\n", fd, strerror(errno));
					return -1;
				}
			}
			break;
			case UDP_ADDR: {
				struct sockaddr_in sock_addr;
				sock_addr.sin_family = AF_INET;
				sock_addr.sin_port = htons(port);
				inet_pton(AF_INET, ip.c_str(), &sock_addr.sin_addr.s_addr);
				if (bind(fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) < 0) {
					fprintf(stderr, "it can not bind fd %d{%s}\n", fd, strerror(errno));
					return -1;
				}
			
			}
			break;
			default:
				fprintf(stderr, "unknow address type\n");
				return -1;
			break;
		}

		if (listen(fd, 10) < 0) {
			fprintf(stderr, "it can not listen on fd %d{%s}\n", fd, strerror(errno));
			return -1;
		}

		return fd;
	}

public:
	string 		addr;
	string		host;
	string		ip;
	int			port;
	AddrType 	type;
	sockaddr*	sock_addr;
	int			sock_addr_len;
};

#endif // __NET_ADDR_H__
