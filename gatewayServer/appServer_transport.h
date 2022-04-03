/*
 * local_transport.h
 *
 *  Created on: Mar 7, 2013
 *      Author: yaowei
 */

#ifndef LOCAL_TRANSPORT_H_
#define LOCAL_TRANSPORT_H_

#include "defines.h"
#include <boost/thread.hpp>

class CAppServerTransport
{

public:

	static CAppServerTransport* GetInstance();

	void SetupAppServerTransport();

	bool SendToAppServerModule(const char* message, const std::string& module_type, const conn* c);

private:

	CAppServerTransport();

	bool InitAppServerListenSocket(evutil_socket_t& listen_socket);

	void CreateThreadForListenAppServer(void *(*func)(void *), void *arg);

	static void *ReadLibevent(void *arg);

	static void AcceptCb(evutil_socket_t listen_socket, short event, void* arg);

	static void ReadCb(struct bufferevent *bev, void *arg);

	static void ErrorCb(struct bufferevent *bev, short event, void *arg);

	static void SendToClient(const char* buf, const int local_sfd);

private:

	evutil_socket_t sfd_;

	static CAppServerTransport* appServer_transport_ptr_;


	struct event_base 	*main_base_;
	struct event		*appServer_listen_event_;
	evutil_socket_t 	appServer_listen_socket_;

	static std::map<std::string, int>  map_appServer_sfd_;		//连接网关进程的微服务进程的sfd-->业务微服务进程标识

	static boost::mutex mutex_;
};

#endif /* LOCAL_TRANSPORT_H_ */
