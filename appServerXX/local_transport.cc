/*
 * local_transport.cpp
 *
 *  Created on: Mar 7, 2013
 *      Author: yaowei
 */

#include "local_transport.h"
#include "global_settings.h"
#include "json_opt.h"
#include "../public/utils.h"
#include "../public/socket_wrapper.h"
#include "../public/message.h"

struct ClientInfo g_clientInfo;

consumers* CLocalTransport::consumers_ = NULL;

CLocalTransport::CLocalTransport()
{
	main_base_ = NULL;
	recv_event_ = NULL;
	stats_listen_sfd_ =0;
	local_tranfer_sfd_ = 0;
	local_connect_flag_ = false;
	consumers_ = new consumers(utils::G<CGlobalSettings>().thread_num_, utils::G<CGlobalSettings>().queue_num_);
}

CLocalTransport::~CLocalTransport()
{

}

bool CLocalTransport::CheckLibeventVersion()
{
	const char* libevent_version = event_get_version();
	assert(libevent_version != NULL);

	LOG4CXX_TRACE(g_logger, "The libevent version is " << libevent_version);

	if (strncmp(libevent_version, "2", 1) == 0)
		return true;
	else
		return false;
}

bool CLocalTransport::SetupTransport()
{
	if (!CheckLibeventVersion())
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::SetupTransport The libevent version Require at  2.0.*");
		return false;
	}

	main_base_ = event_base_new();
	assert(main_base_ != NULL);

	/* 开启状态查看服务监听端口 */
	if(!InitStatsSocket(stats_listen_sfd_))
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::SetupTransport:InitStatsSocket faliled.");
		return false;
	}

	evutil_make_socket_nonblocking(stats_listen_sfd_);

	struct bufferevent *listen_bev = bufferevent_socket_new(main_base_, stats_listen_sfd_, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(listen_bev, AcceptCb, NULL, NULL, (void*) this);
	bufferevent_enable(listen_bev, EV_READ | EV_PERSIST);

	return true;
}

bool CLocalTransport::InitStatsSocket(evutil_socket_t& listen_socket)
{
	listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_socket < 0)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::InitStatsSocket:socket error = " << strerror(errno));
		return false;
	}

	int flags = 1;
	if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (void *) &flags, sizeof(flags)) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::InitStatsSocket:setsockopt SO_REUSEADDR error = " << strerror(errno));
		close(listen_socket);
		return false;
	}

	if (setsockopt(listen_socket, IPPROTO_TCP, TCP_NODELAY, (void *) &flags, sizeof(flags)) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::InitStatsSocket:setsockopt TCP_NODELAY error = " << strerror(errno));
		close(listen_socket);
		return false;
	}

	sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(utils::G<CGlobalSettings>().stats_listen_port_ );

	if (bind(listen_socket, (const sockaddr*)&servaddr, sizeof(servaddr)) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::InitStatsSocket:bind error = " << strerror(errno));
		close(listen_socket);
		return false;
	}

	if(listen(listen_socket, BACKLOG) != 0)
	{
	    LOG4CXX_ERROR(g_logger, "CLocalTransport::InitStatsSocket:Listen error = " << strerror(errno));
	    close(listen_socket);
	    return false;
	}

	return true;
}

void CLocalTransport::AcceptCb(struct bufferevent *bev, void *arg)
{
	CLocalTransport* pThis = static_cast<CLocalTransport*>(arg);

	evutil_socket_t sfd;
	struct sockaddr_in sin;
	socklen_t slen = sizeof(sin);

	sfd = accept(pThis->stats_listen_sfd_, (struct sockaddr *) &sin, &slen);
	if (-1 == sfd)
	{
		LOG4CXX_WARN(g_logger, "CLocalTransport::accept error = " << strerror(errno));
		return;
	}

	if (!SocketOperate::SetSocketNoBlock(sfd))
	{
		LOG4CXX_WARN(g_logger, "CLocalTransport::AcceptCb:SetSocketNoBlock error = " << strerror(errno))
		close(sfd);
		return;
	}

	/* 利用libevent处理新的socket */
	//TODO:监控管理进程服务

}

