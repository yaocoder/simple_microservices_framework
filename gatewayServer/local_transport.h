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

class CLocalTransport
{

public:

	static CLocalTransport* GetInstance();

	void SetupLocalTransport();

	bool SendToLogicModule(const char* message, const std::string& module_type, const conn* c);

private:

	CLocalTransport();

	bool InitLocalListenSocket(evutil_socket_t& listen_socket);

	void CreateThreadForListenLocal(void *(*func)(void *), void *arg);

	static void *ReadLibevent(void *arg);

	static void AcceptCb(evutil_socket_t listen_socket, short event, void* arg);

	static void ReadCb(struct bufferevent *bev, void *arg);

	static void ErrorCb(struct bufferevent *bev, short event, void *arg);

	static void SendToClient(const char* buf, const int local_sfd);

private:

	evutil_socket_t sfd_;

	static CLocalTransport* local_transport_ptr_;


	struct event_base 	*main_base_;
	struct event		*local_listen_event_;
	evutil_socket_t 	local_listen_socket_;

	static std::map<std::string, int>  map_logic_sfd_;		//连接分发进程的业务进程sfd-->业务进程标识

	static boost::mutex mutex_;
};

#endif /* LOCAL_TRANSPORT_H_ */
