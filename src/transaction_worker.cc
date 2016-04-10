#include <unistd.h>
#include <sys/time.h>
#include <openssl/md5.h>
#include "transaction_worker.h"

int TransactionWorker::init(const string& cf) {
	return 0;
}

int TransactionWorker::run() {

	if (0 != pthread_create(&worker_pthread, NULL, TransactionWorker::worker_cb, this)) {
		return -1;
	}

	return 0;
}

void* TransactionWorker::worker_cb(void* arg) {
	TransactionWorker* worker = (TransactionWorker*)arg;
	//fprintf(stdout, "%s %d\n", __FUNCTION__, __LINE__);

	while (true) {
		// 获取来自SESSION的数据
		XxbufQue* in_que_sess = worker->get_inque(SESSION_LAYER);
		if (NULL == in_que_sess) {
			return NULL;
		}
		DataBlock* block = in_que_sess->wait(1000000000);
		if (NULL != block) {
			// 生成事务，并将数据拷贝
			char* d = DATABLK_ADDR(block);
			SessTrans* st = (SessTrans*)(d);
			DataSrc* ds = &st->h.ds;
			DataMsg* dm = &st->h.dm;
#if 1
			// 事务ID生成算法：以socket、时间戳作为元数据，MD5的结果即为事务ID
			int d_len = sizeof(int)+sizeof(struct timeval);
			char key_d[sizeof(int)+sizeof(struct timeval)];
			*(int*)key_d = ds->fd;
			gettimeofday((struct timeval*)(key_d+sizeof(int)), NULL);
			TransId id;
			MD5((const unsigned char*)key_d, d_len, (unsigned char*)id);
			//fprintf(stdout, "Transaction Worker %s %d\n", __FUNCTION__, __LINE__);
			const char* hex_chars = "0123456789ABCDEF";
			// 将数据串转为十六进制字符数据
			for (int i = 15; i >= 0; --i) {
				char hc = 0xFF & (id[i]>>4);
				char lc = id[i] & 0xFF;
				int pos_h = i*2;
				int pos_l = pos_h + 1;
				id[pos_h] = *(hex_chars+hc);
				id[pos_l] = *(hex_chars+lc);
			}

			//fprintf(stdout, "TransactionWorker %s %d\n", __FUNCTION__, __LINE__);
			DataBlock* t_block = worker->pool.get();	
			//fprintf(stdout, "%s %d\n", __FUNCTION__, __LINE__);
			if (NULL == t_block) {
				in_que_sess->lease_data_block();
				//fprintf(stdout, "TransactionWorker %s %d\n", __FUNCTION__, __LINE__);
				continue;
			}
			//fprintf(stdout, "TransactionWorker %s %d\n", __FUNCTION__, __LINE__);
			worker->trans.insert(pair<string,DataBlock*>(string(id,sizeof(id)),t_block));
			//fprintf(stdout, "TransactionWorker %s %d\n", __FUNCTION__, __LINE__);
			char* buf_addr = DATABLK_ADDR(t_block);
			int buf_size = t_block->size;
			DataSrc *t_ds = (DataSrc*)buf_addr;
			memcpy(t_ds, ds, sizeof(DataSrc));
			HttpReqMeta* http_reqmeta = (HttpReqMeta*)(buf_addr + sizeof(DataSrc));
			memcpy(http_reqmeta, &st->http_meta, sizeof(HttpReqMeta));
			int *httpdata_len = (int*)(buf_addr+sizeof(DataSrc)+sizeof(HttpReqMeta));
			*httpdata_len = st->httpdata_len;
			char* http_data = buf_addr+sizeof(DataSrc)+sizeof(HttpReqMeta) + sizeof(int);
			memcpy(http_data, d+sizeof(SessTrans), st->httpdata_len);
			http_reqmeta->http_start = http_data;
			int ret = 0;
			BufInfo* rsp_buf = NULL;
			BufInfo* app_buf = NULL;
			if (worker->pre_processor.f) {
				pre_processor_fun f = worker->pre_processor.f;
				rsp_buf = (BufInfo*)(http_data+st->httpdata_len);
				rsp_buf->addr = (unsigned long long)((char*)(rsp_buf)+sizeof(BufInfo));
				rsp_buf->size = 1024;
				ret = f(&st->http_meta, http_data, *httpdata_len, *rsp_buf, worker->pre_processor.ctx);
				BufInfo* app_buf = (BufInfo*)((char*)rsp_buf + sizeof(BufInfo) + rsp_buf->size);	
				app_buf->addr = (unsigned long long)((char*)app_buf + sizeof(BufInfo));
				app_buf->size = buf_size - sizeof(DataSrc) - sizeof(HttpReqMeta) - sizeof(int) - *httpdata_len - sizeof(BufInfo) - rsp_buf->size - sizeof(BufInfo);
			} else {
				app_buf = (BufInfo*)(http_data+st->httpdata_len);
				app_buf->addr = (unsigned long long)((char*)(rsp_buf)+sizeof(BufInfo));
				app_buf->size = buf_size - sizeof(DataSrc) - sizeof(HttpReqMeta) - sizeof(int) - *httpdata_len - sizeof(BufInfo);
			}
			in_que_sess->lease_data_block();
			if (ret == -1) {
				// 如果预处理器返回-1,则立即返回数据，并删除该事务
				map<string,DataBlock*>::iterator t_find_iter = worker->trans.find(string(id));
				if (t_find_iter != worker->trans.end()) {	
					worker->trans.erase(t_find_iter);
				}
				int w_ret = write(ds->fd, (char*)rsp_buf->addr, rsp_buf->size);
				if (w_ret < 0) {
					fprintf(stdout, "事务返回失败 %d,%s\n", errno, strerror(errno));
				} else {
					fprintf(stdout, "事务返回成功 %d,%s\n", errno, strerror(errno));
				}
				worker->pool.giveback(t_block);
			}
			// TODO 将事务发送给TC

			// TODO 将事务发送给APP
#else
			char buf[1024];	
			static const char *rsp_body = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"></meta><title>测试</title></head><body></body></html>";
			static const char *test_rsp = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Length: %d\r\n\r\n%s";
			in_que_sess->lease_data_block();
			int n = sprintf(buf, test_rsp, strlen(rsp_body), rsp_body);
			buf[n] = '\0';
			int w_ret = write(ds->fd, (char*)buf, n);
			if (w_ret < 0) {
				fprintf(stdout, "事务返回失败 %d,%s\n", errno, strerror(errno));
			}
#endif
		} else {
			fprintf(stdout, "TransactionWorker no data\n");
		}

#if 0
		// 获取来自TC的数据
		XxbufQue* in_que_tc = worker->get_inque(TRANSACTION_CONTROLLER_LAYER);
		if (NULL == in_que_tc) {
			return NULL;
		}
		block = in_que_tc->get_data_block();
		if (NULL != block) {
			// 处理数据
			in_que_tc->lease_data_block();
		}
#endif
		//usleep(1);
	}

	return arg;
}
