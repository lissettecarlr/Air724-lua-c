/*用于与WIFI模块通信*/

#include <stdio.h>
#include "string.h"
#include "iot_uart.h"
#include "iot_os.h"
#include "debug.h"
#include "AtProtocol_6266.h"

/*线程句柄*/
HANDLE atUartHandle;

/*超时定时器*/
static HANDLE timerRckAck;

/*线程通信载体*/
typedef struct sAtUartMsg
{
    UINT32 id;
    UINT32 len;
    VOID *param;
}AtUartMsg_t;

/*消息类别*/
typedef enum eAtUartMsgId
{
    UART_RECV_MSG = 1,
	UART_SEND_MSG ,
	UART_SEND_VOER_MSG,
	ATUART_TIMEOUT,
	// ATUART_SEND_OK,
	// ATUSRT_SEND_ERROR,
}AtUartMsgId_t;

typedef enum eAtUartStatus
{
	ATUART_RCV,///等待接收状态
	ATUART_NOT_RCV,///不对接收到的数据进行处理
	ATUART_DISCONNECT,
	ATUART_CONNECT,
}AtUartStatus_t;

typedef enum eAtUartMode
{
	ATUART_MODE_NORMAL,
	ATUART_MODE_ACK,
}AtUartMode_t;

/*发送状态标志位，主要用于最外层接口判断发送接收整个流程是否成功*/
// typedef enum eAtUartSendStatus
// {
// 	ATUART_SEND_START=0,
// 	ATUART_SEND_END=1,
// 	ATUART_RCV_OK,
// 	ATUART_RCV_ERROR,
// 	ATUART_TIMEOUT,
// }AtUartSendStatus_t;
// AtUartSendStatus_t AtUartSendStatus;


/**
 * @brief 数据发送的回调函数
 * @param status 发送成功与否，0表示成功
*/
void (*sendCb)(int status);


/**
 * @brief 消息发送
*/
void atUartMsgSend(HANDLE hTask, UINT32 id, void *param, UINT32 len)
{
    AtUartMsg_t *msg = NULL;
    msg = (AtUartMsg_t *)iot_os_malloc(sizeof(AtUartMsg_t));
    msg->id = id;
    msg->param = param;
    msg->len = len;
    iot_os_send_message(hTask, msg);
}

/**
 * @brief 定时器回调
*/
void timerRckAckCallbackFun(void *p)
{
	app_debug_print("timer\n");
	atUartMsgSend(atUartHandle,ATUART_TIMEOUT,0,0);
	//AtUartSendStatus = ATUART_TIMEOUT;
    // AtUartMsg_t *msg = NULL;
    // msg = (AtUartMsg_t *)iot_os_malloc(sizeof(AtUartMsg_t));
    // msg->id = ATUART_TIMEOUT;
    // iot_os_send_message(atUartHandle, msg);	
}

/**
 * @brief 串口绑定的回调函数，目前测试是一帧数据一次中断
*/
void uartCallback(T_AMOPENAT_UART_MESSAGE* evt)
{
	INT8 *recv_buff = NULL;
    int32 recv_len;
    int32 dataLen = evt->param.dataLen;
	//app_debug_print("datalen:%d\n",dataLen);
	if(dataLen)
	{
		recv_buff = iot_os_malloc(dataLen + 1);
		memset(recv_buff, 0, dataLen + 1);
		if(recv_buff == NULL)
		{
			app_debug_print("uart_recv_handle_0 recv_buff malloc fail %d", dataLen);
		}	
		switch(evt->evtId)
		{
			//接收中断
		    case OPENAT_DRV_EVT_UART_RX_DATA_IND: 
				//UART_RECV_TIMEOUT 读取超时时间
		        recv_len = iot_uart_read(OPENAT_UART_1, (UINT8*)recv_buff, dataLen , 1000);
				//app_debug_init("");
		        //app_debug_print("uart_recv_handle_1:recv_len %d", recv_len);
				atUartMsgSend(atUartHandle, UART_RECV_MSG, recv_buff, recv_len);
		        break;
			//发送中断
		    case OPENAT_DRV_EVT_UART_TX_DONE_IND:
		        //发送成功
				//atUartMsgSend(atUartHandle, UART_SEND_VOER_MSG, 0, 0);
				app_debug_print("send ok\n");
				//app_debug_print("uart_recv_handle_2 OPENAT_DRV_EVT_UART_TX_DONE_IND");

		        break;
		    default:
		        break;
		}
	}
}

