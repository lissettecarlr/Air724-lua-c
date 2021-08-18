#include "iot_os.h"
#include "stdbool.h"
#include "debug.h"
#include "stdlib.h"
#include "stdio.h"
#include "atUart.h"

HANDLE wifiTaskHandle=NULL;


/*线程通信载体*/
typedef struct sAtUartMsg
{
    UINT32 id;
    UINT32 len;
    VOID *param;
}AtUartMsg_t;

/*消息类别*/
typedef enum eWifiMsgId
{
    WIFI_SEND_ACK,
}WifiMsgId_t;

/**
 * @brief 消息发送
*/
void wifiMsgSend(HANDLE hTask, UINT32 id, void *param, UINT32 len)
{
    AtUartMsg_t *msg = NULL;
    msg = (AtUartMsg_t *)iot_os_malloc(sizeof(AtUartMsg_t));
    msg->id = id;
    msg->param = param;
    msg->len = len;
    iot_os_send_message(hTask, msg);
}

void wifiSendAckCallback(int status)
{
    app_debug_print("send status = %d\n",status);
    wifiMsgSend(wifiTaskHandle,WIFI_SEND_ACK,0,status);

}

static void wifiTask(PVOID pParameter)
{
    AtUartMsg_t *msg=NULL;
    iot_os_wait_message(wifiTaskHandle, (PVOID*)&msg);
    while(1)
    {
        switch (msg->id)
        {
            case WIFI_SEND_ACK:
            {
                
            }
            break;
        
            default:
            break;
        }
        iot_os_sleep(10000);
        app_debug_print("------------");
        atUartSend("1234567890");
    }
}

int wifiInit(void)
{
    int rt=0;
    rt = atUartInit();
    atUartLinkCb(wifiSendAckCallback);
    wifiTaskHandle = iot_os_create_task(wifiTask, NULL, 1024, 10, OPENAT_OS_CREATE_DEFAULT, "wifiTask");
}