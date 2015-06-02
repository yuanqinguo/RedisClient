#include "CRedisClientAsync.h"

#define WAIT_SECOND 5

CRedisClientAsync::CRedisClientAsync()
{
	m_isStop = false;
	m_isInited = false;
	m_pCondition = new Condition(m_locker);
}

CRedisClientAsync::~CRedisClientAsync()
{
	m_isStop = true;
	m_isInited = false;
	if(m_pCondition)
	{
		m_pCondition->signalAll();
		delete m_pCondition;
		m_pCondition = NULL;
	}
}

bool CRedisClientAsync::OnInit(std::string& address, int& port)
{
	bool bret = m_RedisClient.OnInit(address, port);
	if(bret)
	{
		pthread_create(&m_threadid, NULL, ThreadFun, this);
	}
	m_isInited = bret;
	return bret;
}

bool CRedisClientAsync::TestOnline()
{
	return m_RedisClient.TestOnline();
}


void CRedisClientAsync::ExecCommand(std::string& cmd, CMD_TYPE eType)
{
	{
		cmd_data* pData = new cmd_data();
		pData->cmd = cmd;
		pData->eType = eType;
		LockerGuard guard(m_locker);
		m_cmdList.push_back(pData);
	}

	m_pCondition->signal();
}

void CRedisClientAsync::ExecCommand(const char* cmd, CMD_TYPE eType)
{
	{
		cmd_data* pData = new cmd_data();
		pData->cmd = cmd;
		pData->eType = eType;
		LockerGuard guard(m_locker);
		m_cmdList.push_back(pData);
	}

	m_pCondition->signal();
}

void* CRedisClientAsync::ThreadFun(void* pArg)
{
	CRedisClientAsync* pThisObj = (CRedisClientAsync*)pArg;
	if(pThisObj)
		pThisObj->OnThreadWorker();

	return NULL;
}

void CRedisClientAsync::OnThreadWorker()
{
	std::string cmd;
	CMD_TYPE eType = CMD_UNKNOW;
	while (!m_isStop)
	{
		m_locker.lock();
		if (m_cmdList.empty())
		{
			m_locker.unlock();
			continue;
		}

		if(m_isInited)
		{
			cmd_data* pdata = NULL;
			std::list<cmd_data*>::iterator it;
			for ( it = m_cmdList.begin(); it != m_cmdList.end(); )
			{
				pdata = *it;
				eType = CMD_UNKNOW;
				cmd = pdata->cmd;
				eType = (CMD_TYPE)pdata->eType;
				RunRedisCmd(cmd, eType);

				m_cmdList.erase(it++);
			}
		}

		m_pCondition->waitForSeconds(WAIT_SECOND);
		m_locker.unlock();
	}
}

void CRedisClientAsync::RunRedisCmd(std::string& cmd, CMD_TYPE eType)
{
	int iErrorCode = 0;
	void* pCallData = NULL;
	int iCount = 0;

	switch(eType)
	{
	case CMD_GET:
	case CMD_SPOP:
		{
			std::string result;
			iErrorCode = m_RedisClient.ExecCommand(cmd, result);
			if (m_pCallBack)
			{
				iCount = 1;
				m_pCallBack(iErrorCode, iCount, CMD_GET, (void*)result.c_str(), m_pCallBackArg);
			}
		}break;

	case CMD_SCARD:
		{
			int result;
			iErrorCode = m_RedisClient.ExecCommand(cmd, result);
			if (m_pCallBack)
			{
				iCount = 1;
				m_pCallBack(iErrorCode, iCount, CMD_SCARD, (void*)&result, m_pCallBackArg);
			}
		}break;

	case CMD_GET_STR_LIST:
		{
			std::vector<std::string> result;
			iErrorCode = m_RedisClient.ExecCommand(cmd, result);
			if (m_pCallBack)
			{
				iCount = result.size();
				m_pCallBack(iErrorCode, iCount, CMD_GET_STR_LIST, (void*)&result, m_pCallBackArg);
			}
		}break;

	case CMD_GET_INT_LIST:
		{
			std::vector<int> result;
			iErrorCode = m_RedisClient.ExecCommand(cmd, result);
			if (m_pCallBack)
			{
				iCount = result.size();
				m_pCallBack(iErrorCode, iCount, CMD_GET_STR_LIST, (void*)&result, m_pCallBackArg);
			}
		}break;
	case CMD_SET:
	case CMD_SADD:
		{
			iErrorCode = m_RedisClient.ExecCommand(cmd);
			if (m_pCallBack)
			{
				pCallData = (void*)"OK";
				m_pCallBack(iErrorCode, iCount, CMD_SADD, pCallData, m_pCallBackArg);
			}
		}break;

	default:
		{
			iErrorCode = -1;
			if (m_pCallBack)
			{
				pCallData = (void*)"CMD_UNKNOW";
				m_pCallBack(iErrorCode, iCount, CMD_UNKNOW, pCallData, m_pCallBackArg);
			}
		}break;
	}
}