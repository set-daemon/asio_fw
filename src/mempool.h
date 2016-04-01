/*
 * file name: mempool.h
 * purpose  :
 * author   :
 * date     : 2016-04-01
 * history  :
 */
#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

#include <list>
using namespace std;

#include "data_interface.h"

class MemPool {
public:
	MemPool(int _block_size, int _block_num) : block_size(_block_size), block_num(_block_num) {
	}
	~MemPool() {
	}

	int init() {
		p = (char*)malloc(block_size*block_num);
		for (int i = 0; i < block_num; ++i) {
			char *tmp = p + i*block_size;
			free_list.push_back(tmp);
		}
	}

	DataBlock* get() {
		DataBlock *block = (DataBlock*)free_list.front();
		if (block == NULL) {
			return NULL;
		}
		block->size = block_size - sizeof(DataBlock);
		free_list.pop_front();

		return block;
	}

	int giveback(DataBlock* block) {
		free_list.push_back((char*)block);
	}

private:
	int 		block_size;
	int 		block_num;
	char 		*p;
	list<char*> free_list;	
};

#endif // __MEMPOOL_H__
