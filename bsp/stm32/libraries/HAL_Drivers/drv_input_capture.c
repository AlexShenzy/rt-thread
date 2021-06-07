/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-04-07     AlexShen     first version
 */

#include "board.h"
#ifdef RT_USING_INPUT_CAPTURE
#include "drv_config.h"

//#define DRV_DEBUG
#define LOG_TAG             "drv.input_capture"
#include <drv_log.h>

#if !defined(BSP_USING_INPUT_CAPTURE2) && !defined(BSP_USING_INPUT_CAPTURE3) \
    && !defined(BSP_USING_INPUT_CAPTURE4) && !defined(BSP_USING_INPUT_CAPTURE5)
    #error "Please define at least one BSP_USING_INPUT_CAPTUREx"
    /* this driver can be disabled at menuconfig → RT-Thread Components → Device Drivers */
#endif

#define AUTO_RELOAD_VALUE   0xFFFF

enum
{
#ifdef BSP_USING_INPUT_CAPTURE2
    INPUT_CAPTURE2_INDEX,
#endif
#ifdef BSP_USING_INPUT_CAPTURE3
    INPUT_CAPTURE3_INDEX,
#endif
#ifdef BSP_USING_INPUT_CAPTURE4
    INPUT_CAPTURE4_INDEX,
#endif
#ifdef BSP_USING_INPUT_CAPTURE5
    INPUT_CAPTURE5_INDEX,
#endif
};



struct stm32_input_capture_device
{
    struct rt_input_capture_device  parent;
    
    char                            *name;
    TIM_HandleTypeDef               tim_handler;
    IRQn_Type                       capture_irqn;
    
    rt_uint32_t                     channel;
    rt_uint32_t                     flowcount;
    rt_bool_t                       first_edge;
    rt_bool_t                       level;
    
};

static struct stm32_input_capture_device stm32_input_capture_obj[] =
{
#ifdef BSP_USING_INPUT_CAPTURE2
    INPUT_CAPTURE2_CONFIG,
#endif
#ifdef BSP_USING_INPUT_CAPTURE3
    INPUT_CAPTURE3_CONFIG,
#endif
#ifdef BSP_USING_INPUT_CAPTURE4
    INPUT_CAPTURE4_CONFIG,
#endif
#ifdef BSP_USING_INPUT_CAPTURE5
    INPUT_CAPTURE5_CONFIG,
#endif
};

