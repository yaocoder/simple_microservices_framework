/*
 * logic_operate.h
 *
 *  Created on: Mar 11, 2013
 *      Author: yaowei
 */

#ifndef LOGIC_OPERATE_H_
#define LOGIC_OPERATE_H_

#include "defines.h"
#include "../public/user_interface_defines.h"

class CLogicOperate
{
public:
	CLogicOperate();
	virtual ~CLogicOperate();

public:

	void StartLogicOpt(const std::string& message);

private:

	void SendToDispatch();

private:

	std::string responseToDispatch_;
};

#endif /* LOGIC_OPERATE_H_ */
