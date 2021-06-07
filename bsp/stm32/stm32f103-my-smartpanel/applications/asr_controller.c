#include <rtthread.h>
#include <rtdevice.h>

#include "communication.h"

#define ASR_THREAD_STACK_SIZE   1024
#define ASR_THREAD_PRIORITY     20

static struct rt_semaphore asr_wkup_sem;

static void asr_wakeup_isr(void *args)
{
    /* �ж����Ž��յ��źţ����ô˻ص�������Ȼ���ͽ����ź��� */
    rt_sem_release(&asr_wkup_sem);
}

static void asr_thread_entry(void *parameter)
{
    int s1,s2,s3;
    while(1) {
        rt_sem_take(&asr_wkup_sem, RT_WAITING_FOREVER);
        rt_thread_mdelay(100);
        
        s1 = rt_pin_read(ASR_S1_PIN);
        s2 = rt_pin_read(ASR_S2_PIN);
        s3 = rt_pin_read(ASR_S3_PIN);
        
        if(s1 == PIN_HIGH && s2 == PIN_LOW && s3 == PIN_LOW) {
            // �򿪷���
            char cmd[64] = "";
            rt_snprintf(cmd, sizeof(cmd), "{\"Motor1_Angle\":%d,\"Motor2_Angle\":%d}\r\n", 90, 90);
            com_send(cmd,rt_strlen(cmd));
        }
        else if(s1 == PIN_LOW && s2 == PIN_HIGH && s3 == PIN_LOW) {
            // ���Ű뿪
            char cmd[64] = "";
            rt_snprintf(cmd, sizeof(cmd), "{\"Motor1_Angle\":%d,\"Motor2_Angle\":%d}\r\n", 135, 135);
            com_send(cmd,rt_strlen(cmd));
        }
        else if(s1 == PIN_LOW && s2 == PIN_LOW && s3 == PIN_HIGH) {
            // �رշ���
            char cmd[64] = "";
            rt_snprintf(cmd, sizeof(cmd), "{\"Motor1_Angle\":%d,\"Motor2_Angle\":%d}\r\n", 180, 180);
            com_send(cmd,rt_strlen(cmd));
        }
        
    }
}    

int asr_ctrl_init(void)
{
    /* ���� ASR �����߳� */
    rt_thread_t tid = rt_thread_create("asr", 
                                       (void (*)(void *parameter))asr_thread_entry, 
                                       (void *)RT_NULL, 
                                       ASR_THREAD_STACK_SIZE, ASR_THREAD_PRIORITY, 10);
                                                         
    /* ��ʼ���ź��� */
    rt_sem_init(&asr_wkup_sem, "asr_wkup", 0, RT_IPC_FLAG_FIFO);
    
    /* ��ʼ���ź����� */
    rt_pin_mode(ASR_S1_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(ASR_S2_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(ASR_S3_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(ASR_S4_PIN, PIN_MODE_INPUT_PULLUP);
    /* ���жϣ��½���ģʽ���ص�������Ϊgrs_isr */
    rt_pin_attach_irq(ASR_S4_PIN, PIN_IRQ_MODE_FALLING, asr_wakeup_isr, RT_NULL);
    /* ʹ���ж� */
    rt_pin_irq_enable(ASR_S4_PIN, PIN_IRQ_ENABLE);
                                       
    /* �����ɹ��������߳� */
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return RT_EOK;
}
INIT_APP_EXPORT(asr_ctrl_init);