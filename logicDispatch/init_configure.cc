/*
 * Init_log4cxx.cc
 *
 *  Created on: 2012-9-12
 *      Author: yaowei
 */

#include "init_configure.h"
#include "defines.h"
#include "../public/utils.h"
#include "../public/config_file.h"

#define LOG_CON_FILE        "_log.conf"
#define CONF_FILE        	"_config.ini"

LoggerPtr g_logger;


CInitConfig::CInitConfig()
{

}

CInitConfig::~CInitConfig()
{
}

void CInitConfig::InitLog4cxx(const std::string& project_name)
{
	std::string log_path = config_file_path_ + project_name + std::string(LOG_CON_FILE);
	PropertyConfigurator::configure(log_path);
	g_logger = Logger::getLogger(project_name);
	LOG4CXX_INFO(g_logger, "Run...");
}

bool CInitConfig::LoadConfiguration(const std::string& project_name)
{
	std::locale old_locale = std::locale::global(std::locale(""));
	std::string file_path = config_file_path_ + project_name + CONF_FILE;
	std::ifstream conf_file(file_path.c_str());
	std::locale::global(old_locale);
	if (!conf_file)
	{
		LOG4CXX_ERROR(g_logger, "CInitConfig::LoadConfiguration failed.");
		return false;
	}
	conf_file >> utils::G<ConfigFile>();
	return true;
}


