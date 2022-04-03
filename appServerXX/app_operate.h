/*
 * app_operate.h
 *
 *  Created on: Mar 11, 2013
 *      Author: yaowei
 */

#ifndef APP_OPERATE_H_
#define APP_OPERATE_H_

#include "defines.h"

class CAppOperate
{
public:
	CAppOperate();
	virtual ~CAppOperate();

public:

	void StartAppOpt(const std::string& message);

private:

	void SendToGateway();

private:

	std::string responseToGateway_;
};

#endif /* APP_OPERATE_H_ */
