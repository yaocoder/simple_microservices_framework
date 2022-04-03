#ifndef MESSAGE_H
#define MESSAGE_H

#define TOKEN_LENGTH			5
#define TOKEN_STR				"*****"


/* JK(JSON KEY) */
#define JK_CLINET_SFD			"cfd"
#define JK_CLINET_SFD_ID		"cfdid"
/* 公共部分 */

#define JK_MESSAGE_ID			"mid"			//标识这次网络访问的唯一id
#define JK_MESSAGE_TYPE 		"mt"			//标识具体的操作类型
#define JK_PROTO_VERSION		"pv"			//协议版本号，现在均为1.0
#define JK_LOGIC_PROCESS_TYPE	"lpt"			//后台服务分发标识
#define JK_RESULT    			"rt"			//服务器回复的结果码
#define JK_SERVER_MESSAGE		"msg"			//服务器返回的错误信息

enum TcpConnectFlag
{
	SHORT_CONNECTION,
	PERSIST_CONNECTION,
};

enum DistributedProcessType
{
	REPORT_PROCESS_NAME = 101,
	REPORT_PROCESS_NAME_RESPONSE = 102
};

/* 分布式进行标识 */
#define JK_PROCESS_MESSAGE_TYPE	"proMsgType"
#define JK_RECEIVE_PROCESS_NAME	"receivSysName"
#define JK_SEND_PROCESS_NAME	"sendSysName"
#define JK_DISPACH_PROCESS		"dispachSys"

#endif
