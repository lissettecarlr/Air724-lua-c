/**
 * @file Atprotocopl_6266
 * @brief 南方硅谷6266WIFI模块的AT通讯协议
 * @details 
 * @author
 * @version
 * @date
 */
#include <stdlib.h>
#include <string.h>

/**
 * @brief 插入字符串
*/
char *strcat_m(char *dest,const char *str)
{
	char *cp = dest;
	while(*cp!='\0') ++cp;
 
	while((*cp++=*str++)!='\0')
	{}
	return dest;
}

/**
 * @brief 数组转字符串
*/
char *itoa(int num, char *str, int radix)
{ /*索引表*/
    char index[] = "0123456789ABCDEF";
    unsigned unum; /*中间变量*/
    int i = 0, j, k;
    /*确定unum的值*/
    if (radix == 10 && num < 0) /*十进制负数*/
    {
        unum = (unsigned)-num;
        str[i++] = '-';
    }
    else
        unum = (unsigned)num; /*其他情况*/
    /*转换*/
    do
    {
        str[i++] = index[unum % (unsigned)radix];
        unum /= radix;
    } while (unum);
    str[i] = '\0';
    /*逆序*/
    if (str[0] == '-')
        k = 1; /*十进制负数*/
    else
        k = 0;
    char temp;
    for (j = k; j <= (i - 1) / 2; j++)
    {
        temp = str[j];
        str[j] = str[i - 1 + k - j];
        str[i - 1 + k - j] = temp;
    }
    return str;
}

typedef struct sAT6266MqttCmd
{
    char *SetMode;
    char *SetAp;
    char *SetMqttServerName;
    char *SetMqttServerPassword;
    char *SetMqttSub;
    char *SetMqttPub;
    char *SetMqttHost;
    char *startMqtt;
}AT6266MqttCmd_t;



typedef struct sAT6226Cmd
{
    char *test;
    char *rst;
    char *reset;
    char *setUart;
    char *setPowerMode;
    char *setWifiMode;
    char *connectWifi;
    char *netStatus;
    char *prepareTcpSend;
    char *ping;
    char *tcpConnect;
    char *mqttSend;
    char *setSendMode;
    char *transparentSend;
    char *exitTransparentSend;
    char *AutoTcpTransparentMode;///
    char mqttConnect[8][50];
}At6266Cmd_t;

/**
 * @brief 保存命令返回的特殊字符
*/
typedef struct eAT6266Ack
{
    char *succeed;
    char *fail;
    char *netStatus;
    char *domain;
    char *dnsFail;
    char *connectOk;
    char *send;
}At6266Ack_t;

typedef enum sAT6266AckType
{
    AT6266ACK_TYPE_SUCCEED,
    AT6266ACK_TYPE_FAIL,
    AT6266ACK_TYPE_SEND,
    AT6266ACK_TYPE_CONNECT,
    AT6266ACK_TYPE_UNKNOW,
}AT6266AckType_t;

At6266Ack_t AT6266Ack={
    .succeed="OK",
    .fail="ERROR",
    .netStatus="STATUS",
    .domain="CIPDOMAIN:",
    .dnsFail="DNS Fail",
    .connectOk="CONNECT",
    .send="<"
};

At6266Cmd_t At6266Cmd={
    .test="AT\r\n",
    .rst="AT+RST\r\n",
    .reset="AT+RESET\r\n",
    .setUart="AT+HSUART_DEF=1152000,8,1,0,0\r\n",
    .setPowerMode="AT+POWER_MODE=0\r\n",
    .setWifiMode="AT+WIFI_MODE_DEF=1\r\n",
    .connectWifi="AT+CONN_AP_CUR=\"R2S-FFF\",\"fff123456_\"\r\n",
    .netStatus="AT+WIFI_STATUS?\r\n",
    .prepareTcpSend="AT+TCPIPSEND=\r\n",
    .ping="AT+DOMAIN=\"WWW.baidu.com\"\r\n",
    //.tcpConnect="AT+TCPIPSTART=\"TCP\",\"ali.kala.love\",5001\r\n",
    .tcpConnect="AT+TCPIPSTART=\"TCP\",\"test.kala.love\",5612\r\n",
    .mqttSend="AT+MQTT_PUB_BODY=\r\n",
    .setSendMode="AT+TCPIPMODE=1\r\n",
    .transparentSend="AT+TCPIPSEND\r\n",
    .exitTransparentSend="+++",
    .AutoTcpTransparentMode="AT+SAVELINK=1,\"test.kala.love\",5612,\"TCP\"",
    .mqttConnect={
        "AT+MQTT_MODULE=0\r\n",
        "AT+AP=R2S-FFF,fff123456_\r\n",
        "AT+SET_DEVICE_NAME=\"FFF\"\r\n",
        "AT+SET_DEVICE_SECRET=\"FFF\"\r\n",
        "AT+SUB=topicdemo\r\n",
        "AT+PUB=topicdemo\r\n",
        "AT+MQTT_HOST=\"broker.emqx.io\"\r\n",
        "AT+MQTT\r\n",
    }
};

