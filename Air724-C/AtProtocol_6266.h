#ifndef _AT_PROTOCOL_6266
#define _AT_PROTOCOL_6266


typedef enum sAT6266AckType
{
    AT6266ACK_TYPE_SUCCEED,
    AT6266ACK_TYPE_FAIL,
    AT6266ACK_TYPE_SEND,
    AT6266ACK_TYPE_CONNECT,
    AT6266ACK_TYPE_UNKNOW,
}AT6266AckType_t;


char* AT6266Test(char *s);
/**
 * @brief 复位WIFI模组命令
*/
char* AT6266Rst(char *s);
/**
 * @brief 恢复出厂值命令
*/
char* AT6266Reset(char *s);

/**
 * @brief 设置串口命令
*/
char* AT6266SetUart(char *s,int band);

/**
 * @brief 设置电源模式
 * @mode 
*/
char* AT6266SetPowerMode(char *s,char mode);

/**
 * @brief 设置WIFI模式为sta
*/
char* AT6266WifiMode(char *s);
/**
 * @brief 连接wifi
*/
char* AT6266ConnectAp(char *s,char *name,char *password);

/**
 * @brief 查询网络连接状态
*/
char* AT6266NetStatus(char *s);

/**
 * @brief 先发送此命令，再发送数据
*/
char* AT6266PrepareTcpSend(char *s,unsigned short len);

/**
 * 测试网络连接情况
*/
char *AT6266Ping(char *s,char *host);

/**
 * @brief 返回连接mqtt服务器 订阅主题的命令组，暂时写死
*/
char AT6266MqttConnect(char s[8][50]);

char* AT6266TcpConnect(char *s);

AT6266AckType_t AT6266DecodeAck(char *str,char rtParam);


#endif