void CLocalTransport::CreateWorker(void *(*func)(void *), void *arg)
{
	pthread_t thread;
	pthread_attr_t attr;
	int ret;

	pthread_attr_init(&attr);

	if ((ret = pthread_create(&thread, &attr, func, arg)) != 0)
	{
		LOG4CXX_FATAL(g_logger, "CLocalTransport::CreateWorker:Can't create thread:" << strerror(ret));
		exit(1);
	}
}


void *CLocalTransport::SetupLocalTransport(void *arg)
{
	CLocalTransport* pThis = static_cast<CLocalTransport*>(arg);

	/* 创建生产者、消费者队列模型（开启线程，防止阻塞应用），将协议逻辑处理放入生产者队列，进行消费处理 */
	pThis->consumers_->runAllConsumers();

	/* 当和逻辑分发进程的连接未正常建立时定时进行重试 */
	while (!pThis->local_connect_flag_)
	{
		boost::asio::deadline_timer t(pThis->io_, boost::posix_time::seconds(5));
		t.wait();
		LOCAL_REV_DATA* ptr_recv_data = new LOCAL_REV_DATA;
		bzero(ptr_recv_data->buf, DATA_BUFFER_SIZE);
		ptr_recv_data->base = NULL;
		ptr_recv_data->len = 0;
		ptr_recv_data->sfd = 0;

		if(!pThis->InitSocketFd(ptr_recv_data->sfd))
		{
			continue;
		}

		if(!pThis->AddNewConnToEvent(ptr_recv_data))
		{
			continue;
		}

		pThis->local_connect_flag_ = true;

		int ret = event_base_dispatch(ptr_recv_data->base);
		if (-1 == ret)
		{
			LOG4CXX_FATAL(g_logger,
					"CLocalTransport::Run():event_base_dispatch description = " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		} else if (1 == ret)
		{
			LOG4CXX_FATAL(g_logger,
					"CLocalTransport::Run():no events were registered.");
		}

		pThis->local_connect_flag_ = false;

	}
	return NULL;
}

bool CLocalTransport::AddNewConnToEvent(LOCAL_REV_DATA* ptr_recv_data)
{
	ptr_recv_data->base = event_base_new();
	assert(ptr_recv_data->base  != NULL);
	int flag = EV_READ | EV_PERSIST;
	struct bufferevent *logic_pro_bev = bufferevent_socket_new(ptr_recv_data->base , ptr_recv_data->sfd, BEV_OPT_CLOSE_ON_FREE);
	if(logic_pro_bev == NULL)
	{
		return false;
	}
	bufferevent_setcb(logic_pro_bev, ReadCb, NULL, ErrorCb, (void*) ptr_recv_data);
	bufferevent_enable(logic_pro_bev, flag);
	return true;
}

bool CLocalTransport::InitSocketFd(evutil_socket_t& sfd)
{
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd < 0)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::InitSocketFd:socket error = " << strerror(errno));
		return false;
	}

	int flags = 1;
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void *) &flags, sizeof(flags)) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::InitSocketFd:setsockopt SO_REUSEADDR error = " << strerror(errno));
		close(sfd);
		return false;
	}

	/* 绑定固定端口以便于分发进程识别 */
	sockaddr_in localAddr;
	bzero(&localAddr, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddr.sin_port = htons(utils::G<CGlobalSettings>().bind_port_);

	if (bind(sfd, (const sockaddr*) &localAddr, sizeof(localAddr)) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::InitSocketFd:bind error = " << strerror(errno));
		close(sfd);
		return false;
	}

	sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr((utils::G<CGlobalSettings>().gateway_ip_).c_str());
	servaddr.sin_port = htons(utils::G<CGlobalSettings>().gateway_port_);

	if (connect_nonb(sfd, (struct sockaddr*) &servaddr, sizeof(servaddr), 5) < 0)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::InitSocketFd:connect error = " << strerror(errno));
		close(sfd);
		return false;
	}

	if (setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (void *) &flags, sizeof(flags)) != 0)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::InitSocketFd:setsockopt TCP_NODELAY error = " << strerror(errno));
		close(sfd);
		return false;
	}
	CJsonOpt jsonOpt;
	std::string proMsg = jsonOpt.JsonJoinDistributedMessage();
	proMsg.append(CRLF);

	if( !SocketOperate::WriteSfd(sfd, proMsg.c_str(), proMsg.length()))
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::WriteSfd to Dispach error = " << strerror(errno));
		close(sfd);
		return false;
	}

	LOG4CXX_TRACE(g_logger, "CLocalTransport::InitSocketFd:sfd = " << sfd << " process message =" << proMsg);

	g_clientInfo.sfd = sfd;

	return true;
}

