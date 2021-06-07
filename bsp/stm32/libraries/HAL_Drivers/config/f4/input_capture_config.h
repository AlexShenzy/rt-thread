/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-04-7      AlexShen     first version
 */

#ifndef __INPUT_CAPTURE_CONFIG_H__
#define __INPUT_CAPTURE_CONFIG_H__

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BSP_USING_INPUT_CAPTURE2
#ifndef INPUT_CAPTURE2_CONFIG
#define INPUT_CAPTURE2_CONFIG                          \
    {                                                  \
       .tim_handler.Instance    = TIM2,                \
       .capture_irqn            = TIM2_IRQn,           \
       .name                    = "capture2"           \
    }
#endif /* INPUT_CAPTURE2_CONFIG */
#endif /* BSP_USING_INPUT_CAPTURE2 */

#ifdef BSP_USING_INPUT_CAPTURE3
#ifndef INPUT_CAPTURE3_CONFIG
#define INPUT_CAPTURE3_CONFIG                          \
    {                                                  \
       .tim_handler.Instance    = TIM3,                \
       .capture_irqn            = TIM3_IRQn,           \
       .name                    = "capture3"           \
    }
#endif /* INPUT_CAPTURE3_CONFIG */
#endif /* BSP_USING_INPUT_CAPTURE3 */

#ifdef BSP_USING_INPUT_CAPTURE4
#ifndef INPUT_CAPTURE4_CONFIG
#define INPUT_CAPTURE4_CONFIG                          \
    {                                                  \
       .tim_handler.Instance    = TIM4,                \
       .capture_irqn            = TIM4_IRQn,           \
       .name                    = "capture4"           \
    }
#endif /* INPUT_CAPTURE4_CONFIG */
#endif /* BSP_USING_INPUT_CAPTURE4 */

#ifdef BSP_USING_INPUT_CAPTURE5
#ifndef INPUT_CAPTURE5_CONFIG
#define INPUT_CAPTURE5_CONFIG                          \
    {                                                  \
       .tim_handler.Instance    = TIM5,                \
       .capture_irqn            = TIM5_IRQn,           \
       .name                    = "capture5"           \
    }
#endif /* INPUT_CAPTURE5_CONFIG */
#endif /* BSP_USING_INPUT_CAPTURE5 */

#ifdef __cplusplus
}
#endif

#endif /* __INPUT_CAPTURE_CONFIG_H__ */
