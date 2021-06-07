#include <board.h>
#include "obd_adapter.h"
#include "cJSON_Utils.h"

#define DBG_TAG     "obd.drv"
#define DBG_LVL     DBG_INFO
#include <rtdbg.h>

static my_obd_t *obd;

/* 接收数据回调函数 */
static rt_err_t obd_rx_call(rt_device_t dev, rt_size_t size)
{
    /* CAN 接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&obd->rx_sem);

    return RT_EOK;
}

static rt_err_t obd_set_filter()
{
    struct rt_can_filter_item items[1] =
    {
        {CAN_STD_ID(0x7E8), 0, 0, 0, 0xFFFFFFFF, 0, RT_NULL, RT_NULL}
    };
    struct rt_can_filter_config cfg = {1, 1, items}; /* 一共有 1 个过滤表 */
    
    if(obd == RT_NULL || obd->device == RT_NULL)
    {
        return -RT_EIO;
    }
    
    /* 设置硬件过滤表 */
    rt_device_control(obd->device, RT_CAN_CMD_SET_FILTER, &cfg);
    
    return RT_EOK;
}

static rt_err_t obd_send_to_ecu(rt_uint8_t *data, rt_size_t length)
{
    struct rt_can_msg   msg = {0};
    rt_size_t           size;
    
    if (obd->device == RT_NULL)
    {
        LOG_D("No OBD device specified\n");
        return -RT_EIO;
    }
    
    msg.id = 0x7DF;
    msg.len = 8;
    size = length > msg.len ? msg.len : length;
    rt_memcpy(msg.data, data, size);
    
    /* 发送一帧 CAN 数据 */
    size = rt_device_write(obd->device, 0, &msg, sizeof(msg));
    if (size == 0)
    {
        LOG_D("%s dev write data failed!\n", obd->device->parent.name);
        return -RT_ERROR;
    }
    return RT_EOK;
}

