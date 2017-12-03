/*
 * json_pt.cc
 *
 *  Created on: Mar 6, 2013
 *      Author: yaowei
 */

#include "json_opt.h"
#include "../public/message.h"

CJsonOpt::CJsonOpt()
{
}

CJsonOpt::~CJsonOpt()
{
}

bool CJsonOpt::JsonParseDistributedMessage(const char *json_in_string, int& promsg_type, std::string& sendSys_name, std::string& receiveSys_name)
{
	try
	{
		in_ = libjson::parse(json_in_string);
	}
	catch (...)
	{
		return false;
	}

	if (VerifyJsonField(in_, JK_PROCESS_MESSAGE_TYPE) && VerifyJsonField(in_, JK_SEND_PROCESS_NAME)
			&& VerifyJsonField(in_, JK_RECEIVE_PROCESS_NAME))
	{
		promsg_type = in_[JK_PROCESS_MESSAGE_TYPE].as_int();
		sendSys_name = in_[JK_SEND_PROCESS_NAME].as_string();
		receiveSys_name = in_[JK_RECEIVE_PROCESS_NAME].as_string();
		return true;
	}
	return false;
}


bool CJsonOpt::RestructJsonStringToLogic(const char *json_in_string, const int sfd, const int sfd_id, std::string& json_out_string, std::string& type)
{
	try
	{
		in_ = libjson::parse(json_in_string);
	}
	catch (...)
	{
		return false;
	}

	if (!VerifyCommonJson(in_))
	{
		return false;
	}

	type = in_[JK_LOGIC_PROCESS_TYPE].as_string();

	in_.push_back(JSONNode(JK_CLINET_SFD, sfd));
	in_.push_back(JSONNode(JK_CLINET_SFD_ID, sfd_id));
	json_out_string = in_.write();

	return true;
}

bool CJsonOpt::RestructJsonStringToclient(const char* json_in_string, std::string& json_out_string, int& sfd, int& sfd_id)
{
	try
	{
		out_ = libjson::parse(json_in_string);
	}
	catch(...)
	{
		LOG4CXX_WARN(g_logger, "CJsonOpt::RestructJsonStringToclient:parse failed. string = " << json_in_string);
		return false;
	}

	if(!(VerifyJsonField(out_, JK_CLINET_SFD) && VerifyJsonField(out_, JK_CLINET_SFD_ID)))
	{
		return false;
	}

	sfd 	= out_[JK_CLINET_SFD].as_int();
	sfd_id 	= out_[JK_CLINET_SFD_ID].as_int();
	out_.pop_back(JK_CLINET_SFD);
	out_.pop_back(JK_CLINET_SFD_ID);

	json_out_string = out_.write();

	return true;
}

bool CJsonOpt::VerifyCommonJson(JSONNode & in)
{
	return VerifyJsonField(in, JK_LOGIC_PROCESS_TYPE)&&VerifyJsonField(in, JK_PROTO_VERSION);
}


bool CJsonOpt::VerifyJsonField(const JSONNode & in, const std::string & field)
{
	try
		{
			return (in.end()!= in.find(field));
		}
		catch(...)
		{
			// LOG4CXX_ERROR(g_logger, "json异常");
			return false;
		}
}