/**
 * @brief 串口初始化
 * @return 返回0表示成功
*/
static int uartOpen(void)
{
    BOOL err;
    T_AMOPENAT_UART_PARAM uartCfg;
    
    memset(&uartCfg, 0, sizeof(T_AMOPENAT_UART_PARAM));
    uartCfg.baud = OPENAT_UART_BAUD_921600; //波特率
    uartCfg.dataBits = 8;   //数据位
    uartCfg.stopBits = 1; // 停止位
    uartCfg.parity = OPENAT_UART_NO_PARITY; // 无校验
    uartCfg.flowControl = OPENAT_UART_FLOWCONTROL_NONE; //无流控
    uartCfg.txDoneReport = TRUE; // 设置TURE可以在回调函数中收到OPENAT_DRV_EVT_UART_TX_DONE_IND
    uartCfg.uartMsgHande = uartCallback; //回调函数

    // 配置uart1 使用中断方式读数据
    err = iot_uart_open(OPENAT_UART_1, &uartCfg);
	if(err == TRUE)
		return 0;
	else
		return -1;
}

/**
 * @brief at串口的线程
*/
static void atUartTask(PVOID pParameter)
{
	AtUartMsg_t *msg = NULL;
	/*是否需要接收数据标志*/
	AtUartStatus_t status = ATUART_NOT_RCV;
	/*是否等待应答标志*/
	//AtUartMode_t mode = ATUART_MODE_NORMAL;
	AtUartMode_t mode = ATUART_MODE_ACK;
	while(1)
	{
		iot_os_wait_message(atUartHandle, (PVOID*)&msg);
		switch(msg->id)
	    {
	        case UART_RECV_MSG: //如果是接收数据消息，则打印
			{
				if(status == ATUART_RCV)
				{
					/*判断数据类型*/
					if(AT6266DecodeAck(msg->param,0) == AT6266ACK_TYPE_SUCCEED)
					{
						app_debug_print("[uart]recv send ack is ok\n");
						//AtUartSendStatus = ATUART_RCV_OK;
						iot_os_stop_timer(timerRckAck);
						sendCb(0);
						status = ATUART_NOT_RCV;
					}
					else if(AT6266DecodeAck(msg->param,0)  == AT6266ACK_TYPE_FAIL)
					{
						app_debug_print("[uart]recv send ack is error\n");
						//AtUartSendStatus = ATUART_RCV_ERROR;
						iot_os_stop_timer(timerRckAck);
						sendCb(-1);
						status = ATUART_NOT_RCV;
					}
					else if(AT6266DecodeAck(msg->param,0)  == AT6266ACK_TYPE_CONNECT)
					{
						app_debug_print("[uart]recv send ack is connect\n");
						iot_os_stop_timer(timerRckAck);
						status = ATUART_NOT_RCV;
					}
					else
					{
						app_debug_print("[uart]recv send ack is other\n");
					}
				}
				app_debug_print("[uart] recv data:  %s\n", msg->param);

				if(msg->param)
				{
					iot_os_free(msg->param);
					msg->param = NULL;
				}
			}
	        	break;
			case UART_SEND_MSG:
			{
				// int32 write_len;
				//write_len = iot_uart_write(OPENAT_UART_1, (UINT8*)(msg->param), msg->len);
				iot_uart_write(OPENAT_UART_1, (UINT8*)(msg->param), msg->len);
			}
			//发送完毕
			case UART_SEND_VOER_MSG:
			{
				app_debug_print("[uart] send over\n");
				//如果不需要等待应答则直接出发回调，发送完成
				if(mode == ATUART_MODE_NORMAL)
				{
					status = ATUART_NOT_RCV;
					sendCb(0);
				}
				else
				{
					status = ATUART_RCV;
					iot_os_start_timer(timerRckAck, 5000);
				}	
			}
			break;
			case ATUART_TIMEOUT:
			{
				status = ATUART_NOT_RCV;
				sendCb(-2);
			}
			break;
	        default:
	            break;
	    }

	    if(msg)
	    {
	        iot_os_free(msg);
	        msg = NULL;
	    }
	}
}

