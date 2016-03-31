/*
 * file name: http_parser.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2016-03-27
 * history  :
 */
#ifndef __HTTP_PARSER_H__
#define __HTTP_PARSER_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "http_meta.h"

namespace parser {

typedef enum {
	CMD_LINE_STAGE = 0x00, // 请求命令行阶段
	HEADER_STAGE,			// http头解析阶段
	BODY_STAGE				// http数据阶段
}HTTP_STAGE;

typedef enum {
	METHOD_STEP = 0x00,	// 解析方法
	URI_STEP,			// REQ的URI解析
	VERSION_STEP,		// 版本	
	HEADER_STEP,		// 解析HTTP头名称
	HEADER_VALUE_STEP,	// 解析HTTP头数据
	BODY_STEP			// 解析HTTP负载数据
}PARSE_STEP;

// 描述PARSE_STEP
typedef enum {
	PARSE_WAIT = 0x00, 	// 待解析，未开始
	PARSE_START, 		// 开始解析
	PARSING,			// 解析中，等待未接受完的数据
	PARSE_COMPLETED		// 解析结束
}PARSE_STATUS;

struct HttpParserStatus {
	HTTP_STAGE 		stage;
	PARSE_STEP 		step;
	PARSE_STATUS 	status;
	int				offset;
	HttpSegData *   cur_http;	
};

#define HTTP_DATA_SIZE 4096
struct HttpDataBlock {
	HttpReqMeta	meta;
	int			size;
	int			len;
	char		data[HTTP_DATA_SIZE]; // 暂时做固定分配，下版优化时，将使用内存池
};

class HttpParseInfo {
public:
	HttpParseInfo() {
		status.stage = CMD_LINE_STAGE;
		status.step = METHOD_STEP;
		status.status = PARSE_WAIT;
		status.offset = 0; // 已解析的位置
		status.cur_http = NULL; // 当前的http头
		// 初始化http数据部分
		http_data.size = HTTP_DATA_SIZE;
		http_data.len = 0;
		memset(&http_data.meta, 0x00,sizeof(HttpReqMeta));
	}
	~HttpParseInfo() {
	}

public:
	HttpParserStatus status;
	HttpDataBlock http_data;
};

int http_req_parse(HttpParseInfo &info);

int http_rsp_parse(HttpParseInfo &info);

}

#endif // __HTTP_PARSER_H__