rt_err_t input_capture_init(struct rt_input_capture_device *device)
{
    rt_err_t result = RT_EOK;
    TIM_HandleTypeDef *tim = RT_NULL;
    TIM_IC_InitTypeDef sConfigIC = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    struct stm32_input_capture_device *stm32_device;
    
    RT_ASSERT(device != RT_NULL);

    stm32_device = (struct stm32_input_capture_device*)device;
    tim = (TIM_HandleTypeDef *)&stm32_device->tim_handler;
    
    tim->Init.Prescaler = 0;
    tim->Init.CounterMode = TIM_COUNTERMODE_UP;
    tim->Init.Period = AUTO_RELOAD_VALUE;
    tim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    tim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if (HAL_TIM_Base_Init(tim) != HAL_OK)
    {
        LOG_E("%s base init failed", stm32_device->name);
        result = -RT_ERROR;
        goto __exit;
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(tim, &sClockSourceConfig) != HAL_OK)
    {
        LOG_E("%s clock init failed", stm32_device->name);
        result = -RT_ERROR;
        goto __exit;
    }

    if (HAL_TIM_IC_Init(tim) != HAL_OK)
    {
        LOG_E("%s ic init failed", stm32_device->name);
        result = -RT_ERROR;
        goto __exit;
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(tim, &sMasterConfig) != HAL_OK)
    {
        LOG_E("%s master config failed", stm32_device->name);
        result = -RT_ERROR;
        goto __exit;
    }

    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;

    /* config ic channel */
    if (stm32_device->channel == TIM_CHANNEL_1)
    {
        if (HAL_TIM_IC_ConfigChannel(tim, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
        {
            LOG_E("%s channel1 config failed", stm32_device->name);
            result = -RT_ERROR;
            goto __exit;
        }
    }
    else if (stm32_device->channel == TIM_CHANNEL_2)
    {
        if (HAL_TIM_IC_ConfigChannel(tim, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
        {
            LOG_E("%s channel2 config failed", stm32_device->name);
            result = -RT_ERROR;
            goto __exit;
        }
    }
    else if (stm32_device->channel == TIM_CHANNEL_3)
    {
        if (HAL_TIM_IC_ConfigChannel(tim, &sConfigIC, TIM_CHANNEL_3) != HAL_OK)
        {
            LOG_E("%s channel3 config failed", stm32_device->name);
            result = -RT_ERROR;
            goto __exit;
        }
    }
    else if (stm32_device->channel == TIM_CHANNEL_4)
    {
        if (HAL_TIM_IC_ConfigChannel(tim, &sConfigIC, TIM_CHANNEL_4) != HAL_OK)
        {
            LOG_E("%s channel4 config failed", stm32_device->name);
            result = -RT_ERROR;
            goto __exit;
        }
    }

    HAL_NVIC_SetPriority(stm32_device->capture_irqn, 3, 0);

__exit:
    return result;
}

rt_err_t input_capture_open(struct rt_input_capture_device *device)
{
    struct stm32_input_capture_device *stm32_device;
    
    RT_ASSERT(device != RT_NULL);
    
    stm32_device = (struct stm32_input_capture_device*)device;

    stm32_device->first_edge   = RT_TRUE;
    stm32_device->level        = PIN_LOW;
    
    /* enable the TIMx global Interrupt */
    HAL_NVIC_EnableIRQ(stm32_device->capture_irqn);

    /* clear update flag */
    __HAL_TIM_CLEAR_FLAG(&stm32_device->tim_handler, TIM_FLAG_UPDATE);
    
    /* clear Capture compare event*/
    if (stm32_device->channel == TIM_CHANNEL_1)
    {
        __HAL_TIM_CLEAR_FLAG(&stm32_device->tim_handler, TIM_FLAG_CC1);
    }
    else if (stm32_device->channel == TIM_CHANNEL_2)
    {
        __HAL_TIM_CLEAR_FLAG(&stm32_device->tim_handler, TIM_FLAG_CC2);
    }
    else if (stm32_device->channel == TIM_CHANNEL_3)
    {
        __HAL_TIM_CLEAR_FLAG(&stm32_device->tim_handler, TIM_FLAG_CC3);
    }
    else if (stm32_device->channel == TIM_CHANNEL_4)
    {
        __HAL_TIM_CLEAR_FLAG(&stm32_device->tim_handler, TIM_FLAG_CC4);
    }
    
    /* enable update request source */
    __HAL_TIM_URS_ENABLE(&stm32_device->tim_handler);
    
    __HAL_TIM_SET_CAPTUREPOLARITY(&stm32_device->tim_handler, stm32_device->channel, TIM_INPUTCHANNELPOLARITY_RISING);
    HAL_TIM_IC_Start_IT(&stm32_device->tim_handler, stm32_device->channel);
    
    return RT_EOK;
}

rt_err_t input_capture_close(struct rt_input_capture_device *device)
{
    struct stm32_input_capture_device *stm32_device;
    
    RT_ASSERT(device != RT_NULL);
    
    stm32_device = (struct stm32_input_capture_device*)device;
    
    /* disable the TIMx global Interrupt */
    HAL_NVIC_DisableIRQ(stm32_device->capture_irqn);
    
    /* disable update request source */
    __HAL_TIM_URS_DISABLE(&stm32_device->tim_handler);
    
    HAL_TIM_IC_Stop_IT(&stm32_device->tim_handler, stm32_device->channel);
    
    return RT_EOK;
}

rt_err_t input_capture_get_pulsewidth(struct rt_input_capture_device *device, rt_uint32_t *pulsewidth_us)
{
    rt_uint64_t tim_clock_us;
    struct stm32_input_capture_device *stm32_device;
    rt_uint64_t tick;
    
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(pulsewidth_us != RT_NULL);
    
    stm32_device = (struct stm32_input_capture_device*)device;
    
    if (stm32_device->tim_handler.Instance == TIM1 || stm32_device->tim_handler.Instance == TIM8
        || stm32_device->tim_handler.Instance == TIM9 || stm32_device->tim_handler.Instance == TIM10
        || stm32_device->tim_handler.Instance == TIM11)
    {
        tim_clock_us = HAL_RCC_GetPCLK2Freq() * 2 / 1000000;
    }
    else
    {
        tim_clock_us = HAL_RCC_GetPCLK1Freq() * 2 / 1000000;
    }
    
    tick = HAL_TIM_ReadCapturedValue(&stm32_device->tim_handler, stm32_device->channel);
    tick += stm32_device->flowcount * (stm32_device->tim_handler.Instance->ARR + 1);
    *pulsewidth_us = tick / tim_clock_us;
    
    __HAL_TIM_SET_COUNTER(&stm32_device->tim_handler, 0);   //清零计数器
    stm32_device->flowcount = 0;    //清除溢出计数
    return RT_EOK;
}

void input_capture_update_isr(struct stm32_input_capture_device *device)
{
    /* TIM Update event */
    if (__HAL_TIM_GET_FLAG(&device->tim_handler, TIM_FLAG_UPDATE) != RESET)
    {
        __HAL_TIM_CLEAR_IT(&device->tim_handler, TIM_IT_UPDATE);
        if (__HAL_TIM_IS_TIM_COUNTING_DOWN(&device->tim_handler))
        {
            device->flowcount--;
        }
        else
        {
            device->flowcount++;
        }
    }
    /* Capture compare 1 event */
    if (__HAL_TIM_GET_FLAG(&device->tim_handler, TIM_FLAG_CC1) != RESET)
    {
        __HAL_TIM_CLEAR_IT(&device->tim_handler, TIM_IT_CC1);
        /* Input capture event */
        if ((device->tim_handler.Instance->CCMR1 & TIM_CCMR1_CC1S) != 0x00U)
        {
            if(device->first_edge)
            {
                device->first_edge = RT_FALSE;
				__HAL_TIM_SET_COUNTER(&device->tim_handler, 0);
                device->flowcount = 0;
            }
            else
            {
                rt_hw_input_capture_isr(&device->parent, device->level);
            }
            
            if(device->level == PIN_LOW)
            {
                device->level = PIN_HIGH;
                
                /* 切换捕获极性 */
                __HAL_TIM_SET_CAPTUREPOLARITY(&device->tim_handler, device->channel, TIM_INPUTCHANNELPOLARITY_FALLING);
            }
            else
            {
                device->level = PIN_LOW;
            
                /* 切换捕获极性 */
                __HAL_TIM_SET_CAPTUREPOLARITY(&device->tim_handler, device->channel, TIM_INPUTCHANNELPOLARITY_RISING);
            }
            
			HAL_TIM_IC_Start_IT(&device->tim_handler, device->channel);
        }
    }
    /* Capture compare 2 event */
    if (__HAL_TIM_GET_FLAG(&device->tim_handler, TIM_FLAG_CC2) != RESET)
    {
        __HAL_TIM_CLEAR_IT(&device->tim_handler, TIM_IT_CC2);
        /* Input capture event */
        if ((device->tim_handler.Instance->CCMR1 & TIM_CCMR1_CC2S) != 0x00U)
        {
            if(device->first_edge)
            {
                device->first_edge = RT_FALSE;
				__HAL_TIM_SET_COUNTER(&device->tim_handler, 0);
                device->flowcount = 0;
            }
            else
            {
                rt_hw_input_capture_isr(&device->parent, device->level);
            }
            
            if(device->level == PIN_LOW)
            {
                device->level = PIN_HIGH;
                
                /* 切换捕获极性 */
                __HAL_TIM_SET_CAPTUREPOLARITY(&device->tim_handler, device->channel, TIM_INPUTCHANNELPOLARITY_FALLING);
            }
            else
            {
                device->level = PIN_LOW;
            
                /* 切换捕获极性 */
                __HAL_TIM_SET_CAPTUREPOLARITY(&device->tim_handler, device->channel, TIM_INPUTCHANNELPOLARITY_RISING);
            }
            
			HAL_TIM_IC_Start_IT(&device->tim_handler, device->channel);
        }
    }
    /* Capture compare 3 event */
    if (__HAL_TIM_GET_FLAG(&device->tim_handler, TIM_FLAG_CC3) != RESET)
    {
        __HAL_TIM_CLEAR_IT(&device->tim_handler, TIM_IT_CC3);
        /* Input capture event */
        if ((device->tim_handler.Instance->CCMR2 & TIM_CCMR2_CC3S) != 0x00U)
        {
            if(device->first_edge)
            {
                device->first_edge = RT_FALSE;
				__HAL_TIM_SET_COUNTER(&device->tim_handler, 0);
                device->flowcount = 0;
            }
            else
            {
                rt_hw_input_capture_isr(&device->parent, device->level);
            }
            
            if(device->level == PIN_LOW)
            {
                device->level = PIN_HIGH;
                
                /* 切换捕获极性 */
                __HAL_TIM_SET_CAPTUREPOLARITY(&device->tim_handler, device->channel, TIM_INPUTCHANNELPOLARITY_FALLING);
            }
            else
            {
                device->level = PIN_LOW;
            
                /* 切换捕获极性 */
                __HAL_TIM_SET_CAPTUREPOLARITY(&device->tim_handler, device->channel, TIM_INPUTCHANNELPOLARITY_RISING);
            }
            
			HAL_TIM_IC_Start_IT(&device->tim_handler, device->channel);
        }
    }
    /* Capture compare 4 event */
    if (__HAL_TIM_GET_FLAG(&device->tim_handler, TIM_FLAG_CC4) != RESET)
    {
        __HAL_TIM_CLEAR_IT(&device->tim_handler, TIM_IT_CC4);
        /* Input capture event */
        if ((device->tim_handler.Instance->CCMR2 & TIM_CCMR2_CC4S) != 0x00U)
        {
            if(device->first_edge)
            {
                device->first_edge = RT_FALSE;
				__HAL_TIM_SET_COUNTER(&device->tim_handler, 0);
                device->flowcount = 0;
            }
            else
            {
                rt_hw_input_capture_isr(&device->parent, device->level);
            }
            
            if(device->level == PIN_LOW)
            {
                device->level = PIN_HIGH;
                
                /* 切换捕获极性 */
                __HAL_TIM_SET_CAPTUREPOLARITY(&device->tim_handler, device->channel, TIM_INPUTCHANNELPOLARITY_FALLING);
            }
            else
            {
                device->level = PIN_LOW;
            
                /* 切换捕获极性 */
                __HAL_TIM_SET_CAPTUREPOLARITY(&device->tim_handler, device->channel, TIM_INPUTCHANNELPOLARITY_RISING);
            }
            
			HAL_TIM_IC_Start_IT(&device->tim_handler, device->channel);
        }
    }
    /* TIM Break input event */
    if (__HAL_TIM_GET_FLAG(&device->tim_handler, TIM_FLAG_BREAK) != RESET)
    {
        __HAL_TIM_CLEAR_IT(&device->tim_handler, TIM_IT_BREAK);
    }
    /* TIM Trigger detection event */
    if (__HAL_TIM_GET_FLAG(&device->tim_handler, TIM_FLAG_TRIGGER) != RESET)
    {
        __HAL_TIM_CLEAR_IT(&device->tim_handler, TIM_IT_TRIGGER);
    }
    /* TIM commutation event */
    if (__HAL_TIM_GET_FLAG(&device->tim_handler, TIM_FLAG_COM) != RESET)
    {
        __HAL_TIM_CLEAR_IT(&device->tim_handler, TIM_FLAG_COM);
    }
}

#ifdef BSP_USING_INPUT_CAPTURE2
void TIM2_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();
    input_capture_update_isr(&stm32_input_capture_obj[INPUT_CAPTURE2_INDEX]);
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif
#ifdef BSP_USING_INPUT_CAPTURE3
void TIM3_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();
    input_capture_update_isr(&stm32_input_capture_obj[INPUT_CAPTURE3_INDEX]);
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif
#ifdef BSP_USING_INPUT_CAPTURE4
void TIM4_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();
    input_capture_update_isr(&stm32_input_capture_obj[INPUT_CAPTURE4_INDEX]);
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif
#ifdef BSP_USING_INPUT_CAPTURE5
void TIM5_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();
    input_capture_update_isr(&stm32_input_capture_obj[INPUT_CAPTURE5_INDEX]);
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

static const struct rt_input_capture_ops _ops =
{
    .init = input_capture_init,
    .open = input_capture_open,
    .close = input_capture_close,
    .get_pulsewidth = input_capture_get_pulsewidth,
};

static void input_capture_get_channel(void)
{
#ifdef BSP_USING_INPUT_CAPTURE2_CH1
    stm32_input_capture_obj[INPUT_CAPTURE2_INDEX].channel = TIM_CHANNEL_1;
#endif
#ifdef BSP_USING_INPUT_CAPTURE2_CH2
    stm32_input_capture_obj[INPUT_CAPTURE2_INDEX].channel = TIM_CHANNEL_2;
#endif
#ifdef BSP_USING_INPUT_CAPTURE2_CH3
    stm32_input_capture_obj[INPUT_CAPTURE2_INDEX].channel = TIM_CHANNEL_3;
#endif
#ifdef BSP_USING_INPUT_CAPTURE2_CH4
    stm32_input_capture_obj[INPUT_CAPTURE2_INDEX].channel = TIM_CHANNEL_4;
#endif
#ifdef BSP_USING_INPUT_CAPTURE3_CH1
    stm32_input_capture_obj[INPUT_CAPTURE3_INDEX].channel = TIM_CHANNEL_1;
#endif                                                   
#ifdef BSP_USING_INPUT_CAPTURE3_CH2                      
    stm32_input_capture_obj[INPUT_CAPTURE3_INDEX].channel = TIM_CHANNEL_2;
#endif                                                   
#ifdef BSP_USING_INPUT_CAPTURE3_CH3                      
    stm32_input_capture_obj[INPUT_CAPTURE3_INDEX].channel = TIM_CHANNEL_3;
#endif                                                   
#ifdef BSP_USING_INPUT_CAPTURE3_CH4                      
    stm32_input_capture_obj[INPUT_CAPTURE3_INDEX].channel = TIM_CHANNEL_4;
#endif
#ifdef BSP_USING_INPUT_CAPTURE4_CH1
    stm32_input_capture_obj[INPUT_CAPTURE4_INDEX].channel = TIM_CHANNEL_1;
#endif                                                   
#ifdef BSP_USING_INPUT_CAPTURE4_CH2                      
    stm32_input_capture_obj[INPUT_CAPTURE4_INDEX].channel = TIM_CHANNEL_2;
#endif                                                   
#ifdef BSP_USING_INPUT_CAPTURE4_CH3                      
    stm32_input_capture_obj[INPUT_CAPTURE4_INDEX].channel = TIM_CHANNEL_3;
#endif                                                   
#ifdef BSP_USING_INPUT_CAPTURE4_CH4                      
    stm32_input_capture_obj[INPUT_CAPTURE4_INDEX].channel = TIM_CHANNEL_4;
#endif
#ifdef BSP_USING_INPUT_CAPTURE5_CH1
    stm32_input_capture_obj[INPUT_CAPTURE5_INDEX].channel = TIM_CHANNEL_1;
#endif                                                   
#ifdef BSP_USING_INPUT_CAPTURE5_CH2                      
    stm32_input_capture_obj[INPUT_CAPTURE5_INDEX].channel = TIM_CHANNEL_2;
#endif                                                   
#ifdef BSP_USING_INPUT_CAPTURE5_CH3                      
    stm32_input_capture_obj[INPUT_CAPTURE5_INDEX].channel = TIM_CHANNEL_3;
#endif                                                   
#ifdef BSP_USING_INPUT_CAPTURE5_CH4                      
    stm32_input_capture_obj[INPUT_CAPTURE5_INDEX].channel = TIM_CHANNEL_4;
#endif
}

int hw_input_capture_init(void)
{
    int i;
    int result = RT_EOK;
    
    input_capture_get_channel();
    
    for (i = 0; i < sizeof(stm32_input_capture_obj) / sizeof(stm32_input_capture_obj[0]); i++)
    {
        stm32_input_capture_obj[i].parent.ops   = &_ops;

        if (rt_device_input_capture_register(&stm32_input_capture_obj[i].parent, stm32_input_capture_obj[i].name, RT_NULL) != RT_EOK)
        {
            LOG_E("%s register failed", stm32_input_capture_obj[i].name);
            result = -RT_ERROR;
        }
    }
    
    return result;
}
INIT_BOARD_EXPORT(hw_input_capture_init);

#endif
