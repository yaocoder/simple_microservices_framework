/*
 * json_opt.h
 *
 *  Created on: Mar 6, 2013
 *      Author: yaowei
 */

#ifndef JSON_OPT_H_
#define JSON_OPT_H_

#include <libjson/libjson.h>
#include <libjson/Source/JSONNode.h>
#include "defines.h"

class CJsonOpt
{
public:
	CJsonOpt();
	virtual ~CJsonOpt();

	bool JsonParseDistributedMessage(const char *json_in_string, int& promsg_type, std::string& sendSys_name, std::string& receiveSys_name);

	bool RestructJsonStringToLogic(const char *json_in_string, const int sfd, const int sfd_id, std::string& json_out_string, std::string& type);

	bool RestructJsonStringToclient(const char* json_in_string, std::string& json_out_string, int& sfd, int& sfd_id);

private:

	bool VerifyCommonJson(JSONNode& in);

	bool VerifyJsonField(const JSONNode& in, const std::string& field);

private:

	JSONNode in_;
	JSONNode out_;

};

#endif /* JSON_OPT_H_ */
