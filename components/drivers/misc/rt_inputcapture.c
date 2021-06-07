/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-08-13     balanceTWK   the first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#include <drivers/rt_inputcapture.h>

#define DBG_TAG    "INPUTCAPTURE"
#define DBG_LVL    DBG_INFO
#include <rtdbg.h>

static rt_err_t rt_input_capture_init(struct rt_device *dev)
{
    rt_err_t ret;
    struct rt_input_capture_device *input_capture;

    RT_ASSERT(dev != RT_NULL);

    ret = RT_EOK;
    input_capture = (struct rt_input_capture_device *)dev;
    input_capture->watermark = RT_INPUT_CAPTURE_RB_SIZE / 2;
    if (input_capture->ops->init)
    {
        ret = input_capture->ops->init(input_capture);
    }

    return ret;
}

static rt_err_t rt_input_capture_open(struct rt_device *dev, rt_uint16_t oflag)
{
    rt_err_t ret;
    struct rt_input_capture_device *input_capture;

    RT_ASSERT(dev != RT_NULL);

    ret = RT_EOK;
    input_capture = (struct rt_input_capture_device *)dev;
    if (input_capture->ringbuff == RT_NULL)
    {
        input_capture->ringbuff = rt_ringbuffer_create(sizeof(struct rt_input_capture_data) * RT_INPUT_CAPTURE_RB_SIZE);
    }
    if (input_capture->ops->open)
    {
        ret = input_capture->ops->open(input_capture);
    }

    return ret;
}

static rt_err_t rt_input_capture_close(struct rt_device *dev)
{
    rt_err_t ret;
    struct rt_input_capture_device *input_capture;

    RT_ASSERT(dev != RT_NULL);

    ret = -RT_ERROR;
    input_capture = (struct rt_input_capture_device *)dev;

    if (input_capture->ops->close)
    {
        ret = input_capture->ops->close(input_capture);
    }

    if (ret != RT_EOK)
    {
        return ret;
    }

    if (input_capture->ringbuff)
    {
        rt_ringbuffer_destroy(input_capture->ringbuff);
        input_capture->ringbuff = RT_NULL;
    }
    return ret;
}

static rt_size_t rt_input_capture_read(struct rt_device *dev,
                                 rt_off_t          pos,
                                 void             *buffer,
                                 rt_size_t         size)
{
    rt_size_t receive_size;
    struct rt_input_capture_device *input_capture;

    RT_ASSERT(dev != RT_NULL);

    input_capture = (struct rt_input_capture_device *)dev;
    receive_size = rt_ringbuffer_get(input_capture->ringbuff, (rt_uint8_t *)buffer, sizeof(struct rt_input_capture_data) * size);

    return receive_size / sizeof(struct rt_input_capture_data);
}

static rt_err_t rt_input_capture_control(struct rt_device *dev, int cmd, void *args)
{
    rt_err_t result;
    struct rt_input_capture_device *input_capture;

    RT_ASSERT(dev != RT_NULL);

    result = RT_EOK;
    input_capture = (struct rt_input_capture_device *)dev;
    switch (cmd)
    {
    case INPUT_CAPTURE_CMD_CLEAR_BUF:
        if (input_capture->ringbuff)
        {
            rt_ringbuffer_reset(input_capture->ringbuff);
        }
        break;
    case INPUT_CAPTURE_CMD_SET_WATERMARK:
        input_capture->watermark = *(rt_size_t *)args;
        break;
    case INPUT_CAPTURE_CMD_ENABLE:
        result = input_capture->ops->open(input_capture);
        break;
    case INPUT_CAPTURE_CMD_DISABLE:
        result = input_capture->ops->close(input_capture);
        break;
    default:
        result = -RT_ENOSYS;
        break;
    }

    return result;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops input_capture_ops =
{
    rt_input_capture_init,
    rt_input_capture_open,
    rt_input_capture_close,
    rt_input_capture_read,
    RT_NULL,
    rt_input_capture_control
};
#endif

rt_err_t rt_device_input_capture_register(struct rt_input_capture_device *input_capture, const char *name, void *user_data)
{
    struct rt_device *device;

    RT_ASSERT(input_capture != RT_NULL);
    RT_ASSERT(input_capture->ops != RT_NULL);
    RT_ASSERT(input_capture->ops->get_pulsewidth != RT_NULL);

    device = &(input_capture->parent);

    device->type        = RT_Device_Class_Miscellaneous;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;
    input_capture->ringbuff = RT_NULL;

#ifdef RT_USING_DEVICE_OPS
    device->ops         = &input_capture_ops;
#else
    device->init        = rt_input_capture_init;
    device->open        = rt_input_capture_open;
    device->close       = rt_input_capture_close;
    device->read        = rt_input_capture_read;
    device->write       = RT_NULL;
    device->control     = rt_input_capture_control;
#endif
    device->user_data   = user_data;

    return rt_device_register(device, name, RT_DEVICE_FLAG_RDONLY | RT_DEVICE_FLAG_STANDALONE);
}

/**
 * This function is ISR for input_capture interrupt.
 * level: RT_TRUE denotes high level pulse, and RT_FALSE denotes low level pulse.
 */
void rt_hw_input_capture_isr(struct rt_input_capture_device *input_capture, rt_bool_t level)
{
    struct rt_input_capture_data data;
    rt_size_t receive_size;
    if (input_capture->ops->get_pulsewidth(input_capture, &data.pulsewidth_us) != RT_EOK)
    {
        return;
    }

    data.is_high = level;
    if (rt_ringbuffer_put(input_capture->ringbuff, (rt_uint8_t *)&data, sizeof(struct rt_input_capture_data)) == 0)
    {
        LOG_W("input_capture ringbuffer doesn't have enough space.");
    }

    receive_size =  rt_ringbuffer_data_len(input_capture->ringbuff) / sizeof(struct rt_input_capture_data);

    if (receive_size >= input_capture->watermark)
    {
        /* indicate to upper layer application */
        if (input_capture->parent.rx_indicate != RT_NULL)
            input_capture->parent.rx_indicate(&input_capture->parent, receive_size);
    }
}
