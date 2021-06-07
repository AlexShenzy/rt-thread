#include <string.h>

#include "communication.h"

static my_com_t *com;


/* �������ݻص����� */
static rt_err_t com_rx_call(rt_device_t dev, rt_size_t size)
{
    /* ���ڽ��յ����ݺ�����жϣ����ô˻ص�������Ȼ���ͽ����ź��� */
    rt_sem_release(&com->rx_sem);

    return RT_EOK;
}

static void com_rx_thread_entry(void *parameter)
{
    char ch;
    char uart_name[RT_NAME_MAX];
    
    rt_strncpy(uart_name, COM_DEV_NAME, RT_NAME_MAX);
    
    /* ����ϵͳ�еĴ����豸 */
    if(com->device == RT_NULL)
    {
        com->device = rt_device_find(uart_name);
        if (com->device == RT_NULL)
        {
            rt_kprintf("find %s failed!\n", uart_name);
            return;
        }
    }

    /* ���жϽ��ռ���ѯ����ģʽ�򿪴����豸 */
    rt_device_open(com->device, RT_DEVICE_FLAG_INT_RX);
    
    /* ���ý��ջص����� */
    rt_device_set_rx_indicate(com->device, com_rx_call);
    
    while (1) {
        /* �Ӵ��������ȴ���ȡһ������ */
        while (rt_device_read(com->device, -1, &ch, 1) != 1)
            rt_sem_take(&com->rx_sem, RT_WAITING_FOREVER);
        
        /* received null or error */
        if (ch == '\0' || ch == 0xFF) continue;
        
        /* handle end of line, break */
        if (ch == '\n' && com->buffer[com->position - 1] == '\r')
        {
            if(com->recv_callback)
            {
                com->recv_callback(com->buffer, com->position - 1);
            }
            memset(com->buffer, 0, sizeof(com->buffer));
            com->position = 0;
            continue;
        }
        
        /* normal character */
        com->buffer[com->position] = ch;

        ch = 0;
        com->position++;
        
        /* it's a large data, discard it */
        if (com->position >= COM_MAX_LEN)
            com->position = 0;
        
    }
}

void com_set_recv_callback(int (*callback)(char *data, rt_size_t length))
{
    if(com)
        com->recv_callback = callback;
}

void com_send(char *data, rt_size_t length)
{
    if(com && com->device)
        rt_device_write(com->device, 0, data, length);
}

int commuication_init(void)
{    
    if(com)
    {
        rt_kprintf("commuication already init.\n");
        return RT_EOK;
    }
    
    com = (my_com_t *)rt_calloc(1, sizeof(my_com_t));
    if (com == RT_NULL)
    {
        rt_kprintf("no memory for commuication\n");
        return -RT_ENOMEM;
    }

    /* ���� COM �����߳� */
    rt_thread_t tid = rt_thread_create("com", 
                                       (void (*)(void *parameter))com_rx_thread_entry, 
                                       (void *)RT_NULL, 
                                       COM_RX_THREAD_STACK_SIZE, COM_RX_THREAD_PRIORITY, 10);
                                       
    /* ��ʼ���ź��� */
    rt_sem_init(&com->rx_sem, "com_rx", 0, RT_IPC_FLAG_FIFO);
                                       
    /* �����ɹ��������߳� */
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return RT_EOK;
}
//INIT_APP_EXPORT(commuication_init);
