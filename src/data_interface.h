/*
 * file name: data_interface.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2016-03-25
 * history  :
 */
#ifndef __DATA_INTERFACE_H__
#define __DATA_INTERFACE_H__

struct DataBlock {
	int size; // 当要写数据时，表示能写的大小；当读数据时，表示有效数据的大小
};
#define DATABLK_ADDR(p) ((char*)p+sizeof(DataBlock))

// 定义数据来源
struct DataSrc {
	int 	fd;
	char 	ip[16];
	short 	port;
};

// 定义数据消息
typedef enum {
	UPLOAD_DATA = 0x00,
	CHANNEL_LEASE
}DATA_OP;
struct DataMsg {
	DATA_OP op;
};

#endif // __DATA_INTERFACE_H__
