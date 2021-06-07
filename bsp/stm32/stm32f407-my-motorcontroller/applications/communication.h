#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include <rtthread.h>
#include <rtdevice.h>

#ifndef COM_DEV_NAME
#define COM_DEV_NAME                "uart3"     /* COM �豸���� */
#endif
#ifndef COM_RX_THREAD_PRIORITY
#define COM_RX_THREAD_PRIORITY      25          /* COM �����߳����ȼ� */
#endif
#ifndef COM_RX_THREAD_STACK_SIZE
#define COM_RX_THREAD_STACK_SIZE    2048        /* COM �����̶߳�ջ��С */
#endif
#ifndef COM_MAX_LEN
#define COM_MAX_LEN                 512         /* COM ������󻺳��С */
#endif

typedef struct my_com_device
{
    rt_device_t             device;
    struct rt_semaphore     rx_sem;
    
    char                    buffer[COM_MAX_LEN + 1];
    rt_uint16_t             position;
    
    int (*recv_callback)(char *data, rt_size_t length);
} my_com_t;

int commuication_init(void);
void com_set_recv_callback(int (*callback)(char *data, rt_size_t length));
void com_send(char *data, rt_size_t length);

#endif
