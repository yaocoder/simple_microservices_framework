/*
 * local_transport.cc
 *
 *  Created on: Mar 7, 2013
 *      Author: yaowei
 */

#include "local_transport.h"
#include "json_opt.h"
#include "master_thread.h"
#include "global_settings.h"
#include "../public/utils.h"
#include "../public/message.h"
#include "../public/socket_wrapper.h"

CLocalTransport* 	CLocalTransport::local_transport_ptr_ = NULL;
std::map<std::string, int> 	CLocalTransport::map_logic_sfd_;
boost::mutex 		CLocalTransport::mutex_;


CLocalTransport::CLocalTransport()
{
	sfd_ = 0;
	main_base_ = NULL;
	local_listen_event_ = NULL;
	local_listen_socket_ = 0;
}

CLocalTransport* CLocalTransport::GetInstance()
{
	if (NULL == local_transport_ptr_)
	{
		local_transport_ptr_ = new CLocalTransport;
	}

	return local_transport_ptr_;
}

void CLocalTransport::SetupLocalTransport()
{
	main_base_ = event_base_new();
	assert(main_base_ != NULL);

	/* 监听来自逻辑处理进程的长连接 */
	if (!InitLocalListenSocket(local_listen_socket_))
		exit(1);

	evutil_make_socket_nonblocking(local_listen_socket_);

	local_listen_event_ = event_new(main_base_, local_listen_socket_, EV_READ | EV_PERSIST, AcceptCb, (void*) this);
	assert(local_listen_event_ != NULL);

	if (event_add(local_listen_event_, NULL) == -1)
	{
		int error_code = EVUTIL_SOCKET_ERROR();
		LOG4CXX_FATAL(g_logger, "CLocalTransport::SetupLocalTransport:event_add errorCode = " << error_code
				<< ", description = " << evutil_socket_error_to_string(error_code));
		exit(1);
	}

	CreateThreadForListenLocal(ReadLibevent, (void*) this);

	LOG4CXX_INFO(g_logger, "CLocalTransport::SetupLocalTransport:localTransport has setup...");
}