void CLocalTransport::Run()
{
	LOG4CXX_INFO(g_logger, "CLocalTransport::Run: localTransport has start...");

	/* 创建连接逻辑分发进程的线程 */
	CreateWorker(SetupLocalTransport, (void*)this);

	int ret = event_base_dispatch(main_base_);
	if (-1 == ret)
	{
		LOG4CXX_FATAL(g_logger, "CLocalTransport::Run():event_base_dispatch description = " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
	}
	else if (1 == ret)
	{
		LOG4CXX_FATAL(g_logger, "CLocalTransport::Run():no events were registered.");
	}

	event_base_free(main_base_);
}

void CLocalTransport::ReadCb(struct bufferevent *bev, void *arg)
{
	LOCAL_REV_DATA* ptr_data = static_cast<LOCAL_REV_DATA*> (arg);

	int recv_size = 0;
	if ((recv_size = bufferevent_read(bev, ptr_data->buf + ptr_data->len, DATA_BUFFER_SIZE - ptr_data->len)) > 0)
	{
		ptr_data->len = ptr_data->len + recv_size;
	}

	LOG4CXX_TRACE(g_logger, "CLocalTransport::DoRead buf = " << ptr_data->buf);

	std::string str_recv(ptr_data->buf, ptr_data->len);
	if (utils::FindCRLF(str_recv))
	{
		/* 有可能同时收到多条信息 */
		std::vector<std::string> vec_str;
		utils::SplitData(str_recv, CRLF, vec_str);

		for (unsigned int i = 0; i < vec_str.size(); ++i)
		{
			consumers_->putData(vec_str.at(i));
		}

		int len = str_recv.find_last_of(CRLF) + 1;
		memmove(ptr_data->buf, ptr_data->buf + len, DATA_BUFFER_SIZE - len);
		ptr_data->len = ptr_data->len - len;
	}
}

void CLocalTransport::ErrorCb(struct bufferevent *bev, short event, void *arg)
{
	LOCAL_REV_DATA* ptr_data = static_cast<LOCAL_REV_DATA*> (arg);
	utils::SafeDelete(ptr_data);

	if (event & BEV_EVENT_TIMEOUT)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::ErrorCb:Timed out.");
	}
	else if (event & BEV_EVENT_EOF)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::ErrorCb:connection closed.");
	}
	else if (event & BEV_EVENT_ERROR)
	{
		LOG4CXX_ERROR(g_logger, "CLocalTransport::ErrorCb:some other error.");
	}
	bufferevent_free(bev);
}

int CLocalTransport::connect_nonb( int sockfd, struct sockaddr* saptr, socklen_t salen, int nsec )
{
	int flags, n, error = 0;
	socklen_t len;
	fd_set rset, wset;
	struct timeval tval;

	flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	if ((n = ::connect(sockfd, saptr, salen) < 0))
	{
		if (EINPROGRESS != errno)
		{
			LOG4CXX_ERROR(g_logger, "CLocalTransport::connect_nonb:connect failed. errorcode = " << errno);
			return (-1);
		}
	}

	if(0 == n)
		goto done;

	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	wset = rset;
	tval.tv_sec = nsec;
	tval.tv_usec = 0;

	if ((n = select(sockfd+1, &rset, &wset, NULL, nsec?&tval:NULL)) == 0)
	{
		close(sockfd);
		errno = ETIMEDOUT;
		LOG4CXX_ERROR(g_logger, "CLocalTransport::connect_nonb:select failed. errorcode = " << errno);
		return -1;
	}

	if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset))
	{
		len = sizeof(error);
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&error, &len) < 0)
		{
			LOG4CXX_ERROR(g_logger, "CLocalTransport::connect_nonb:getsockopt failed. errorcode = " << errno);
			return -1;
		}
	}
	else
		LOG4CXX_FATAL(g_logger, "CLocalTransport::connect_nonb:select error: sockfd not set");

done:
	if (error)
	{
		close(sockfd);
		errno = error;
		LOG4CXX_ERROR(g_logger, "CLocalTransport::connect_nonb:ioctlsocket1 failed. errorcode = " << errno);
		return -1;
	}

	return 0;
}


