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
#include "hmi_adapter.h"
#include "cJSON_Utils.h"

int com_callback(char *data, rt_size_t length)
{
    cJSON *root = RT_NULL;
    cJSON *tmp = RT_NULL;
    char hmi_cmd[64] = "";
    
    root = cJSON_ParseWithLength(data, length);
    if(root == RT_NULL)
    {
        return -1;
    }
    
    tmp = cJSON_GetObjectItem(root, "ENGINE_LOAD");
    if(tmp)
    {
        rt_snprintf(hmi_cmd, sizeof(hmi_cmd) - 1, "load.val=%d", (int)cJSON_GetNumberValue(tmp));
        hmi_send(hmi_cmd, rt_strlen(hmi_cmd));
        rt_memset(hmi_cmd, 0x00, sizeof(hmi_cmd));
    }
    
    tmp = cJSON_GetObjectItem(root, "ENGINE_COOLANT_TEMP");
    if(tmp)
    {
        rt_snprintf(hmi_cmd, sizeof(hmi_cmd) - 1, "temp.val=%d", (int)cJSON_GetNumberValue(tmp));
        hmi_send(hmi_cmd, rt_strlen(hmi_cmd));
        rt_memset(hmi_cmd, 0x00, sizeof(hmi_cmd));
    }
    
//    tmp = cJSON_GetObjectItem(root, "INTAKE_MANIFOLD_ABS_PRESSURE");
//    if(tmp)
//    {
//        rt_snprintf(hmi_cmd, sizeof(hmi_cmd) - 1, "perssure.val=%d", (int)cJSON_GetNumberValue(tmp));
//        hmi_send(hmi_cmd, rt_strlen(hmi_cmd));
//        rt_memset(hmi_cmd, 0x00, sizeof(hmi_cmd));
//    }
    
    tmp = cJSON_GetObjectItem(root, "ENGINE_RPM");
    if(tmp)
    {
        rt_int32_t rpm = cJSON_GetNumberValue(tmp);
        rpm = rpm > 9000 ? 9000 : rpm;
        rt_snprintf(hmi_cmd, sizeof(hmi_cmd) - 1, "rpm.val=%d", rpm * 30 / 1000);
        hmi_send(hmi_cmd, rt_strlen(hmi_cmd));
        rt_memset(hmi_cmd, 0x00, sizeof(hmi_cmd));
    }
    
    tmp = cJSON_GetObjectItem(root, "VEHICLE_SPEED");
    if(tmp)
    {
        rt_snprintf(hmi_cmd, sizeof(hmi_cmd) - 1, "speed.val=%d", (int)cJSON_GetNumberValue(tmp));
        hmi_send(hmi_cmd, rt_strlen(hmi_cmd));
        rt_memset(hmi_cmd, 0x00, sizeof(hmi_cmd));
    }
    
    tmp = cJSON_GetObjectItem(root, "Motor1_Angle");
    if(tmp)
    {
        rt_snprintf(hmi_cmd, sizeof(hmi_cmd) - 1, "degree.val=%d", (int)cJSON_GetNumberValue(tmp));
        hmi_send(hmi_cmd, rt_strlen(hmi_cmd));
        rt_memset(hmi_cmd, 0x00, sizeof(hmi_cmd));
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
    
    commuication_init();
    com_set_recv_callback(com_callback);

    while (count++)
    {
        rt_thread_mdelay(1000);
    }

    return RT_EOK;
}
