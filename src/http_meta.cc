#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "http_meta.h"

void http_req_meta_print(HttpReqMeta& req_meta) {
	fprintf(stdout, "\n");
	fprintf(stdout, "method = [%.*s]\n", req_meta.method.length, req_meta.http_start + req_meta.method.offset);
	fprintf(stdout, "version = [%.*s]\n", req_meta.version.length, req_meta.http_start + req_meta.version.offset);
	fprintf(stdout, "uri = [%.*s]\n", req_meta.uri.length, req_meta.http_start + req_meta.uri.offset);
	fprintf(stdout, "user-agent = [%.*s]\n", req_meta.user_agent.length, req_meta.http_start + req_meta.user_agent.offset);
	fprintf(stdout, "cookie = [%.*s]\n", req_meta.cookie.length, req_meta.http_start + req_meta.cookie.offset);
	fprintf(stdout, "content_length = [%.*s]\n", req_meta.content_length.length, req_meta.http_start + req_meta.content_length.offset);
	fprintf(stdout, "connection = [%.*s]\n", req_meta.connection.length, req_meta.http_start + req_meta.connection.offset);
	fprintf(stdout, "\n");
}
