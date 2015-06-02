#include "CRedisClientAsync.h"
#include "CRedisClient.h"
#include "CUtilRedis.h"

#define SERVER_HOST "192.168.1.161"
#define SERVER_PORT 6379

bool g_quit = false;
int g_callbackCount = 0;

void CmdCallBackFun(int iErrCode, int iCount, CMD_TYPE eType, void* pRetData, void* pArg)
{
	printf("CmdCallback::iErrCode = %d, iCount = %d, eType = %d\n", iErrCode, iCount, eType);

	switch(eType)
	{
	case CMD_GET:
	case CMD_SPOP:
		{
			if (pRetData != NULL)
			{
				char* pret = (char*)pRetData;
				printf("CmdCallBack::retdata = %s\n", pret);
			}
		}break;
	case CMD_SET:
		{
			if (pRetData != NULL)
			{
				char* pret = (char*)pRetData;
				printf("CmdCallBack::retdata = %s\n", pret);
			}
		}break;
	default:
		break;
	}
	g_callbackCount++;
	if (g_callbackCount > 1)
	{
		g_quit = true;
	}

}

int main(int argc, char** argv)
{
	std::string server_host = SERVER_HOST;
	int server_port = SERVER_PORT;

	std::string keyHeader = "001";
	std::string keyAppend = "_test";
	//构造一个key为：001_test
	std::string key = CUtilRedis::MakeOneKey(keyHeader, keyAppend.c_str());

	std::string keyValue = "test001"; //key的值为 test001
	std::string cmdset = CUtilRedis::MakeSetRedisCmd(key, keyValue);

	//同步对象调用
	CRedisClient redisclient;
	redisclient.OnInit(server_host, server_port);

	//测试链接是否可用，不可用将在内部重连，返回true表示连接可用
	if (!redisclient.TestOnline())
		return -1;

	//SET
	int iret = redisclient.ExecCommand(cmdset);
	printf("CRedisClient.ExecCommand Set return iret = %d\n", iret);

	//GET
	std::string cmdget = CUtilRedis::MakeGetRedisCmd(key);
	iret = redisclient.ExecCommand(cmdget, keyValue);
	printf("CRedisClient.ExecCommand Get return iret = %d, KeyValue = %s\n", iret, keyValue.c_str());


	//异步对象调用
	CRedisClientAsync redisClientAsync;
	redisClientAsync.OnInit(server_host, server_port);
	redisClientAsync.SetCallBack(CmdCallBackFun, NULL);

	redisClientAsync.ExecCommand(cmdget, CMD_GET);

	keyValue = "Test002";
	cmdset = CUtilRedis::MakeSetRedisCmd(key, keyValue);
	redisClientAsync.ExecCommand(cmdset, CMD_SET);

	redisClientAsync.ExecCommand(cmdget, CMD_GET);

	do 
	{
#ifdef WIN32
		Sleep(100);
#else
		sleep(1);
#endif
	} while (!g_quit);

	getchar();
	return 0;
}