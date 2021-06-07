/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 */

#include <rtthread.h>
#include <rtdevice.h>

static struct rt_semaphore custom_rx_sem;
    
/* 接收数据回调函数 */
static rt_err_t custom_rx_call(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&custom_rx_sem);

    return RT_EOK;
}

void customized_thread_entry(void *parameter)
{
    char dev_name[RT_NAME_MAX];
    rt_device_t dev;
    struct rt_input_capture_data data[2];
    
    rt_strncpy(dev_name, "capture4", RT_NAME_MAX);
    
    dev = rt_device_find(dev_name);
    if (dev == RT_NULL)
    {
        rt_kprintf("find %s failed!\n", dev_name);
        return;
    }
    
    rt_device_open(dev, RT_DEVICE_FLAG_RDWR);
    
    rt_ubase_t water_mark = 2;
    rt_device_control(dev, INPUT_CAPTURE_CMD_SET_WATERMARK, (void *)&water_mark);
    
    rt_sem_init(&custom_rx_sem, "my_rx", 0, RT_IPC_FLAG_FIFO);
    
    rt_device_set_rx_indicate(dev, custom_rx_call);
    
    while(1)
    {
        rt_device_control(dev, INPUT_CAPTURE_CMD_CLEAR_BUF, RT_NULL);
        rt_device_control(dev, INPUT_CAPTURE_CMD_ENABLE, RT_NULL);
        rt_sem_take(&custom_rx_sem, RT_WAITING_FOREVER);
        rt_device_control(dev, INPUT_CAPTURE_CMD_DISABLE, RT_NULL);
        rt_device_read(dev, 0, data, 2);
        
//        rt_kprintf("pulse width: %d us, level: %d\n", data[0].pulsewidth_us, data[0].is_high);
//        rt_kprintf("pulse width: %d us, level: %d\n", data[1].pulsewidth_us, data[1].is_high);
        
        rt_thread_mdelay(1000);
    }
}

int customized_sample_init(void)
{
    rt_thread_t tid;
    
    tid = rt_thread_create("custom", 
                           customized_thread_entry, 
                           (void *)RT_NULL, 
                           1024, 20, 10);
                           
    if (tid == RT_NULL)
    {
        return -RT_ERROR;
    }
    rt_thread_startup(tid);
    
    return RT_EOK;
}
//INIT_APP_EXPORT(customized_sample_init);
