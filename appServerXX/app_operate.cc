/*
 * app_operate.cc
 *
 *  Created on: Mar 11, 2013
 *      Author: yaowei
 */

#include "app_operate.h"
#include "global_settings.h"
#include "../public/utils.h"
#include "../public/message.h"
#include "../public/socket_wrapper.h"
#define SALT_SIZE 8

extern struct ClientInfo g_clientInfo;


CAppOperate::CAppOperate()
{

}

CAppOperate::~CAppOperate()
{

}

void CAppOperate::StartAppOpt(const std::string& message)
{
	/** 恢复客户端进行的转义处理 */
	std::string string = utils::ReplaceString(message, "\\\\r\\\\n", "\\r\\n");

	/* 根据不同的协议解析类型进行不同的业务处理 */
	
	/* 处理结果返回分发进程  */
	SendToGateway();
}


void CAppOperate::SendToGateway()
{
	responseToGateway_.append(CRLF);
	utils::Unicode2UTF8(responseToGateway_);
	if(responseToGateway_.length() >= DATA_BUFFER_SIZE)
	{
		LOG4CXX_WARN(g_logger, "CAppOperate::SendToGateway data too large. data = " << responseToGateway_);
		return;
	}
	if (!SocketOperate::WriteSfd(g_clientInfo.sfd, responseToGateway_.c_str(), responseToGateway_.length()))
	{
		LOG4CXX_WARN(g_logger, "CAppOperate::SendToGateway:sendto failed.");
	}
	LOG4CXX_TRACE(g_logger, "CAppOperate::SendToGateway:sendto = " << responseToGateway_);
}





