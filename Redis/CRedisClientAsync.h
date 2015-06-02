#ifndef CREDIS_CLIENT_ASYNC_H
#define CREDIS_CLIENT_ASYNC_H

#include <hiredis.h>
#include <string>
#include <stdlib.h>
#include <list>
#include "locker.h"
#include "CRedisClient.h"

/*
* CRedisClientAsync 对象
*/

typedef enum
{
	CMD_UNKNOW = -1,
	CMD_GET = 0,
	CMD_SET = 1,
	CMD_SADD = 2,
	CMD_SPOP = 3,
	CMD_SCARD = 4,
	CMD_GET_STR_LIST = 5,
	CMD_GET_INT_LIST = 6,
}CMD_TYPE;


typedef void (*CmdCallBack)(int iErrCode, int iCount, CMD_TYPE eType, void* pRetData, void* pArg);


class CRedisClientAsync
{
public:
	typedef struct  
	{
		CMD_TYPE eType;
		std::string cmd;
	}cmd_data;

	
	CRedisClientAsync();

	~CRedisClientAsync();

	bool OnInit(std::string& address, int& port);

	void SetCallBack(CmdCallBack callBack, void* pArg)
	{
		m_pCallBack = callBack;
		m_pCallBackArg = pArg;
	}

	void ExecCommand(const char* cmd, CMD_TYPE eType);
	void ExecCommand(std::string& cmd, CMD_TYPE eType);

	//测试链接是否可用，若不可用，则将进行重连
	bool TestOnline();

protected:
	static void* ThreadFun(void* pArg);

	void OnThreadWorker();

	void RunRedisCmd(std::string& cmd, CMD_TYPE eType);
private:
	CmdCallBack m_pCallBack;
	void* m_pCallBackArg;

	bool m_isInited;
	bool m_isStop;
	Locker m_locker;
	pthread_t m_threadid;
	std::list<cmd_data*> m_cmdList;
	Condition* m_pCondition;

	CRedisClient m_RedisClient;
};
#endif //CREDIS_CLIENT_ASYNC_H