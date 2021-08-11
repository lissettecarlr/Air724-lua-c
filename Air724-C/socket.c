#include "string.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_network.h"
#include "iot_socket.h"
#include "debug.h"

#define SOCKET_MSG_NETWORK_READY (0)
#define SOCKET_MSG_NETWORK_LINKED (1)

typedef enum eSocketStatus{
    NETWORK_READY = 0,
    NETWORK_LINKED,
    SOCKET_CONNECT,
    SOCKET_SEND,
    SOCKET_RECV,
    SOCKET_DELAY,
}SocketStatus_t;

typedef struct {
    SocketStatus_t type;
    UINT32 len;
    VOID *param;
}SOCKET_MESSAGE;

void socket_msg_send(HANDLE hTask, SocketStatus_t status, void *param, UINT32 len)
{
    SOCKET_MESSAGE *msg = NULL;
    msg = (SOCKET_MESSAGE *)iot_os_malloc(sizeof(SOCKET_MESSAGE));
    msg->type = status;
    msg->param = param;
    msg->len = len;
    iot_os_send_message(hTask, msg);
}

// #define DEMO_SERVER_TCP_IP "47.98.213.36"
// #define DEMO_SERVER_TCP_PORT 5001

#define SERVER_TCP_HOST "ali.kala.love"
#define SERVER_TCP_PORT 5001

                        
static HANDLE g_s_socket_task_connect;
static HANDLE g_s_socket_task_send;
static HANDLE g_s_socket_task_rcv;
static int socketHandle=0;

static HANDLE timerSocketReconnect;
static HANDLE timerSocketRcv;



/**
 * @brief socket重连的定时器回调
*/
VOID timerSocketReconnectHandle(void *p)
{
    socket_msg_send(g_s_socket_task_connect,SOCKET_CONNECT,0,0);
}

/**
 * @brief socket定时数据接收
*/
VOID timerSocketRcvHandle(void *p)
{
    socket_msg_send(g_s_socket_task_connect,SOCKET_RECV,0,0);
}

/**
 * @brief 使用TCP发送数据
 * @param data 发送的数据
 * @param len 数据长度
 * @return 返回发送的数据长度
*/
int socketTcpSend(char *data,int len)
{
    if(socketHandle < 0)
        app_debug_print("[socket] socketHandle<0\n");
    int rt = send(socketHandle,data,len,0);
    app_debug_print("[socket] tcp send data result = %d\n", rt);
    if(rt<0)
    {
        int err = socket_errno(socketHandle);
        app_debug_print("[socket] send last error %d\n", err);
    }
    return rt;
}

/**
 * @brief TCP连接
*/
static int demo_socket_tcp_connect_server(char *inputIp,unsigned short inputPort)
//static int demo_socket_tcp_connect_server(void)
{
    int socketfd;
    int connErr;
    struct openat_sockaddr_in tcp_server_addr; 
    
    // 创建tcp socket
    socketfd = socket(OPENAT_AF_INET,OPENAT_SOCK_STREAM,0);
    if (socketfd < 0)
    {
        app_debug_print("[socket] create tcp socket error\n");
        return -1;
    }
       
    app_debug_print("[socket] create tcp socket success\n");
    
    // 建立TCP链接
    memset(&tcp_server_addr, 0, sizeof(tcp_server_addr)); // 初始化服务器地址  
    tcp_server_addr.sin_family = OPENAT_AF_INET;  
    //tcp_server_addr.sin_port = htons((unsigned short)DEMO_SERVER_TCP_PORT);  
    tcp_server_addr.sin_port = htons(inputPort);  
    inet_aton(inputIp,&tcp_server_addr.sin_addr);

    app_debug_print("[socket] tcp connect to addr %s\n", inputIp);
   
 
    connErr = connect(socketfd, (const struct sockaddr *)&tcp_server_addr, sizeof(struct openat_sockaddr));
    if(connErr <0)
    {
        app_debug_print("[socket] tcp connect error %d\n", socket_errno(socketfd));
        return -1
    }
    else
    {
        app_debug_print("[socket] tcp connect success\n");
        return socketfd;
    }
}



/**
 * 域名转IP地址
*/
char* demo_gethostbyname(char *name)
{
    //域名解析
    struct openat_hostent *hostentP = NULL;
    char *ipAddr = NULL;

    //获取域名ip信息
    hostentP = gethostbyname(name);

    if (!hostentP)
    {
        app_debug_print("[socket] gethostbyname %s fail\n", name);
        return 0;
    }
    // 将ip转换成字符串
    ipAddr = ipaddr_ntoa((const openat_ip_addr_t *)hostentP->h_addr_list[0]);
    app_debug_print("[socket] gethostbyname %s ip %s\n", name, ipAddr);
    return ipAddr;

}


