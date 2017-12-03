//============================================================================
// Name        : logicXX.cpp
// Author      : yaocoder
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "defines.h"
#include "init_configure.h"
#include "local_transport.h"
#include "global_settings.h"
#include "../public/config_file.h"
#include "../public/utils.h"

CSqlConnPool g_OptUserDataDB;

static void InitConfigure();
static void SettingsAndPrint();
static void Run();
static void SigUsr(int signo);

int main(int argc, char **argv)
{

	/* process arguments */
	int c;
	std::string version = std::string("1.0.404:412M");
	while (-1 != (c = getopt(argc, argv,
		  "v" 	/* 获取程序版本号，配合svn */
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

	if (signal(SIGUSR1, SigUsr) == SIG_ERR )
	{
		LOG4CXX_FATAL(g_logger, "Configure signal failed.");
		exit(EXIT_FAILURE);
	}

	InitRedis();

	InitXXSql();

	if (daemon(1, 0) == -1)
	{
		LOG4CXX_FATAL(g_logger, "daemon failed.");
	}

	Run();

	return EXIT_SUCCESS;
}

void Run()
{
	CLocalTransport localTransport;
	if (!localTransport.SetupTransport())
	{
		LOG4CXX_FATAL(g_logger, "SetupTransport failed.");
		exit(EXIT_FAILURE);
	}

	localTransport.Run();
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
	std::string project_name = "logicXX";
	initConfObj.InitLog4cxx(project_name);
	if (!initConfObj.LoadConfiguration(project_name))
	{
		LOG4CXX_FATAL(g_logger, "LoadConfiguration failed.");
		exit(EXIT_FAILURE);
	}
}

void SettingsAndPrint()
{
	utils::G<CGlobalSettings>().dispatch_ip_ 	= utils::G<ConfigFile>().read<std::string> ("logicDispatch.local.ip","127.0.0.1");
	utils::G<CGlobalSettings>().dispatch_port_ 	= utils::G<ConfigFile>().read<int> ("logicDispatch.local.listen.port", 10000);
	utils::G<CGlobalSettings>().bind_port_ 		= utils::G<ConfigFile>().read<int> ("logicXX.bind.port", 10001);
	utils::G<CGlobalSettings>().thread_num_ 	= utils::G<ConfigFile>().read<int> ("logicXX.worker.thread.num", 4);
	utils::G<CGlobalSettings>().queue_num_ 		= utils::G<ConfigFile>().read<int> ("logicXX.worker.queue.num", 16);
	utils::G<CGlobalSettings>().stats_listen_port_ = utils::G<ConfigFile>().read<int>("logicXX.stats.listen.port", 10002);
	
	LOG4CXX_INFO(g_logger, "******logicDispatch.local.ip = " 	<< utils::G<CGlobalSettings>().dispatch_ip_ << "******");
	LOG4CXX_INFO(g_logger, "******dispatch.proc.port =" 		<< utils::G<CGlobalSettings>().dispatch_port_<< "******");
	LOG4CXX_INFO(g_logger, "******logicXX.bind.port=" 		<< utils::G<CGlobalSettings>().bind_port_ << "******");
	LOG4CXX_INFO(g_logger, "******logicXX.worker.thread.num ="<< utils::G<CGlobalSettings>().thread_num_<< "******");
	LOG4CXX_INFO(g_logger, "******logicXX.worker.queue.num =" << utils::G<CGlobalSettings>().queue_num_<< "******");
	LOG4CXX_INFO(g_logger, "******logicXX.stats.listen.port = "  << utils::G<CGlobalSettings>().stats_listen_port_ << "******");
}

