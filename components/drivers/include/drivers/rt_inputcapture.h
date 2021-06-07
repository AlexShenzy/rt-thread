/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-08-13     balanceTWK   first version.
 */

#ifndef __INPUT_CAPTURE_H__
#define __INPUT_CAPTURE_H__

#include <rtthread.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C" {
#endif

/* capture control command */
#define INPUT_CAPTURE_CMD_CLEAR_BUF        (128 + 0)    /* clear capture buf */
#define INPUT_CAPTURE_CMD_SET_WATERMARK    (128 + 1)    /* Set the callback threshold */
#define INPUT_CAPTURE_CMD_ENABLE           (128 + 2)    /* Enable input capture */
#define INPUT_CAPTURE_CMD_DISABLE          (128 + 3)    /* Disable input capture */

struct rt_input_capture_data
{
    rt_uint32_t pulsewidth_us;
    rt_bool_t   is_high;
};

struct rt_input_capture_device
{
    struct rt_device                    parent;

    const struct rt_input_capture_ops   *ops;
    struct rt_ringbuffer                *ringbuff;
    rt_size_t                           watermark;
};

/**
 * capture operators
 */
struct rt_input_capture_ops
{
    rt_err_t (*init)(struct rt_input_capture_device *input_capture);
    rt_err_t (*open)(struct rt_input_capture_device *input_capture);
    rt_err_t (*close)(struct rt_input_capture_device *input_capture);
    rt_err_t (*get_pulsewidth)(struct rt_input_capture_device *input_capture, rt_uint32_t *pulsewidth_us);
};

void rt_hw_input_capture_isr(struct rt_input_capture_device *input_capture, rt_bool_t level);

rt_err_t rt_device_input_capture_register(struct rt_input_capture_device *input_capture,
                                         const char                      *name,
                                         void                            *data);
#ifdef __cplusplus
}
#endif

#endif /* __INPUT_CAPTURE_H__ */