rt_int32_t obd_query_pid(rt_uint8_t pid)
{
    rt_err_t res;
    rt_uint8_t cmd[] = {0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    struct rt_can_msg rxmsg = {0};
    
    cmd[2] = pid;
    res = obd_send_to_ecu(cmd, sizeof(cmd));
    if(res != RT_EOK)
    {
        return -1;
    }
    
    /* hdr 值为 - 1，表示直接从 uselist 链表读取数据 */
    rxmsg.hdr = -1;
    /* 阻塞等待接收信号量 */
    if(RT_ETIMEOUT == rt_sem_take(&obd->rx_sem, 500)) {
        return -1;
    }
    
    /* 从 CAN 读取一帧数据 */
    res = rt_device_read(obd->device, 0, &rxmsg, sizeof(rxmsg));
    if(res == 0 || rxmsg.data[1] != 0x41 || rxmsg.data[2] != pid)
    {
        return -1;
    }
    
    return ((rt_int32_t)rxmsg.data[3] << 8 | rxmsg.data[4]);
}

#define CAN1_EN_PIN  GET_PIN(A, 6)
#define CAN1_STB_PIN GET_PIN(A, 7)
int obd_init(void)
{
    char can_name[RT_NAME_MAX];
    
    if(obd)
    {
        LOG_D("OBD already init.\n");
        return RT_EOK;
    }
    
    obd = (my_obd_t *)rt_calloc(1, sizeof(my_obd_t));
    if (obd == RT_NULL)
    {
        LOG_D("no memory for OBD\n");
        return -RT_ENOMEM;
    }                        
    
    rt_strncpy(can_name, OBD_DEV_NAME, RT_NAME_MAX);
    
    /* 查找系统中的 CAN 设备 */
    if(obd->device == RT_NULL)
    {
        obd->device = rt_device_find(can_name);
        if (obd->device == RT_NULL)
        {
            LOG_D("find %s failed!\n", can_name);
            return -RT_EIO;
        }
    }

    rt_pin_mode(CAN1_EN_PIN,    PIN_MODE_OUTPUT);
    rt_pin_mode(CAN1_STB_PIN,   PIN_MODE_OUTPUT);
    rt_pin_write(CAN1_EN_PIN,   PIN_HIGH);
    rt_pin_write(CAN1_STB_PIN,  PIN_HIGH);
	
    /* 以中断接收及发送方式打开 CAN 设备 */
    rt_device_open(obd->device, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
    
    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(obd->device, obd_rx_call);
    
    /* 初始化信号量 */
    rt_sem_init(&obd->rx_sem, "obd_rx", 0, RT_IPC_FLAG_FIFO);
    
    /* 设置 CAN 设备过滤器 */
    obd_set_filter();
    
    return RT_EOK;
}

#include "communication.h"
#include "cJSON_Utils.h"
#include "motor_driver.h"
void obd_update_thread_entry(void *parameter)
{
    rt_int32_t val;
    cJSON *root = RT_NULL;
//    rt_bool_t flag = RT_TRUE;
//    rt_int32_t tmp = 0;
    float angle;
    
    while(1)
    {
//        if(flag)
//        {
//            tmp += 0x1000;
//            if(tmp >= 0xFFFF)  flag = RT_FALSE;
//        }
//        else
//        {
//            tmp -= 0x1000;
//            if(tmp <= 0)  flag = RT_TRUE;
//        }
        root = cJSON_CreateObject();
//		LOG_RAW("\x1b[2J\x1b[H");
//		LOG_RAW("======= Diagnostic Services Info =======\n");
        val = obd_query_pid(ENGINE_LOAD);
//        val = tmp;
		if(val != -1)
		{
			val = (val >> 8) * 100 / 0xFF;
            cJSON_AddNumberToObject(root, "ENGINE_LOAD", val);
//			LOG_RAW("ENGINE_LOAD : %d %%\n", val);
		}
		
		val = obd_query_pid(ENGINE_COOLANT_TEMP);
//        val = tmp;
		if(val != -1)
		{
			val = (val >> 8) - 40;
            cJSON_AddNumberToObject(root, "ENGINE_COOLANT_TEMP", val);
//			LOG_RAW("ENGINE_COOLANT_TEMP : %d `C\n", val);
		}
        
		val = obd_query_pid(INTAKE_MANIFOLD_ABS_PRESSURE);
//        val = tmp;
		if(val != -1)
		{
			val = val >> 8;
            cJSON_AddNumberToObject(root, "INTAKE_MANIFOLD_ABS_PRESSURE", val);
//			LOG_RAW("INTAKE_MANIFOLD_ABS_PRESSURE : %d kPa\n", val);
		}
        
		val = obd_query_pid(ENGINE_RPM);
//        val = tmp;
		if(val != -1) 
		{
			val = val / 4;
            cJSON_AddNumberToObject(root, "ENGINE_RPM", val);
//			LOG_RAW("ENGINE_RPM : %d r/min\n", val);
		}

		val = obd_query_pid(VEHICLE_SPEED);
//        val = tmp;
		if(val != -1)
		{
			val = val >> 8;
            cJSON_AddNumberToObject(root, "VEHICLE_SPEED", val);
//			LOG_RAW("VEHICLE_SPEED : %d km/h\n", val);
		}
        
		rt_kprintf("========================================");
        
        if(RT_EOK == motor1_get_angle(&angle)) {
            cJSON_AddNumberToObject(root, "Motor1_Angle", (int)angle);
//			LOG_RAW("Motor1_Angle : %d km/h\n", (int)angle);
        }
        
        if(RT_EOK == motor2_get_angle(&angle)) {
            cJSON_AddNumberToObject(root, "Motor2_Angle", (int)angle);
//			LOG_RAW("Motor2_Angle : %d km/h\n", (int)angle);
        }

        char *output = cJSON_PrintUnformatted(root);
//        com_send(output, rt_strlen(output));
//        com_send("\r\n", 2);
		rt_thread_mdelay(100);
        
        if(root)
        {
            cJSON_Delete(root);
            root = RT_NULL;
        }
        if(output)
        {
            rt_free(output);
            output = RT_NULL;
        }
    }
}

int obd_update_init(void)
{
    rt_err_t res;
    rt_thread_t tid;
    
    res = obd_init();
    if (res != RT_EOK)
    {
        return -res;
    }
    
    tid = rt_thread_create("obd", 
                           (void (*)(void *parameter))obd_update_thread_entry, 
                           (void *)RT_NULL, 
                           OBD_UP_THREAD_STACK_SIZE, OBD_UP_THREAD_PRIORITY, 10);
                           
    if (tid == RT_NULL)
    {
        return -RT_ERROR;
    }
    rt_thread_startup(tid);
    
    return RT_EOK;
}
INIT_APP_EXPORT(obd_update_init);
