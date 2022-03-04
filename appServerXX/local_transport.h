/*
 * local_transport.h
 *
 *  Created on: Mar 7, 2013
 *      Author: yaowei
 */

#ifndef LOCAL_TRANSPORT_H_
#define LOCAL_TRANSPORT_H_

#include "defines.h"
#include "consumers.h"
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

class CLocalTransport
{
public:
	CLocalTransport();
	virtual ~CLocalTransport();

public:

	bool SetupTransport();

	void Run();

private:

	bool CheckLibeventVersion();

	bool InitStatsSocket(evutil_socket_t& listen_socket);

	static void AcceptCb(struct bufferevent *bev, void *arg);


	void CreateWorker(void *(*func)(void *), void *arg);

	static void *SetupLocalTransport(void *arg);

	bool InitSocketFd(evutil_socket_t& sfd);

	int connect_nonb( int sockfd, struct sockaddr* saptr, socklen_t salen, int nsec );

	bool AddNewConnToEvent(LOCAL_REV_DATA* ptr_recv_data);

	static void ReadCb(struct bufferevent *bev, void *arg);

	static void ErrorCb(struct bufferevent *bev, short event, void *arg);

private:

	struct event_base 	*main_base_;
	evutil_socket_t		stats_listen_sfd_;
	evutil_socket_t 	local_tranfer_sfd_;
	boost::asio::io_service io_;
	bool	local_connect_flag_;
	static consumers	*consumers_;
	struct event		*recv_event_;
};


#endif /* LOCAL_TRANSPORT_H_ */