/**
 * @brief 测试AT通讯正常与否的命令
*/
char* AT6266Test(char *s)
{
    strcpy(s,At6266Cmd.test);
    return 0;
}

/**
 * @brief 复位WIFI模组命令
*/
char* AT6266Rst(char *s)
{
    s[0]='\0';
    strcpy(s,At6266Cmd.rst);
    return 0;
}

/**
 * @brief 恢复出厂值命令
*/
char* AT6266Reset(char *s)
{
    s[0]='\0';
    strcpy(s,At6266Cmd.reset);
    return 0;
}

/**
 * @brief 设置串口命令
*/
char* AT6266SetUart(char *s,int band)      
{
    s[0]='\0';
    if(band == -1)
    {
        strcpy(s,At6266Cmd.setUart);
        return 0;
    }
    else
    {
        char strBand[8];
        itoa(band,strBand,10);
        strcpy(s,"AT+HSUART_DEF=");
        strcat_m(s,strBand);
        strcat_m(s,"8,1,0,0\r\n");
        return 0;
    }  
}

/**
 * @brief 设置电源模式
 * @mode 
*/
char* AT6266SetPowerMode(char *s,char mode)
{
    s[0]='\0';
    if(mode == -1)
    {
        strcpy(s,At6266Cmd.setPowerMode);
        return 0;
    }
    else
    {
        char strMode[3];
        itoa(mode,strMode,10);
        strcat_m(s,"AT+POWER_MODE=");
        strcat_m(s,strMode);
        strcat_m(s,"\r\n");
        return 0;
    }
}

/**
 * @brief 设置WIFI模式为sta
*/
char* AT6266WifiMode(char *s)
{
    s[0]='\0';
    strcpy(s,At6266Cmd.setWifiMode);
    return 0;
}

/**
 * @brief 连接wifi
*/
char* AT6266ConnectAp(char *s,char *name,char *password)
{
    s[0]='\0';
    if(name ==0)
    {
        strcpy(s,At6266Cmd.connectWifi);
        return 0;
    }
    else
    {
        strcat_m(s,"AT+CONN_AP_CUR=\"");
        strcat_m(s,name);
        strcat_m(s,"\",\"");
        strcat_m(s,password);
        strcat_m(s,"\"\r\n");
        return s;
    }
}

/**
 * @brief 查询网络连接状态
*/
char* AT6266NetStatus(char *s)
{
    s[0]='\0';
    strcpy(s,At6266Cmd.netStatus);
    return 0;
}


char* AT6266TcpConnect(char *s)
{
    s[0]='\0';
    strcpy(s,At6266Cmd.tcpConnect);
    return 0; 
}

/**
 * @brief 先发送此命令，再发送数据
*/
char* AT6266PrepareTcpSend(char *s,unsigned short len)
{
    s[0]='\0';
    char strLen[4];
    itoa(len,strLen,10);
    strcat_m(s,"AT+TCPIPSEND=");
    strcat_m(s,strLen);
    strcat_m(s,"\r\n");
    return s;
}

/**
 * @brief 测试网络连接情况
*/
char *AT6266Ping(char *s,char *host)
{
    s[0]='\0';
    strcat_m(s,"AT+PING=\"");
    strcat_m(s,host);
    strcat_m(s,"\"\r\n");
    return s;
}


/**
 * @brief 设置传输模式为透传
*/
char *AT6266SetMode(char *s)
{
    s[0]='\0';
    strcpy(s,At6266Cmd.setSendMode);
    return 0;
}

/**
 * @brief 返回连接mqtt服务器 订阅主题的命令组，暂时写死
*/
//#include <stdio.h>
char AT6266MqttConnect(char s[8][50])
{
    memcpy(s,At6266Cmd.mqttConnect,8*50);
    //printf("%08X\n",At6266Cmd.MqttConnect);
    return 0;
}

/**
 * @brief 当发送指令后，等待串口接收数据，通过此函数解析
 * @param str 传入接收到的字符串
 * @param rtParam 如果命令需要返回数据，则保存此指针中
 * @return 返回应答的类型
*/
AT6266AckType_t AT6266DecodeAck(char *str,char rtParam)
{
    //查找子串
    if(strstr(str,AT6266Ack.succeed) != NULL)
    {
        return AT6266ACK_TYPE_SUCCEED;
    }
    else if(strstr(str,AT6266Ack.fail) != NULL)
    {
        return AT6266ACK_TYPE_FAIL;
    }
    else if(strstr(str,AT6266Ack.connectOk) !=NULL)
    {
        return AT6266ACK_TYPE_CONNECT;
    }
    else if(strstr(str,AT6266Ack.send) != NULL)
    {   
        return AT6266ACK_TYPE_SEND;
    }
    else
    {
        return AT6266ACK_TYPE_UNKNOW;
    }

}
