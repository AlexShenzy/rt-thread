#ifndef __HMI_ADAPTER_H__
#define __HMI_ADAPTER_H__

#include <rtthread.h>
#include <rtdevice.h>

#ifndef HMI_DEV_NAME
#define HMI_DEV_NAME                "uart2"     /* HMI 设备名称 */
#endif
#ifndef HMI_RX_THREAD_PRIORITY
#define HMI_RX_THREAD_PRIORITY      25          /* HMI 接收线程优先级 */
#endif
#ifndef HMI_RX_THREAD_STACK_SIZE
#define HMI_RX_THREAD_STACK_SIZE    2048        /* HMI 接收线程堆栈大小 */
#endif
#ifndef HMI_MAX_LEN
#define HMI_MAX_LEN                 512         /* HMI 接收最大缓冲大小 */
#endif

typedef struct my_hmi_device
{
    rt_device_t             device;
    struct rt_semaphore     rx_sem;
    
    char                    buffer[HMI_MAX_LEN + 1];
    rt_uint16_t             position;
    
    rt_uint8_t              page_id;
    rt_uint8_t              req_sta;
    rt_uint8_t              req_page_id;
    rt_uint8_t              req_ctl_id;
    
} my_hmi_t;

int hmi_adapter_init(void);
void hmi_send(char *cmd, rt_size_t length);
void hmi_page_up(void);
void hmi_page_down(void);
#endif
