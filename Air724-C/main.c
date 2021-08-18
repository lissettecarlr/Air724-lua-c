#include "iot_debug.h"
#include "iot_os.h"
#include "stdbool.h"
#include "debug.h"
#include "stdlib.h"
#include "stdio.h"
#include "iot_pmd.h"
#include "fifo.h"
#include "atUart.h"
#include "string.h"

HANDLE main_task_handle=NULL;
extern void bleStart(void);
//extern void socket_init(void);
//extern int atUartInit(void);

// void sendcallback(int s)
// {
//     app_debug_print("send status = %d\n",s);
// }

/*心跳包定时器*/
static HANDLE timerHeartbeat;

unsigned char heartbeat[8]={0XF0,0XAD,0xD8,0X0B,0XCB,0X61,0X02,0XE7};

/*暴力测试*/
#include "iot_uart.h"
void testUartInit()
{
    T_AMOPENAT_UART_PARAM uartCfg;
    memset(&uartCfg, 0, sizeof(T_AMOPENAT_UART_PARAM));
    uartCfg.baud = OPENAT_UART_BAUD_115200; //波特率
    uartCfg.dataBits = 8;   //数据位
    uartCfg.stopBits = 1; // 停止位
    uartCfg.parity = OPENAT_UART_NO_PARITY; // 无校验
    uartCfg.flowControl = OPENAT_UART_FLOWCONTROL_NONE; //无流控
    iot_uart_open(OPENAT_UART_1,&uartCfg);
}

void testUartSend(unsigned char* data,unsigned short len)
{
    HANDLE lock=iot_os_enter_critical_section();
    iot_uart_write(OPENAT_UART_1,(UINT8 *)data,len);
    iot_os_exit_critical_section(lock);
}

/**
 * 每20s发送一次心跳包
*/
void timerHeartbeatCallback(void *p)
{
    //inputTrack(heartbeat,8);
    //testUartSend(heartbeat,8);
    iot_os_start_timer(timerHeartbeat,20*1000);
}
//分包错误


// F0 AA 01 00 01 D8 0B CB 61 02 E7 00 3C FE 01 A0 00 8E 52 9C 01 98 D4 FE 02 19 C4 26 48 56 A6 DF 15 
// FC 09 A0 02 78 00 11 C4 17 49 FC 09 A0 02 80 00 19 CC 18 4A 
// FC 09 A0 02 88 00 09 C4 13 4B FC 0A A0 02 78 00 01 C8 05 4C 

// F0 AA 01 00 01 D8 0B CB 61 02 E7 00 1E FC 08 A0 02 68 07 E1 C8 00 4D 
// FC 0A A0 01 80 06 E9 C4 00 4E FC 0C A0 06 90 04 B1 BC 00 4F 

// unsigned char a[10]={0XFC,0X09,0XA4,0X0A,0X20,0X11,0XEB,0XC4,0X24,0XA9};
// unsigned char b[10]={0XFC,0X0A,0XA4,0X0A,0X60,0X13,0X3B,0XD4,0X25,0XAA};
// unsigned char c[10]={0XFC,0X0B,0XA4,0X0A,0X78,0X0C,0X3B,0XD0,0X36,0XAB};

static void main_task(PVOID pParameter)
{
    unsigned int len=0;
    unsigned int pckLen=0;
    unsigned char tempData[200];
    unsigned char pckData[200]={};

    pckData[0] = 0xF0;
    pckData[1] = 0XAA;
    pckData[2] = 0X01;
    pckData[3] = 0X00;
    pckData[4] = 0X01;

    unsigned char mac[6]={0xD8,0X0B,0XCB,0X61,0X02,0XE7};
    memcpy(pckData+5,mac,6);
    /*初始化WIFI模块*/
    //wifiInitTest();
    //app_debug_print("wifi:%d\n",wifiStatus);

    //等待WIFI初始化完成
    iot_os_sleep(10000);
    app_debug_print("wifi is ok\n");

    /*每20s发送一次心跳*/
    
    iot_os_start_timer(timerHeartbeat,20*1000);
    /**/
    // inputTrack(a,10);
    // inputTrack(b,10);
    // inputTrack(c,10);
    // app_debug_print("len=%d\n",getTrackFifoLen());
    //iot_os_sleep(10000);

    while(true)
    {
        // iot_os_sleep(30000);
        // app_debug_print("------------");
        if(getTrackFifoLen()>0)
        {
            len=outputTrack(tempData);
            //app_debug_print("[output] len=%d\n",len);
            app_debug_print("fifo output size=%d\n",getTrackFifoLen());
            // for(int i=0;i<len;i++)
            // {
            //     app_debug_print(" %02X",tempData[i]);
            // }
            // app_debug_print("\n");
            pckData[12] = (unsigned char)len;
            memcpy(pckData+13,tempData,len);
            pckLen = len + 13;
            for(int i=0;i<pckLen;i++)
            {
                app_debug_print(" %02X",pckData[i]);
            }
            app_debug_print("\n");
            testUartSend(pckData,pckLen);
            iot_os_sleep(80);

            //tcpSend(pckData,pckLen);
        }
        else
        {
            iot_os_sleep(10);
        }
    }
    iot_os_delete_task(main_task_handle);
}

int appimg_enter(void *param)
{
#if CONFIG_APP_DEBUG == 1
    iot_debug_set_fault_mode(OPENAT_FAULT_HANG);
#endif // CONFIG_APP_DEBUG
    iot_pmd_exit_deepsleep();
    app_debug_init();
    TrackFifoInit();
    //atUartInit();
    testUartInit();
    timerHeartbeat = iot_os_create_timer(timerHeartbeatCallback, 0);
    main_task_handle = iot_os_create_task(main_task, NULL, 2046, 10, OPENAT_OS_CREATE_DEFAULT, "main");
    bleStart();
    //socket_init();
    
    return 0;
}
void appimg_exit(void)
{
    app_debug_print("exit");
}
