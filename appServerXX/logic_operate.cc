/*
 * logic_operate.cc
 *
 *  Created on: Mar 11, 2013
 *      Author: yaowei
 */

#include "logic_operate.h"
#include "global_settings.h"
#include "../public/utils.h"
#include "../public/message.h"
#include "../public/redis_key.h"
#include "../public/socket_wrapper.h"
#define SALT_SIZE 8

extern struct ClientInfo g_clientInfo;


CLogicOperate::CLogicOperate()
{

}

CLogicOperate::~CLogicOperate()
{

}

void CLogicOperate::StartLogicOpt(const std::string& message)
{
	/** 恢复客户端进行的转义处理 */
	std::string string = utils::ReplaceString(message, "\\\\r\\\\n", "\\r\\n");

	/* 根据不同的协议解析类型进行不同的业务处理 */
	
	/* 处理结果返回分发进程  */
	SendToDispatch();
}


void CLogicOperate::SendToDispatch()
{
	responseToDispatch_.append(CRLF);
	utils::Unicode2UTF8(responseToDispatch_);
	if(responseToDispatch_.length() >= DATA_BUFFER_SIZE)
	{
		LOG4CXX_WARN(g_logger, "CLogicOperate::SendToDispatch data too large. data = " << responseToDispatch_);
		return;
	}
	if (!SocketOperate::WriteSfd(g_clientInfo.sfd, responseToDispatch_.c_str(), responseToDispatch_.length()))
	{
		LOG4CXX_WARN(g_logger, "CLogicOperate::SendToDispatch:sendto failed.");
	}
	LOG4CXX_TRACE(g_logger, "CLogicOperate::SendToDispatch:sendto = " << responseToDispatch_);
}





