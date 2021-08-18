/*������WIFIģ��ͨ��*/

#include <stdio.h>
#include "string.h"
#include "iot_uart.h"
#include "iot_os.h"
#include "debug.h"
#include "AtProtocol_6266.h"

/*�߳̾��*/
HANDLE atUartHandle;

/*��ʱ��ʱ��*/
static HANDLE timerRckAck;

/*�߳�ͨ������*/
typedef struct sAtUartMsg
{
    UINT32 id;
    UINT32 len;
    VOID *param;
}AtUartMsg_t;

/*��Ϣ���*/
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
	ATUART_RCV,///�ȴ�����״̬
	ATUART_NOT_RCV,///���Խ��յ������ݽ��д���
	ATUART_DISCONNECT,
	ATUART_CONNECT,
}AtUartStatus_t;

typedef enum eAtUartMode
{
	ATUART_MODE_NORMAL,
	ATUART_MODE_ACK,
}AtUartMode_t;

/*����״̬��־λ����Ҫ���������ӿ��жϷ��ͽ������������Ƿ�ɹ�*/
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
 * @brief ���ݷ��͵Ļص�����
 * @param status ���ͳɹ����0��ʾ�ɹ�
*/
void (*sendCb)(int status);


/**
 * @brief ��Ϣ����
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
 * @brief ��ʱ���ص�
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
 * @brief ���ڰ󶨵Ļص�������Ŀǰ������һ֡����һ���ж�
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
			//�����ж�
		    case OPENAT_DRV_EVT_UART_RX_DATA_IND: 
				//UART_RECV_TIMEOUT ��ȡ��ʱʱ��
		        recv_len = iot_uart_read(OPENAT_UART_1, (UINT8*)recv_buff, dataLen , 1000);
				//app_debug_init("");
		        //app_debug_print("uart_recv_handle_1:recv_len %d", recv_len);
				atUartMsgSend(atUartHandle, UART_RECV_MSG, recv_buff, recv_len);
		        break;
			//�����ж�
		    case OPENAT_DRV_EVT_UART_TX_DONE_IND:
		        //���ͳɹ�
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
 * @brief ���ڳ�ʼ��
 * @return ����0��ʾ�ɹ�
*/
static int uartOpen(void)
{
    BOOL err;
    T_AMOPENAT_UART_PARAM uartCfg;
    
    memset(&uartCfg, 0, sizeof(T_AMOPENAT_UART_PARAM));
    uartCfg.baud = OPENAT_UART_BAUD_921600; //������
    uartCfg.dataBits = 8;   //����λ
    uartCfg.stopBits = 1; // ֹͣλ
    uartCfg.parity = OPENAT_UART_NO_PARITY; // ��У��
    uartCfg.flowControl = OPENAT_UART_FLOWCONTROL_NONE; //������
    uartCfg.txDoneReport = TRUE; // ����TURE�����ڻص��������յ�OPENAT_DRV_EVT_UART_TX_DONE_IND
    uartCfg.uartMsgHande = uartCallback; //�ص�����

    // ����uart1 ʹ���жϷ�ʽ������
    err = iot_uart_open(OPENAT_UART_1, &uartCfg);
	if(err == TRUE)
		return 0;
	else
		return -1;
}

/**
 * @brief at���ڵ��߳�
*/
static void atUartTask(PVOID pParameter)
{
	AtUartMsg_t *msg = NULL;
	/*�Ƿ���Ҫ�������ݱ�־*/
	AtUartStatus_t status = ATUART_NOT_RCV;
	/*�Ƿ�ȴ�Ӧ���־*/
	//AtUartMode_t mode = ATUART_MODE_NORMAL;
	AtUartMode_t mode = ATUART_MODE_ACK;
	while(1)
	{
		iot_os_wait_message(atUartHandle, (PVOID*)&msg);
		switch(msg->id)
	    {
	        case UART_RECV_MSG: //����ǽ���������Ϣ�����ӡ
			{
				if(status == ATUART_RCV)
				{
					/*�ж���������*/
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
			//�������
			case UART_SEND_VOER_MSG:
			{
				app_debug_print("[uart] send over\n");
				//�������Ҫ�ȴ�Ӧ����ֱ�ӳ����ص����������
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

/*����ӿ�*/
/*********************************************************************************************/

/**
 * @brief AT���ڵĳ�ʼ��
 * @return 0��ʾ�ɹ�
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
 * @brief �����ַ���
 * @param str �����͵��ַ���
 * @param ack �Ƿ�ȴ�Ӧ��
 * @return 0��ʾ�ɹ� -1��ʾ��ʱ
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
 * @brief �����ݽ��ջص�
 * @param cb �ص�����
 * @return 0��ʾ�ɹ�
*/
int atUartLinkCb( void (*cb)(int status) )
{
	sendCb = cb;
	return 0;
}

/*WIFIģ�����*/
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
	//�ȷ��� ׼������
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
	iot_os_sleep(100);//WIFI ͸��ģʽ����Ҫ20MS�ָ��������ݰ�
	// while(1)
	// {
	// 	iot_os_sleep(1);
	// 	if()
	// }	
	return 0;
}