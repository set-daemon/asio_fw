/*
 * file name: xxbuf_que.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2016-03-25
 * history  :
 *   20160325: 创建内存块的循环队列，以数组作为底层容器，xx表示从x到x，完成了一个循环；当前支持一对一的Producer/Consumer模型
 *             block_size是2的倍数
 *   20160326: 增加异步框架相关的数据块定义：增加socket字段，占用4个字节
 */
#ifndef __XXBUF_QUE_H__
#define __XXBUF_QUE_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include <semaphore.h>

#include "data_interface.h"

class XxbufQue {
public:
	XxbufQue(int _block_size, int _elem_num): block_size(_block_size),elem_num(_elem_num),
		head(0),rear(0) {
		status = create();	
		sem_init(&notify_sem, 0, 0);
	}

	~XxbufQue() {
		if (p != NULL) {
			free(p);
		}
		sem_destroy(&notify_sem);
	}

	int is_ok() {
		return status == 0;
	}

	DataBlock* get_free_block() {
		int next = (rear + 1) % elem_num;
		if (next == head) {
			return NULL;
		}

		DataBlock* block = (DataBlock*)(p + rear * block_size);
		//block->addr = (char*)block;
		block->size = block_size - sizeof(DataBlock) - sizeof(DataMsg);

		return block;
	}

	/* 由于是循环buffer，只能在当前rear节点上增加 */
	int add_data_block(DataBlock* block) {
		int next = (rear + 1) % elem_num;
		if (next == head) {
			return -1;
		}
		rear = (rear + 1) % elem_num;

		// 信号量
		int sem_ret = sem_post(&notify_sem);
		if (-1 == sem_ret) {
			fprintf(stderr, "sem_post %d, %s\n", errno, strerror(errno));
			return -2;
		}

		return 0;
	}

	DataBlock* get_data_block() {
		if (head == rear) {
			return NULL;
		}

		DataBlock* block = (DataBlock*)(head*block_size+p);

		return block;
	}

	DataBlock* wait(long long nsec) {
		if (0 == nsec) { 
		} else if (nsec < 0) {
			if (0 != sem_wait(&notify_sem)) {
				return NULL;
			}
		} else {
			struct timespec tw;
			//tw.tv_sec = 0;
			clock_gettime(CLOCK_REALTIME, &tw);
			tw.tv_nsec += 100000000;
			tw.tv_sec += 1;
			if (0 != sem_timedwait(&notify_sem, &tw)) {
				return NULL;
			}
		}

		return get_data_block();
	}

	/* 需要配合get_data_block使用，即将数据处理完后，将数据块放入队列中，变成可用内存块 */
	int lease_data_block() {
		if (head != rear) {
			head = (head + 1) % elem_num;
		}

		return 0;
	}

	int cfg_block_size() {
		return block_size;
	}

private:
	int create() {
		if (block_size <= 0 || elem_num <= 0) {
			fprintf(stdout,"XxbufQue create() %d %d\n", block_size, elem_num);
			return -1;
		}

		int total_size = block_size * elem_num;
		p = (char*)malloc(total_size);	
		if (NULL == p) {
			return -1;
		}
		memset(p, 0x00, total_size);

		return 0;
	}

private:
	int status;
	int head;
	int rear;

	int block_size;
	int elem_num;
	char *p;

	sem_t notify_sem;
};

#endif // __XXBUF_QUE_H__
