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
	fprintf(stdout, "%s %d\n", __FUNCTION__, __LINE__);

	while (true) {
		// 获取来自SESSION的数据
		XxbufQue* in_que_sess = worker->get_inque(SESSION_LAYER);
		if (NULL == in_que_sess) {
			return NULL;
		}
		DataBlock* block = in_que_sess->get_data_block();
		if (NULL != block) {
			// 生成事务，并将数据拷贝
			char* d = DATABLK_ADDR(block);
			SessTrans* st = (SessTrans*)(d);
			DataSrc* ds = &st->h.ds;
			DataMsg* dm = &st->h.dm;
			// 事务ID生成算法：以socket、时间戳作为元数据，MD5的结果即为事务ID
			int d_len = sizeof(int)+sizeof(struct timeval);
			char key_d[sizeof(int)+sizeof(struct timeval)];
			*(int*)key_d = ds->fd;
			gettimeofday((struct timeval*)(key_d+sizeof(int)), NULL);
			TransId id;
			MD5((const unsigned char*)key_d, d_len, (unsigned char*)id);
			fprintf(stdout, "%s %d\n", __FUNCTION__, __LINE__);
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
			
			fprintf(stdout, "%s %d\n", __FUNCTION__, __LINE__);
			DataBlock* t_block = worker->pool.get();	
			fprintf(stdout, "%s %d\n", __FUNCTION__, __LINE__);
			if (NULL == t_block) {
				in_que_sess->lease_data_block();
				fprintf(stdout, "%s %d\n", __FUNCTION__, __LINE__);
				continue;
			}
			fprintf(stdout, "%s %d\n", __FUNCTION__, __LINE__);
			worker->trans.insert(pair<string,DataBlock*>(string(id,sizeof(id)),t_block));
			fprintf(stdout, "%s %d\n", __FUNCTION__, __LINE__);
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
			if (worker->pre_processor.f) {
				pre_processor_fun f = worker->pre_processor.f;
				BufInfo* rsp_buf = (BufInfo*)(http_data+st->httpdata_len);
				rsp_buf->addr = (unsigned long long)(char*)(rsp_buf+sizeof(BufInfo));
				rsp_buf->size = 1024;
				f(&st->http_meta, http_data, *httpdata_len, *rsp_buf, worker->pre_processor.ctx);
			}
			in_que_sess->lease_data_block();
		}

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
		usleep(1);
	}

	return arg;
}
