#ifndef __FIFO_H__
#define __FIFO_H__

/*! 
 * \brief 当数据存满后是否抛弃旧数据存入新数据
 */
#define RING_OUT_THE_OLD

/*!
 * \brief 判断队列是否为空
 */
#define isFifoEmpty( fifo )                     ((fifo)->posR == (fifo)->posW)

/*!
 * \brief 判断队列是否已经满
 */
#define isFifoFull( fifo )                      ((((fifo)->posW+1) % (fifo)->size) == (fifo)->posR)



/*!
 * \brief 队列数据类型
 */
typedef enum 
{
    DATA_TYPE_U8=1,
    DATA_TYPE_U16=2,
    DATA_TYPE_POINTER=4,
}DataType_t;

/*!
 * \brief 队列结构体
 */
typedef struct sFifo
{
    void *data; //数据存储区
    unsigned int size;//数据存储区大小
    unsigned int posR;//读地址
    unsigned int posW;//写地址
    DataType_t type;//存储区数据类型
}Fifo_t;

/*!
 * \brief 存入数据
 * \param [IN]fifo 执行的队列
 * \param [IN]data 需要存入的数据
 */
int fifoPush(Fifo_t *fifo,void *data);


/*!
 * \brief 取出数据
 * \param [IN]  fifo 执行的队列
 * \param [OUT] data 取出的数据
 */
int fifoPop(Fifo_t *fifo,void *data);

/*!
 * \brief 初始化队列，注意，由于要区分空和满，所以实际使用空间为传入size-1
 * \param [IN] fifo 执行的队列
 * \param [IN] data 用于存储的区域
 * \param [IN] size 存储区域大小
 * \param [IN] type 存储数据类型
 */
void fifoInit(Fifo_t *fifo,void *data,unsigned int size,DataType_t type);

/*!
 * \brief 清空队列
 * \param [IN] fifo 执行的队列
 */
void fifoClear(Fifo_t *fifo);

/*!
 * \brief 获取队列长度
 * \param [IN] fifo 执行的队列
 */
unsigned int fifoLenth(Fifo_t *fifo);


/********************/
Fifo_t TrackBuffer;

void TrackFifoInit(void);
unsigned int getTrackFifoLen();
void inputTrack(unsigned char *data,unsigned short len);
unsigned short outputTrack(unsigned char* data);
#endif


