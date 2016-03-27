
#include "http_parser.h"

namespace parser {

#define PRINT_FILELINE fprintf(stdout, "%s,%d\n", __FUNCTION__, __LINE__);
#define PRINT_FILELINE

static int _http_cmdline_stage(HttpParseInfo &info) {
	char *p = info.http_data.data + info.status.offset;
	int len = info.http_data.len;
	int exit = 0;

	do {
		switch (info.status.step) {
		case METHOD_STEP:
			info.http_data.meta.http_start = info.http_data.data;
			info.http_data.meta.method.offset = 0;
			info.http_data.meta.method.length = 0;
			// 寻找空格
			while (info.status.offset < len && p[info.status.offset] != ' ') {
				++info.status.offset;
			}
			if (info.status.offset == len) {
				// 未找到，则继续保持状态
				exit = 1;
				PRINT_FILELINE
			} else {
				// 找到，判断是否是POST、GET、PUT、...
				info.http_data.meta.method.length = info.status.offset - info.http_data.meta.method.offset;
				++info.status.offset; // 挪至下一个数据
				info.status.step = URI_STEP;
				PRINT_FILELINE
			}
		break;
		case URI_STEP: {
			info.http_data.meta.uri.offset = info.status.offset;
			info.http_data.meta.uri.length = 0;
			while (info.status.offset < len && p[info.status.offset] != ' ') {
				++info.status.offset;
			}
			if (info.status.offset == len) {
				// 未找到，则继续保持状态
				PRINT_FILELINE
				exit = 1;
			} else {
				// 找到
				info.http_data.meta.uri.length = info.status.offset - info.http_data.meta.uri.offset;
				++info.status.offset;
				info.status.step = VERSION_STEP;
				PRINT_FILELINE
			}
		}
		break;
		case VERSION_STEP: {
			info.http_data.meta.version.offset = info.status.offset;
			info.http_data.meta.version.length = 0;
			while (info.status.offset < len && p[info.status.offset] != '\r') {
				++info.status.offset;
			}
			if (info.status.offset == len) {
				// 未找到，则继续保持状态
				PRINT_FILELINE
				exit = 1;
			} else {
				// 判断是否等于\n
				if (info.status.offset+1 < len && p[info.status.offset+1] == '\n') {
					info.http_data.meta.uri.length = info.status.offset - info.http_data.meta.version.offset;
					++info.status.offset;
					info.status.step = HEADER_STEP;
					info.status.stage = HEADER_STAGE;
					exit = 1;
					PRINT_FILELINE
				} else {
					// 未找到\n
					exit = 1;
					PRINT_FILELINE
				}
			}
		}
		break;
		}
	} while (!exit);

	return 0;
}

static int _http_headers_stage(HttpParseInfo &info) {
	int exit = 1;

	do {
		switch (info.status.step) {
		case HEADER_STEP:
		break;
		case HEADER_VALUE_STEP:
		break;
		}
	} while (!exit);
	return 0;
}

static int _http_body_stage(HttpParseInfo &info) {
	int exit = 1;

	// 如果是GET请求或者Content-Length为0,则不需要读取BODY

	do {
		switch (info.status.step) {
		case BODY_STEP:
		break;
		}
	} while (!exit);

	return 0;
}

int http_req_parse(HttpParseInfo &info) {
	do {
		switch (info.status.stage) {
		case CMD_LINE_STAGE:
			_http_cmdline_stage(info);
		break;
		case HEADER_STAGE:
			_http_headers_stage(info);
			info.status.status = PARSE_COMPLETED;
		break;
		case BODY_STAGE:
			_http_body_stage(info);
		break;
		}
	} while (info.status.status != PARSE_COMPLETED);

	return 0;
}

int http_rsp_parse(HttpParseInfo &info) {

	return 0;
}

}
