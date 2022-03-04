//============================================================================
// Name        : gatewayServer.cpp
// Author      : yaocoder
// Version     :
// Copyright   : Your copyright notice
// Description : 网关服务，实现类nginx的反向代理功能
//============================================================================

#include <stdlib.h>
#include <signal.h>
#include "defines.h"
#include "init_configure.h"
#include "master_thread.h"
#include "global_settings.h"
#include "../public/utils.h"
#include "../public/config_file.h"

static void InitConfigure();
static void SettingsAndPrint();
static void Run();
static void SigUsr(int signo);

int main(int argc, char **argv)
{

	/* process arguments */
	int c;
	std::string version = std::string("1.0.");
	while (-1 != (c = getopt(argc, argv, "v" /* 获取程序版本号，配合svn */
	)))
	{
		switch (c)
		{
		case 'v':
			printf("The version is %s\n", version.c_str());
			return EXIT_SUCCESS;
		default:
			break;
		}
	}

	InitConfigure();

	SettingsAndPrint();

	if(signal(SIGUSR1, SigUsr) == SIG_ERR)
	{
		LOG4CXX_FATAL(g_logger, "Configure signal failed.");
		exit(EXIT_FAILURE);
	}

	if (daemon(1, 0) == -1)
	{
		LOG4CXX_FATAL(g_logger, "daemon failed.");
	}

	Run();

	return EXIT_SUCCESS;
}

void Run()
{
	CMasterThread masterThread;
	if(!masterThread.InitMasterThread())
	{
		LOG4CXX_FATAL(g_logger, "InitNetCore failed.");
		exit(EXIT_FAILURE);
	}

	masterThread.Run();
}

void SigUsr(int signo)
{
	if(signo == SIGUSR1)
	{
		/* 重新加载应用配置文件（仅仅是连接超时时间），log4cxx日志配置文件*/
		InitConfigure();
		SettingsAndPrint();
		LOG4CXX_INFO(g_logger, "reload configure.");
		return;
	}
}

void InitConfigure()
{
	CInitConfig initConfObj;
	initConfObj.SetConfigFilePath(std::string("../conf/"));
	std::string project_name = "gatewayServer";
	initConfObj.InitLog4cxx(project_name);
	if (!initConfObj.LoadConfiguration(project_name))
	{
		LOG4CXX_FATAL(g_logger, "LoadConfiguration failed.");
		exit(EXIT_FAILURE);
	}
}

void SettingsAndPrint()
{
	utils::G<CGlobalSettings>().remote_listen_port_ = utils::G<ConfigFile>().read<int>("gatewayServer.remote.listen.port", 11001);
	utils::G<CGlobalSettings>().thread_num_  		= utils::G<ConfigFile>().read<int>("gatewayServer.worker.thread.num", 4);
	utils::G<CGlobalSettings>().local_listen_port_	= utils::G<ConfigFile>().read<int>("gatewayServer.local.listen.port", 10000);
	utils::G<CGlobalSettings>().client_timeout_		= utils::G<ConfigFile>().read<int>("gatewayServer.client.timeout.s", 11);
	utils::G<CGlobalSettings>().appServerXX_port_	= utils::G<ConfigFile>().read<int>("logicXX.bind.port", 11002);


	LOG4CXX_INFO(g_logger, "******gatewayServer.remote.listen.port = "	<< utils::G<CGlobalSettings>().remote_listen_port_	<< "******");
	LOG4CXX_INFO(g_logger, "******gatewayServer.worker.thread.num = "	<< utils::G<CGlobalSettings>().thread_num_	 	  	<< "******");
	LOG4CXX_INFO(g_logger, "******gatewayServer.local.listen.port= "	<< utils::G<CGlobalSettings>().local_listen_port_ 	<< "******");
	LOG4CXX_INFO(g_logger, "******gatewayServer.client.timeout.s = "	<< utils::G<CGlobalSettings>().client_timeout_ 		<< "******");
	LOG4CXX_INFO(g_logger, "******appServerXX.bind.port = " 			<< utils::G<CGlobalSettings>().appServerXX_port_   << "******");


}




