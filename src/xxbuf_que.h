/*
 * file name: xxbuf_que.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2016-03-25
 * history  :
 *   20160325: 创建内存块的循环队列，以数组作为底层容器，xx表示从x到x，完成了一个循环；当前支持一对一的Producer/Consumer模型
 *             block_size是2的倍数
 */
#ifndef __XXBUF_QUE_H__
#define __XXBUF_QUE_H__

struct DataBlock {
	int size; // 当要写数据时，表示能写的大小；当读数据时，表示有效数据的大小
};
#define DATABLK_ADDR(p) ((char*)p+sizeof(DataBlock))

class XxbufQue {
public:
	XxbufQue(int _block_size, int _elem_num): block_size(_block_size),elem_num(_elem_num),
		head(0),rear(0) {
		status = create();	
	}

	~XxbufQue() {
		if (p != NULL) {
			free(p);
		}
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
		block->size = block_size - sizeof(DataBlock);

		return block;
	}

	/* 由于是循环buffer，只能在当前rear节点上增加 */
	int add_data_block(DataBlock* block) {
		int next = (rear + 1) % elem_num;
		if (next == head) {
			return -1;
		}
		rear = (rear + 1) % elem_num;

		return 0;	
	}

	DataBlock* get_data_block() {
		if (head == rear) {
			return NULL;
		}

		DataBlock* block = (DataBlock*)(head*block_size+p);

		return block;
	}

	/* 需要配合get_data_block使用，即将数据处理完后，将数据块放入队列中，变成可用内存块 */
	int lease_data_block() {
		if (head != rear) {
			head = (head + 1) % elem_num;
		}

		return 0;
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
};

#endif // __XXBUF_QUE_H__