static void demo_network_connetck(void)
{
    T_OPENAT_NETWORK_CONNECT networkparam;

    memset(&networkparam, 0, sizeof(T_OPENAT_NETWORK_CONNECT));
    memcpy(networkparam.apn, "CMNET", strlen("CMNET"));

    iot_network_connect(&networkparam);

}

static void demo_networkIndCallBack(E_OPENAT_NETWORK_STATE state)
{
    app_debug_print("[socket] network ind state %d\n", state);
    if(state == OPENAT_NETWORK_LINKED)
    {
        socket_msg_send(g_s_socket_task_connect,NETWORK_LINKED,0,0);
        return;
    }
    else if(state == OPENAT_NETWORK_READY)//回调准备就绪后发送消息
    {
        socket_msg_send(g_s_socket_task_connect,NETWORK_READY,0,0);
        return;
    }
    //iot_os_free(msgptr);
}

/*发送线程*/
static void socket_task_send(PVOID p)
{
    app_debug_print("task send start\n");
    while(1)
    {
        iot_os_sleep(5000);
        if(socketHandle >0)
        {
            char testData[5]={0x11,0x22,0x33,0x44,0x55};
            socketTcpSend(testData,5);
        }
    }
}

/*接收线程*/
static void socket_task_rcv(PVOID p)
{
    app_debug_print("task rcv start\n");
    unsigned char recv_buff[64] = {0};
    int recv_len;

    while(1)
    {
        iot_os_sleep(100);
        if(socketHandle >0)
        {
            recv_len = recv(socketHandle, recv_buff, sizeof(recv_buff), 0);
            if(recv_len == 0)
            {
                app_debug_print("[socket] recv close\n");
            }
            else if(recv_len < 0)
            {
                app_debug_print("[socket] recv error %d\n", socket_errno(socketHandle));
            }
            else
            {
                app_debug_print("[socket] recv data: ");
                for(int i=0;i<recv_len;i++)
                    app_debug_print(" %02X",recv_buff[i]);
                app_debug_print("\n");
            }
        }
    }
}


static void socket_task(PVOID p)
{
    SOCKET_MESSAGE*  msg;
    timerSocketRcv = iot_os_create_timer(timerSocketRcvHandle, 0);
    timerSocketReconnect = iot_os_create_timer(timerSocketReconnectHandle,0);
    while (1)
    {
        iot_os_wait_message(g_s_socket_task_connect, (PVOID)&msg);
        switch (msg->type)
        {
            case NETWORK_READY:
            {
                app_debug_print("[socket] network connecting....\n");
                demo_network_connetck();
            }break;
            case NETWORK_LINKED:
            {
                app_debug_print("[socket] network connected\n");
                socket_msg_send(g_s_socket_task_connect,SOCKET_CONNECT,0,0);
            }break;
            case SOCKET_CONNECT:
            {
                app_debug_print("[socket] start socket connetc\n");				
                //域名解析
                char *testIp = demo_gethostbyname(SERVER_TCP_HOST); 
                socketHandle = demo_socket_tcp_connect_server(testIp,SERVER_TCP_PORT);
                //如果连接失败则延时再次尝试，启动再次连接定时器，超时发送SOCKET_CONNECT
                if(socketHandle<0)
                {
                    iot_os_start_timer(g_demo_timer1, 20*TIMER_1S);
                }
                else
                {
                    //启动数据接收定时器，超时发送SOCKET_RECV

                }
            }
                break;
            case SOCKET_SEND:
                break;
            case SOCKET_RECV:
                break;
            default:
                break;
        }   
    }
}

//extern
void socket_init(void)
{ 
    app_debug_print("[socket] demo_socket_init\n");

    //注册网络状态回调函数
    iot_network_set_cb(demo_networkIndCallBack);

    g_s_socket_task_connect = iot_os_create_task(socket_task,
                        NULL,
                        2046,
                        5,
                        OPENAT_OS_CREATE_DEFAULT,
                        "socket_connect");

    g_s_socket_task_send = iot_os_create_task(socket_task_send,
                        NULL,
                        1024,
                        6,
                        OPENAT_OS_CREATE_DEFAULT,
                        "socket_send");

    g_s_socket_task_rcv = iot_os_create_task(socket_task_rcv,
                        NULL,
                        1024,
                        6,
                        OPENAT_OS_CREATE_DEFAULT,
                        "socket_rcv");

}


