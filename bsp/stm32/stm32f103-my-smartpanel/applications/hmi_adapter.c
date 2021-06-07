#include <string.h>

#include "hmi_adapter.h"
#include "communication.h"

 my_hmi_t *hmi;

static void hmi_update_page_id(rt_uint8_t page_id, rt_uint8_t ctl_id, rt_uint8_t event)
{
    if(page_id == 0) {
        if(ctl_id == 9 && event == 1)
            hmi->page_id = 1;
        else if(ctl_id == 11 && event == 1)
            hmi->page_id = 2;
    }
    else if(page_id == 1) {
        if(ctl_id == 14 && event == 1)
            hmi->page_id = 0;
    }
    else if(page_id == 2) {
        if(ctl_id == 13 && event == 1)
            hmi->page_id = 0;
    }
}

static void hmi_request_value(rt_uint8_t page_id, rt_uint8_t ctl_id, rt_uint8_t event)
{
    if(hmi->req_sta != 0) return;
    
    if(page_id == 0) {
    }
    else if(page_id == 1) {
    }
    else if(page_id == 2) {
        if(ctl_id == 9 && event == 0) {
            hmi_send("get h1.val", rt_strlen("get h1.val"));
            hmi->req_sta = 1;
            hmi->req_page_id = page_id;
            hmi->req_ctl_id = ctl_id;
        }
    }
}

static void hmi_analyze_ctl_val(rt_uint8_t page_id, rt_uint8_t ctl_id, rt_uint32_t val)
{
    if(page_id == 0) {
    }
    else if(page_id == 1) {
    }
    else if(page_id == 2) {
        if(ctl_id == 9) {
            char cmd[64] = "";
            val = val * 90 / 50 + 90;
            rt_snprintf(cmd, sizeof(cmd), "{\"Motor1_Angle\":%d,\"Motor2_Angle\":%d}\r\n", val, val);
            com_send(cmd,rt_strlen(cmd));
            hmi->req_sta = 0;
        }
    }
}

static void hmi_analyze_ctl_str(rt_uint8_t page_id, rt_uint8_t ctl_id, char *str)
{
    return;
}


/* 接收数据回调函数 */
static rt_err_t hmi_rx_call(rt_device_t dev, rt_size_t size)
{
    /* 串口接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&hmi->rx_sem);

    return RT_EOK;
}

static int hmi_recv_completed(char *data, rt_size_t length)
{
    switch(data[0])
    {
        case 0x65:
            hmi_update_page_id(data[1], data[2], data[3]);
            hmi_request_value(data[1], data[2], data[3]);
            break;
        case 0x70:
            if(hmi->req_sta == 2) {
                char *str = data + 1;
                hmi_analyze_ctl_str(hmi->req_page_id, hmi->req_ctl_id, str);
            }
            break;
        case 0x71:
            if(hmi->req_sta == 1) {
                rt_uint32_t val = *(rt_uint32_t *)(data + 1);
                hmi_analyze_ctl_val(hmi->req_page_id, hmi->req_ctl_id, val);
            }
            break;
        default:
            break;
    }
    return 0;
}

static void hmi_rx_thread_entry(void *parameter)
{
    char ch;
    char uart_name[RT_NAME_MAX];
    
    rt_strncpy(uart_name, HMI_DEV_NAME, RT_NAME_MAX);
    
    /* 查找系统中的串口设备 */
    if(hmi->device == RT_NULL)
    {
        hmi->device = rt_device_find(uart_name);
        if (hmi->device == RT_NULL)
        {
            rt_kprintf("find %s failed!\n", uart_name);
            return;
        }
    }

    /* 以中断接收及轮询发送模式打开串口设备 */
    rt_device_open(hmi->device, RT_DEVICE_FLAG_INT_RX);
    
    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(hmi->device, hmi_rx_call);
    
    while (1) {
        /* 从串口阻塞等待读取一个数据 */
        while (rt_device_read(hmi->device, -1, &ch, 1) != 1)
            rt_sem_take(&hmi->rx_sem, RT_WAITING_FOREVER);
                
        /* handle end of line, break */
        if (ch == 0xFF
            && hmi->buffer[hmi->position - 1] == 0xFF
            && hmi->buffer[hmi->position - 2] == 0xFF)
        {
            if(hmi->position > 2)
            {
                hmi_recv_completed(hmi->buffer, hmi->position - 2);
            }
            memset(hmi->buffer, 0, sizeof(hmi->buffer));
            hmi->position = 0;
            continue;
        }
        
        /* normal character */
        hmi->buffer[hmi->position] = ch;

        ch = 0;
        hmi->position++;
        
        /* it's a large data, discard it */
        if (hmi->position >= HMI_MAX_LEN)
            hmi->position = 0;
        
    }
}

void hmi_send(char *cmd, rt_size_t length)
{
    if(hmi)
    {
        rt_device_write(hmi->device, 0, cmd, length);
        rt_device_write(hmi->device, 0, "\xFF\xFF\xFF", 3);
    }
}

void hmi_page_up(void)
{
    if(hmi->page_id == 0) {
        hmi_send("page 1", rt_strlen("page 1"));
        hmi->page_id = 1;
    }
    else if(hmi->page_id == 1) {
        hmi_send("page 2", rt_strlen("page 2"));
        hmi->page_id = 2;
    }
    else if(hmi->page_id == 2) {
        hmi_send("page 0", rt_strlen("page 0"));
        hmi->page_id = 0;
    }
}

void hmi_page_down(void)
{
    if(hmi->page_id == 0) {
        hmi_send("page 2", rt_strlen("page 2"));
        hmi->page_id = 2;
    }
    else if(hmi->page_id == 1) {
        hmi_send("page 0", rt_strlen("page 0"));
        hmi->page_id = 0;
    }
    else if(hmi->page_id == 2) {
        hmi_send("page 1", rt_strlen("page 1"));
        hmi->page_id = 1;
    }
}


int hmi_adapter_init(void)
{    
    if(hmi)
    {
        rt_kprintf("hmi already init.\n");
        return RT_EOK;
    }
    
    hmi = (my_hmi_t *)rt_calloc(1, sizeof(my_hmi_t));
    if (hmi == RT_NULL)
    {
        rt_kprintf("no memory for hmi\n");
        return -RT_ENOMEM;
    }

    /* 创建 HMI 接收线程 */
    rt_thread_t tid = rt_thread_create("hmi", 
                                       (void (*)(void *parameter))hmi_rx_thread_entry, 
                                       (void *)RT_NULL, 
                                       HMI_RX_THREAD_STACK_SIZE, HMI_RX_THREAD_PRIORITY, 10);
                                       
    /* 初始化信号量 */
    rt_sem_init(&hmi->rx_sem, "hmi_rx", 0, RT_IPC_FLAG_FIFO);
                                       
    /* 创建成功则启动线程 */
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return RT_EOK;
}
INIT_APP_EXPORT(hmi_adapter_init);
