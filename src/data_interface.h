/*
 * file name: data_interface.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2016-03-25
 * history  :
 */
#ifndef __DATA_INTERFACE_H__
#define __DATA_INTERFACE_H__

#include "http_meta.h"

struct DataBlock {
	int size; // 当要写数据时，表示能写的大小；当读数据时，表示有效数据的大小
};
#define DATABLK_ADDR(p) ((char*)p+sizeof(DataBlock))

struct BufInfo {
	unsigned long long addr;
	int size;
};

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

// 定义分层
typedef enum {
	LISTENER_LAYER = 0x00,	
	SESSION_LAYER,	
	TRANSACTION_LAYER,	
	TRANSACTION_CONTROLLER_LAYER,	
	APP_LAYER,
	LAYER_NUM
}LayerType;

// 定义通道输入输出类型
typedef enum {
	CHANNEL_IN = 0x00,
	CHANNEL_OUT,
	CHANNELIO_NUM
}ChannelIO;

typedef char TransId[32];

// 消息结构
// TRANSACTION --> APP
typedef struct {
	TransId 			id; // 事务ID
	unsigned long long 	http_meta_addr;
	BufInfo 			buf_info;
}TransApp;

// APP --> TRANSACTION Controller
typedef struct {
	TransId	id;
}AppTc;

// SESSION --> TRANSACTION
typedef struct {
	DataSrc ds;
	DataMsg dm;
}SessTransHeader;

typedef struct {
	SessTransHeader h;
	HttpReqMeta 	http_meta;
	int 			httpdata_len;
}SessTrans;

typedef enum {
	TRANS_EXEC_PREPARE = 0x00,
	TRANS_EXEC_TIMEOUT,
	TRANS_EXEC_OK
}TransEvent;

// TRANSACTION CONTROLLER --> TRANSACTION 
typedef struct {
	TransId		id;
	TransEvent 	ev;
}TcTrans;

// TRANSACTION --> TRANSACTIOn CONTROLLER
typedef struct {
	TransId		id;
	TransEvent 	ev;
}TransTc;

#endif // __DATA_INTERFACE_H__