/*对外接口*/
/*********************************************************************************************/

/**
 * @brief AT串口的初始化
 * @return 0表示成功
*/
int atUartInit(void)
{
	app_debug_print("[uart] atUartInit");
	if(uartOpen() != 0)
	{
		app_debug_print("[uart] init error\n");
		return -1;
	}
	app_debug_print("[uart] init , start task\n");
	timerRckAck = iot_os_create_timer(timerRckAckCallbackFun, 0);
	atUartHandle = iot_os_create_task(atUartTask, NULL, 4096, 10, OPENAT_OS_CREATE_DEFAULT, "uart_task");
	return 0;
}

/**
 * @brief 发送字符串
 * @param str 待发送的字符串
 * @param ack 是否等待应答
 * @return 0表示成功 -1表示超时
*/
int atUartSend(char *str)
{
	atUartMsgSend(atUartHandle,UART_SEND_MSG,str,strlen(str));
	app_debug_print("[uart] sending len %d, buff %s\n", strlen(str), str);
	return 0;
}

int atUartSendHex(unsigned char *data,unsigned short len)
{
	iot_uart_write(OPENAT_UART_1, (UINT8*)(data), len);
	return 0;
}

/**
 * @brief 绑定数据接收回调
 * @param cb 回调函数
 * @return 0表示成功
*/
int atUartLinkCb( void (*cb)(int status) )
{
	sendCb = cb;
	return 0;
}

/*WIFI模组相关*/
/******************************************************************/
volatile int sendStatusFlag = 1;
int tcpConnect=0;

void sendCbTest(int s)
{
	app_debug_print("send status = %d\n",s);
	sendStatusFlag = s;
}

int wifiInitTest()
{
	atUartLinkCb(sendCbTest);
//	char cmd[40];
	/*check module*/
	// AT6266Test(cmd);
	// atUartSend(cmd);
	// iot_os_sleep(1000);
	// if(sendStatusFlag >0) return -1;
	
	// /*link wifi*/
	// AT6266ConnectAp(cmd,0,0);
	// atUartSend(cmd);
	// iot_os_sleep(5000);
	// if(sendStatusFlag >0) return -1;	

	// /*link tcp*/
	// AT6266TcpConnect(cmd);
	// atUartSend(cmd);
	// iot_os_sleep(5000);
	// if(sendStatusFlag >0) return -1;	

	tcpConnect =1;
	return 0;
}

int tcpSend(unsigned char *data,unsigned short len)
{
	// if(tcpConnect !=1)
	// {
	// 	app_debug_print("[tcpSend],tcp disconnect\n");
	// 	return -1;
	// }
	// char cmd[40];
	//先发送 准备命令
	//AT6266PrepareTcpSend(cmd,len);
	//atUartSend(cmd);
	//iot_uart_write(OPENAT_UART_1, (UINT8*)(cmd), strlen(cmd));
	//iot_os_sleep(50);
	// sendStatusFlag =1;
	// while(1)
	// {
	// 	if(sendStatusFlag!=1)
	// 	{
	// 		break;
	// 	}
	// 	iot_os_sleep(1);
	// }
	atUartSendHex(data,len);
	iot_os_sleep(100);//WIFI 透传模式下需要20MS分割两个数据包
	// while(1)
	// {
	// 	iot_os_sleep(1);
	// 	if()
	// }	
	return 0;
}