void CLocalTransport::AcceptCb(evutil_socket_t listen_socket, short event, void* arg)
{
	CLocalTransport *pThis = static_cast<CLocalTransport*> (arg);

	evutil_socket_t sfd;
	struct sockaddr_in sin;
	socklen_t slen = sizeof(sin);

	sfd = accept(listen_socket, (struct sockaddr *) &sin, &slen);
	if (-1 == sfd)
	{
		LOG4CXX_WARN(g_logger, "CLocalTransport::AcceptCb:accept error = " << strerror(errno));
		return;
	}

	if (!SocketOperate::SetSocketNoBlock(sfd))
	{
		LOG4CXX_WARN(g_logger, "CNetCore::AcceptCb:SetSocketNoBlock error = " << strerror(errno))
		close(sfd);
		return;
	}

	LOCAL_REV_DATA* ptr_recv_data = new LOCAL_REV_DATA;
	bzero(ptr_recv_data->buf, DATA_BUFFER_SIZE);
	ptr_recv_data->len = 0;
	ptr_recv_data->sfd = sfd;

	struct bufferevent *logic_pro_bev = bufferevent_socket_new(pThis->main_base_, sfd, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(logic_pro_bev, ReadCb, NULL, ErrorCb, (void*) ptr_recv_data);
	bufferevent_enable(logic_pro_bev, EV_READ | EV_PERSIST);
}

void CLocalTransport::ReadCb(struct bufferevent *bev, void *arg)
{
	LOCAL_REV_DATA* ptr_data = static_cast<LOCAL_REV_DATA*> (arg);

	evutil_socket_t sfd = bufferevent_getfd(bev);

	int recv_size = 0;

	if ((recv_size = bufferevent_read(bev, ptr_data->buf + ptr_data->len, DATA_BUFFER_SIZE - ptr_data->len)) > 0)
	{
		ptr_data->len = ptr_data->len + recv_size;
	}

	std::string str_recv(ptr_data->buf, ptr_data->len);
	if (utils::FindCRLF(str_recv))
	{
		/* 有可能同时收到多条信息 */
		std::vector<std::string> vec_str;
		utils::SplitData(str_recv, CRLF, vec_str);

		for (unsigned int i = 0; i < vec_str.size(); ++i)
		{
			SendToClient(vec_str.at(i).c_str(), sfd);
		}

		int len = str_recv.find_last_of(CRLF) + 1;
		memmove(ptr_data->buf, ptr_data->buf + len, DATA_BUFFER_SIZE - len);
		ptr_data->len = ptr_data->len - len;
	}
}

void CLocalTransport::SendToClient(const char* buf, const int local_sfd)
{
	CJsonOpt jsonOpt;
	std::string string_to_client, sendSys_name, receiveSys_name;
	int proMsg_type;
	int sfd = 0;
	int sfd_id = 0;
	if (jsonOpt.RestructJsonStringToclient(buf, string_to_client, sfd, sfd_id))
	{
		string_to_client.append(CRLF);

		LOG4CXX_TRACE(g_logger, "CLocalTransport::SentTOClient =  " << string_to_client);

		/* 用得到的客户端连接的sfd和相对应的sfd_id同map映射做比对，如果没找到sfd，或者找到的sfd对应的sfd_id不匹配说明原有链接已经失效 */
		int find_sfd_id = 0;
		if(!CMasterThread::map_csfd_id_.find(sfd, find_sfd_id))
		{
			LOG4CXX_WARN(g_logger, "CLocalTransport::SendToClient sfd invalid. sfd = " << sfd << ", message = " << string_to_client);
			return;
		}

		if (sfd_id == find_sfd_id)
		{
			if (!SocketOperate::WriteSfd(sfd, string_to_client.c_str(), string_to_client.length()))
			{
				LOG4CXX_ERROR(g_logger, "CLocalTransport::SendToClient:WriteSfd  errorCode = " << strerror(errno)
										<< ", message = " << string_to_client);
				return;
			}
		}
		else
		{
			LOG4CXX_WARN(g_logger, "CLocalTransport::SendToClient sfd_id != find_sfd_id. sfd = " << sfd << ", sfd_id ="
					               << sfd_id << ", find_sfd_id = " << find_sfd_id
					               << ", message = " << string_to_client);
		}
	}
	else if (jsonOpt.JsonParseDistributedMessage(buf, proMsg_type, sendSys_name, receiveSys_name))
	{
		if(REPORT_PROCESS_NAME == proMsg_type && (!receiveSys_name.compare(JK_DISPACH_PROCESS)))
		{
			CLocalTransport::map_logic_sfd_.insert(std::pair<std::string, int>(sendSys_name, local_sfd));
			LOG4CXX_INFO(g_logger, "CLocalTransport::SendToClient " << sendSys_name << " connect SUCESSS!! local_sfd = " << local_sfd << ", message = " << buf);
		}
		else
		{
			LOG4CXX_WARN(g_logger, "CLocalTransport::SendToClient sfd invalid. local_sfd = " << local_sfd << ", message = " << buf);
			return;
		}
	}
	else
	{
		LOG4CXX_WARN(g_logger, "CLocalTransport::SendToClient message invalid , message = " << buf);
		return;
	}

}

void CLocalTransport::ErrorCb(struct bufferevent *bev, short event, void *arg)
{
	LOCAL_REV_DATA* ptr_data = static_cast<LOCAL_REV_DATA*> (arg);

	utils::SafeDelete(ptr_data);

	evutil_socket_t sfd = bufferevent_getfd(bev);

	bufferevent_free(bev);

	if (event & BEV_EVENT_TIMEOUT)
	{
		LOG4CXX_WARN(g_logger, "CLocalTransport::ErrorCb:TimeOut.");
	}
	else if (event & BEV_EVENT_EOF)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::ErrorCb:connection closed. sfd = " << sfd);
	}
	else if (event & BEV_EVENT_ERROR)
	{
		int error_code = EVUTIL_SOCKET_ERROR();
		LOG4CXX_ERROR(g_logger, "CLocalTransport::ErrorCb:some other errorCode = " << error_code
				<< ", description = " << evutil_socket_error_to_string(error_code));
	}

	/* 清除map记录*/
	std::map<std::string, int>::iterator iter;
	for (iter = map_logic_sfd_.begin(); iter != map_logic_sfd_.end(); ++iter)
	{
		if (sfd == iter->second)
			map_logic_sfd_.erase(iter);
	}
}

