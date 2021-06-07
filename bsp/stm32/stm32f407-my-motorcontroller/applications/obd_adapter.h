#ifndef __OBD_ADAPTER_H__
#define __OBD_ADAPTER_H__

#include <rtthread.h>
#include <rtdevice.h>

#include "obd_pids.h"

#ifndef OBD_DEV_NAME
#define OBD_DEV_NAME                "can1"      /* OBD �豸���� */
#endif
#ifndef OBD_UP_THREAD_PRIORITY
#define OBD_UP_THREAD_PRIORITY      25          /* OBD �����߳����ȼ� */
#endif
#ifndef OBD_UP_THREAD_STACK_SIZE
#define OBD_UP_THREAD_STACK_SIZE    2048        /* OBD �����̶߳�ջ��С */
#endif

typedef struct my_obd_device
{
    rt_device_t             device;
    struct rt_semaphore     rx_sem;
    
} my_obd_t;


int obd_init(void);
rt_int32_t obd_query_pid(rt_uint8_t pid);
#endif
