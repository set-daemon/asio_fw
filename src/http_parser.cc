
#include "http_parser.h"

namespace parser {

//#define PRINT_FILELINE fprintf(stdout, "%s,%d\n", __FUNCTION__, __LINE__);
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
	int exit = 0;
	char *p = info.http_data.data;
	int len = info.http_data.len;

	HttpSegData* cur_http = NULL;

	do {
		switch (info.status.step) {
		case HEADER_STEP: {
			cur_http = NULL;
			int offset = info.status.offset;
			while (offset < len && p[offset] != ':') {
				PRINT_FILELINE;
				++offset;
			}
			if (offset == len) {
				// 未找到
				PRINT_FILELINE;
				exit = 1;
			} else {
				// 找到
				// 判断是否是以下Header：Host、User-Agent、Cookie、Connection、Content-Length
				switch (offset - info.status.offset) {
				case 4:
					if (memcmp(p+info.status.offset, "Host", 4) == 0) {
						cur_http = &info.http_data.meta.host;
					} else {
						info.http_data.meta.headers[info.http_data.meta.header_num].offset = info.status.offset;
						info.http_data.meta.headers[info.http_data.meta.header_num].length = offset - info.status.offset;
					}
				break;
				case 6:
					if (memcmp(p+info.status.offset, "Cookie", 6) == 0) {
						cur_http = &info.http_data.meta.cookie;
					} else {
						info.http_data.meta.headers[info.http_data.meta.header_num].offset = info.status.offset;
						info.http_data.meta.headers[info.http_data.meta.header_num].length = offset - info.status.offset;
					}
				break;
				case 9:
					if (memcmp(p+info.status.offset, "User-Agent", 9) == 0) {
						cur_http = &info.http_data.meta.user_agent;
					} else {
						info.http_data.meta.headers[info.http_data.meta.header_num].offset = info.status.offset;
						info.http_data.meta.headers[info.http_data.meta.header_num].length = offset - info.status.offset;
					}
				break;
				case 10:
					if (memcmp(p+info.status.offset, "Connection", 10) == 0) {
						cur_http = &info.http_data.meta.connection;
					} else {
						info.http_data.meta.headers[info.http_data.meta.header_num].offset = info.status.offset;
						info.http_data.meta.headers[info.http_data.meta.header_num].length = offset - info.status.offset;
					}
				break;
				case 14:
					if (memcmp(p+info.status.offset, "Content-Length", 14) == 0) {
						cur_http = &info.http_data.meta.content_length;
					} else {
						info.http_data.meta.headers[info.http_data.meta.header_num].offset = info.status.offset;
						info.http_data.meta.headers[info.http_data.meta.header_num].length = offset - info.status.offset;
					}
				break;
				default: {
					info.http_data.meta.headers[info.http_data.meta.header_num].offset = info.status.offset;
					info.http_data.meta.headers[info.http_data.meta.header_num].length = offset - info.status.offset;
				}
				break;
				}
				++offset; //跨过符号:
				// 去除空白符号
				while (offset < len && p[offset] == ' ') {
					PRINT_FILELINE;
					++offset;
				}
				PRINT_FILELINE;
				info.status.step = HEADER_VALUE_STEP;
			}
			info.status.offset = offset;
			//printf("offset=%d p[offset]=%c,p[offset+1]=%c\n", offset, p[offset],p[offset+1]);
		}
		break;
		case HEADER_VALUE_STEP: {
			int offset = info.status.offset;
			while (offset+1 < len && p[offset] != '\r' && p[offset+1] != '\n') {
				//PRINT_FILELINE;
				//printf(":offset = %d, len = %d, p[offset]=%c p[offset+1]=%c\n", offset, len, p[offset], p[offset+1]);
				++offset;
			}
			if (offset+1 >= len) {
				PRINT_FILELINE;
			} else {
				// 找到
				int tmp_offset = offset;
				tmp_offset -= 2; // 跳过\r\n符号
				// 去除数据右边的空白
				while (tmp_offset >= info.status.offset && p[tmp_offset] == ' ') {
					PRINT_FILELINE;
					--tmp_offset;
				}
				++tmp_offset;
				if (cur_http != NULL) {
					cur_http->offset = info.status.offset;
					cur_http->length = tmp_offset - info.status.offset;
					cur_http = NULL;
				} else {
					info.http_data.meta.headers_data[info.http_data.meta.header_num].offset = info.status.offset;
					info.http_data.meta.headers_data[info.http_data.meta.header_num].offset = tmp_offset - info.status.offset;
				}
				offset += 2;
				info.status.offset = offset; //offset指向\r，所以需要再挪一个位置
				info.status.step = HEADER_STEP;
				++info.http_data.meta.header_num;
				PRINT_FILELINE;
				// 检查是否到header头的最后
				//fprintf(stdout, "offset=%d p[offset]=%c p[offset+1]=%c\n", offset, p[offset], p[offset+1]);
				if (offset+1 < len && p[offset]=='\r' && p[offset+1]=='\n') {
					PRINT_FILELINE;
					exit = 1;
					info.status.offset = offset + 2;
				}
			}
		}
		break;
		}
	} while (!exit);

	PRINT_FILELINE;
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

	http_req_meta_print(info.http_data.meta);

	return 0;
}

int http_rsp_parse(HttpParseInfo &info) {

	return 0;
}

}
