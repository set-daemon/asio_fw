/*
 * file name: http_meta.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2016-03-27
 * history  :
 */
#ifndef __HTTP_META_H__
#define __HTTP_META_H__

// 当前系统支持的最大额外HTTP头个数，固定分配，可以根据实际业务调配
#define MAX_HEADER_SIZE 16

struct HttpSegData {
	int offset; // 起始位置 + http_start即可获取到数据
	int length; // 数据长度
};

// TODO 实现Http头特有的hash算法，检查已知HTTP头的差异点
struct AuxData {
	int hash_value; 	// hash值
	int pos;			// 数据所指向的位置
};

struct AuxHeaderHash {
	AuxData	aux_data[MAX_HEADER_SIZE];
};

/*
 * 请求头Meta
 */
struct HttpReqMeta {
	char* 			http_start; // http数据开始位置
	//
	HttpSegData 	method;
	HttpSegData 	uri;
	HttpSegData 	version;
	// 固定http头
	HttpSegData 	host;
	HttpSegData 	user_agent;
	HttpSegData 	cookie;
	HttpSegData 	connection;
	HttpSegData 	content_length;
	// http body
	HttpSegData		body;
	// 额外的http头
	AuxHeaderHash	header_hash;	// HTTP辅助头，用于查找该头的数据在headers_data中的位置
	HttpSegData 	headers[MAX_HEADER_SIZE]; 		// HTTP头数据
	HttpSegData 	headers_data[MAX_HEADER_SIZE]; 	// HTTP头数据
	int				header_num;
};

#endif // __HTTP_META_H__