void CLocalTransport::CreateThreadForListenLocal(void *(*func)(void *), void *arg)
{
	pthread_t thread;
	pthread_attr_t attr;
	int ret;

	pthread_attr_init(&attr);

	if ((ret = pthread_create(&thread, &attr, func, arg)) != 0)
	{
		LOG4CXX_FATAL(g_logger, "CLocalTransport::CreateThreadForListenLocal:Can't create thread:" << strerror(ret));
		exit(1);
	}
}

void *CLocalTransport::ReadLibevent(void *arg)
{
	CLocalTransport *pThis = static_cast<CLocalTransport*> (arg);

	LOG4CXX_INFO(g_logger, "CLocalTransport::Run():localTransport has start...");

	int ret = event_base_dispatch(pThis->main_base_);
	if (-1 == ret)
	{
		int error_code = EVUTIL_SOCKET_ERROR();
		LOG4CXX_FATAL(g_logger, "CLocalTransport::Run():event_base_dispatch errorCode = " << error_code
				<< ", description = " << evutil_socket_error_to_string(error_code));
	}
	else if (1 == ret)
	{
		LOG4CXX_FATAL(g_logger, "CLocalTransport::Run():no events were registered.");
	}

	event_base_free(pThis->main_base_);
	exit(EXIT_FAILURE);
}

bool CLocalTransport::InitLocalListenSocket(evutil_socket_t& listen_socket)
{

	listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_socket < 0)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::InitLocalListenSocket:socket error = " << strerror(errno));
		return false;
	}

	int flags = 1;
	if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (void *) &flags, sizeof(flags)) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::InitLocalListenSocket:setsockopt SO_REUSEADDR error = " << strerror(errno));
		close(listen_socket);
		return false;
	}

	sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(utils::G<CGlobalSettings>().local_listen_port_);

	if (bind(listen_socket, (const sockaddr*) &servaddr, sizeof(servaddr)) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::InitLocalListenSocket:bind error = " << strerror(errno));
		close(listen_socket);
		return false;
	}

	if (listen(listen_socket, BACKLOG) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::InitLocalListenSocket:Listen error = " << strerror(errno));
		close(listen_socket);
		return false;
	}

	return true;
}

bool CLocalTransport::SendToLogicModule(const char* message, const std::string& module_type, const conn* c)
{
	if (strlen(message) >= DATA_BUFFER_SIZE)
	{
		LOG4CXX_WARN(g_logger, "CLocalTransport::SendToLogicModule data too large. data = " << message);
		return false;
	}

	boost::mutex::scoped_lock Lock(mutex_);
	std::map<std::string, int>::iterator iter;
	iter = CLocalTransport::map_logic_sfd_.find(module_type);
	if (iter == CLocalTransport::map_logic_sfd_.end())
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::SendToLogicModule map_logic_sfd invalid, message = " << message);
		return false;
	}
	int sfd = iter->second;

	if (!SocketOperate::WriteSfd(sfd, message, strlen(message)))
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::SendToLogicModule:WriteSfd  errorCode = " << strerror(errno)
					  	  	    << ", message =" << message);
		return false;
	}

	LOG4CXX_TRACE(g_logger, "ip = " << c->ip << " , CLocalTransport::SendToLogicModule:send data = " << message);

	return true;
}

