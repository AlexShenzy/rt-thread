#include <rtthread.h>
#include <rtdevice.h>

#include "paj7620.h"
#include "hmi_adapter.h"

#define GRS_DEV_NAME            "i2c1"
#define GRS_THREAD_STACK_SIZE   1024
#define GRS_THREAD_PRIORITY     20

static paj7620_device_t grs_dev = RT_NULL;
static struct rt_semaphore grs_rx_sem;
char *gesture_string[] =
{
    "up",
    "down",
    "left",
    "right",
    "forward",
    "backward",
    "clockwise",
    "anticlockwise",
    "wave",
};

static void grs_isr(void *args)
{
    /* �ж����Ž��յ��źţ����ô˻ص�������Ȼ���ͽ����ź��� */
    rt_sem_release(&grs_rx_sem);
}

static void grs_thread_entry(void *parameter)
{
    paj7620_gesture_t gesture;
    
    rt_thread_mdelay(3000);
    grs_dev = paj7620_init(GRS_DEV_NAME);
    if(!grs_dev)
    {
        return;
    }
    
    while(1) {
        rt_sem_take(&grs_rx_sem, RT_WAITING_FOREVER);
        if (paj7620_get_gesture(grs_dev, &gesture) == RT_EOK)
        {
            if (gesture < PAJ7620_GESTURE_NONE)
            {
//                rt_kprintf("Detected gesture: %s\r\n", gesture_string[gesture]);
                if(gesture == PAJ7620_GESTURE_UP) {
                    hmi_page_down();
                }
                else if(gesture == PAJ7620_GESTURE_DOWN) {
                    hmi_page_up();
                }
            }
        }
    }
}


int grs_ctrl_init(void)
{
    /* ���� GRS �����߳� */
    rt_thread_t tid = rt_thread_create("grs", 
                                       (void (*)(void *parameter))grs_thread_entry, 
                                       (void *)RT_NULL, 
                                       GRS_THREAD_STACK_SIZE, GRS_THREAD_PRIORITY, 10);
                                       
    /* ��ʼ���ź��� */
    rt_sem_init(&grs_rx_sem, "grs_rx", 0, RT_IPC_FLAG_FIFO);
    
    /* ��ʼ���ж��ź����� */
    rt_pin_mode(PAJ_INT_PIN, PIN_MODE_INPUT_PULLUP);
    /* ���жϣ��½���ģʽ���ص�������Ϊgrs_isr */
    rt_pin_attach_irq(PAJ_INT_PIN, PIN_IRQ_MODE_FALLING, grs_isr, RT_NULL);
    /* ʹ���ж� */
    rt_pin_irq_enable(PAJ_INT_PIN, PIN_IRQ_ENABLE);
                                       
    /* �����ɹ��������߳� */
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return RT_EOK;
}
INIT_APP_EXPORT(grs_ctrl_init);

