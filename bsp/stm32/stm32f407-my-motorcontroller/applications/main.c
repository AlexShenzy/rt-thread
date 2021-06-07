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
#include <board.h>

#include "communication.h"
#include "obd_adapter.h"
#include "cJSON_Utils.h"
#include "motor_driver.h"

/* defined the LED0 pin: PB1 */
#define LED0_PIN    GET_PIN(A, 9)
#define CAN1_EN_PIN  GET_PIN(A, 6)
#define CAN1_STB_PIN GET_PIN(A, 7)
#define CAN2_STB_PIN GET_PIN(B, 14)

int com_callback(char *data, rt_size_t length)
{
    cJSON *root = RT_NULL;
    cJSON *tmp = RT_NULL;
    
    root = cJSON_ParseWithLength(data, length);
    if(root == RT_NULL)
    {
        return -1;
    }
    
    tmp = cJSON_GetObjectItem(root, "Motor1_Angle");
    if(tmp)
    {
        motor1_set_angle(cJSON_GetNumberValue(tmp));
    }
    
    tmp = cJSON_GetObjectItem(root, "Motor2_Angle");
    if(tmp)
    {
        motor2_set_angle(cJSON_GetNumberValue(tmp));
    }
    
    if(root)
    {
        cJSON_Delete(root);
        root = RT_NULL;
    }
    return 0;
}
    
int main(void)
{
    int count = 1;
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN,       PIN_MODE_OUTPUT);
    
    commuication_init();
    com_set_recv_callback(com_callback);
    
    motor_enable();
    
    while (count++)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }

    return RT_EOK;
}
