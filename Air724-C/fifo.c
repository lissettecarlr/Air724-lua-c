#include "fifo.h"
#include "iot_os.h"
#include "string.h"
/*!
 * \brief 存入数据
 * \param [IN]fifo 执行的队列
 * \param [IN]data 需要存入的数据
 */
int fifoPush(Fifo_t *fifo,void *data)
{
    if(isFifoFull(fifo) == 1)
    {
        //此宏用于启动队列满时新数据挤掉旧数据的功能
#ifdef RING_OUT_THE_OLD
        //定义一个局部变量来去旧数据
        unsigned short temp;
        fifoPop(fifo,&temp);
#endif        
    }

    if(fifo->type == DATA_TYPE_U8)
    {
       ((unsigned char *)(fifo->data))[fifo->posW] = *(unsigned char*)(data);
    }
    else if(fifo->type == DATA_TYPE_U16)
    {   
       ((unsigned short *)(fifo->data))[fifo->posW] = *(unsigned short*)(data);
    }
    else if(fifo->type == DATA_TYPE_POINTER) 
    {
        ((unsigned int *)(fifo->data))[fifo->posW] = (unsigned int)data; //直接存入指针
    }
    else
    {
        return -1;
    }
    //更新下一次存储位置
    fifo->posW = (fifo->posW+1)%fifo->size;
    return 0;
}


/*!
 * \brief 取出数据
 * \param [IN]  fifo 执行的队列
 * \param [OUT] data 取出的数据,当传入为0时表示抛弃一个数据
 */
int fifoPop(Fifo_t *fifo,void *data)
{
    if( isFifoEmpty(fifo) == 1 )//为空
    {
        return -1;
    }

    if(fifo->type == DATA_TYPE_U8)
    {
        *(unsigned char *)data = ((unsigned char*)(fifo->data))[fifo->posR];
    }
    else if(fifo->type == DATA_TYPE_U16)
    {
        *(unsigned short*)data = ((unsigned short*)(fifo->data))[fifo->posR];
    }
    else if(fifo->type == DATA_TYPE_POINTER)
    {
        //需要传入一个指向指针的指针
        *((unsigned int*)data)=((unsigned int*)(fifo->data))[fifo->posR];
    }
    else
    {
        //类型错误
        return -1;
    }
    fifo->posR = (fifo->posR+1) % fifo->size;
    return 0;
}

/*!
 * \brief 初始化队列
 * \param [IN] fifo 执行的队列
 * \param [IN] data 用于存储的区域
 * \param [IN] size 存储区域大小
 * \param [IN] type 存储数据类型
 */
void fifoInit(Fifo_t *fifo,void *data,unsigned int size,DataType_t type)
{
    fifo->data = data;
    fifo->size = size;
    fifo->type = type;
    fifo->posR = 0;
    fifo->posW = 0;
}

/*!
 * \brief 清空队列
 * \param [IN] fifo 执行的队列
 */
void fifoClear(Fifo_t *fifo)
{
    fifo->posR = 0;
    fifo->posW = 0;
}

/*!
 * \brief 获取队列长度
 * \param [IN] fifo 执行的队列
 */
unsigned int fifoLenth(Fifo_t *fifo)
{
    unsigned int len;
    len = (fifo->posW + fifo->size - fifo->posR) % fifo->size;
    return len;
}


/*************/

typedef struct sfifoTrack
{
    unsigned short len;
    unsigned char data[0];
}fifoTrack_t;

#define FIFO_BUFFER_SIZE 4096
unsigned char TrackBufferStore[FIFO_BUFFER_SIZE];  
void TrackFifoInit(void)
{
    fifoInit(&TrackBuffer,TrackBufferStore,FIFO_BUFFER_SIZE,DATA_TYPE_POINTER);
}

unsigned int getTrackFifoLen()
{
    return fifoLenth(&TrackBuffer);
}

void inputTrack(unsigned char *data,unsigned short len)
{
    fifoTrack_t* InData = (fifoTrack_t *)iot_os_malloc(sizeof(fifoTrack_t)+len);
    InData ->len = len;
    memcpy(InData->data,data,len);
    fifoPush(&TrackBuffer,InData);
}

unsigned short outputTrack(unsigned char* data)
{
    fifoTrack_t *OutData;
    fifoPop(&TrackBuffer,&OutData);
    unsigned short len = OutData->len;
    memcpy(data,OutData->data,len);
    iot_os_free(OutData);
    return len;
}


/*
        inputTrack(data1,3);
        inputTrack(data2,3);
        app_debug_print("fifo len:%d\n",getTrackFifoLen());

        len = outputTrack(data3);
        app_debug_print("fifo data:%02X %02X %02X\n",data3[0],data3[1],data3[2]);
        len = outputTrack(data3);
        app_debug_print("fifo data:%02X %02X %02X\n",data3[0],data3[1],data3[2]);
*/