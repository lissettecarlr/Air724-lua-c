#ifndef _ATUART_H_
#define _ATUART_H_

/**
 * @brief AT串口的初始化
 * @return 0表示成功
*/
int atUartInit(void);

/**
 * @brief 发送字符串
 * @param str 待发送的字符串
 * @param ack 是否等待应答
 * @return 0表示成功
*/
int atUartSend(char *str);
/**
 * @brief 绑定数据接收回调
 * @param cb 回调函数
 * @return 0表示成功
*/
int atUartLinkCb( void (*cb)(int status) );

int wifiInitTest();
int tcpSend(char *data,unsigned short len);
#endif