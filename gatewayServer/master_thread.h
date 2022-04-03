/*
 * master_thread.h
 *
 *  Created on: Mar 4, 2013
 *      Author: yaowei
 */

#ifndef MASTER_THREAD__H_
#define MASTER_THREAD__H_

#include "defines.h"
#include "threadSafe_map.h"

class CWorkerThread;

class CMasterThread
{
public:
	CMasterThread();
	virtual ~CMasterThread();

public:

	bool InitMasterThread();

	void Run();

	static CThreadSafeMap<int, int> map_csfd_id_;          //客户端连接sfd-->随机唯一id，和业务进程传来的sfd和id对应，防止sfd被其他客户端重用导致错发

private:

	bool CheckLibeventVersion();

	bool InitRemoteListenSocket(evutil_socket_t& listen_socket);

	static void AcceptCb(evutil_socket_t listen_socket, short event, void* arg);

	static int  GetClientfdMapId();

private:

	struct event_base 	*main_base_;
	evutil_socket_t 	remote_listen_socket_;
	struct event		*listen_event_;

	static int id_;

	CWorkerThread			*work_thread_ptr_;
};


#endif /* NET_CORE_H_ */
