#include <board.h>

#include "motor_driver.h"

#define DBG_TAG     "motor.drv"
#define DBG_LVL     DBG_LOG
#include <rtdbg.h>

static my_motor_driver_t motor[2];
pid_controller_t pid_config_default = {
	.Kp = 6.0,
	.Ki = 0.013,
	.Kd = 0.1,
	.integral = 0.0,
	.state = {0}
};

/* 接收数据回调函数 */
static rt_err_t angle_rx_call(rt_device_t dev, rt_size_t size)
{
    my_motor_driver_t *mtr = (my_motor_driver_t *)dev->user_data;
    
    rt_sem_release(&mtr->ic_rx);

    return RT_EOK;
}

rt_err_t motor_set_output(my_motor_driver_t *drv, rt_int32_t val)
{
    // 去除死区
	val = (val >= 0)? val + DEAD_ZONE_DUTY : val - DEAD_ZONE_DUTY;
	
	// 限位
	val = (val > 1000)? 1000 : val;
	val = (val < -1000)? -1000 : val;
	
	// 占空比值转换为定时器计数值
	rt_uint32_t output = (rt_uint32_t)((rt_uint64_t)(MOTOR_PWM_DUTY_RANGE - m_abs(val)) * (rt_uint64_t)MOTOR_PWM_PERIOD_NS / MOTOR_PWM_DUTY_RANGE);		
	
	// 输出
	if(val >= 0) {
        rt_pin_write(drv->rev_pin, PIN_HIGH);
        rt_pwm_set(drv->pwm_dev, drv->pwm_channel, MOTOR_PWM_PERIOD_NS, output);
	}
	else {
        rt_pin_write(drv->rev_pin, PIN_LOW);
        rt_pwm_set(drv->pwm_dev, drv->pwm_channel, MOTOR_PWM_PERIOD_NS, output);
	}
    return RT_EOK;
}

rt_err_t motor_get_angle(my_motor_driver_t *drv, float *angle)
{
    struct rt_input_capture_data data[2];
    rt_uint32_t angleHighTime, anglePeriod;
    
    rt_device_control(drv->angle_dev, INPUT_CAPTURE_CMD_CLEAR_BUF, RT_NULL);
    rt_device_control(drv->angle_dev, INPUT_CAPTURE_CMD_ENABLE, RT_NULL);
    
    if(RT_ETIMEOUT == rt_sem_take(&drv->ic_rx, 10)) {
        return -1;
    }
    rt_device_control(drv->angle_dev, INPUT_CAPTURE_CMD_DISABLE, RT_NULL);
    rt_device_read(drv->angle_dev, 0, data, 2);
    
    anglePeriod = data[0].pulsewidth_us + data[1].pulsewidth_us;
    if(data[0].is_high) {
        angleHighTime = data[0].pulsewidth_us;
    }
    else if(data[1].is_high) {
        angleHighTime = data[1].pulsewidth_us;
    }
    else {
        return -RT_ERROR;
    }
    
	*angle = (float)angleHighTime * 360 / anglePeriod;
    
    return RT_EOK;
}


void motor_ctrl_thread_entry(void *parameter)
{
    rt_ubase_t water_mark = 2;
    float angle1, angle2;
    float Err1, Err2;
    float output1, output2;
    
    rt_device_open(motor[0].angle_dev, RT_DEVICE_FLAG_RDWR);
    rt_device_control(motor[0].angle_dev, INPUT_CAPTURE_CMD_DISABLE, RT_NULL);
    rt_device_control(motor[0].angle_dev, INPUT_CAPTURE_CMD_SET_WATERMARK, (void *)&water_mark);
    rt_device_set_rx_indicate(motor[0].angle_dev, angle_rx_call);
    
    rt_device_open(motor[1].angle_dev, RT_DEVICE_FLAG_RDWR);
    rt_device_control(motor[1].angle_dev, INPUT_CAPTURE_CMD_DISABLE, RT_NULL);
    rt_device_control(motor[1].angle_dev, INPUT_CAPTURE_CMD_SET_WATERMARK, (void *)&water_mark);
    rt_device_set_rx_indicate(motor[1].angle_dev, angle_rx_call);
    
    while(1)
    {
        
        motor_get_angle(&motor[0], &angle1);
        Err1 = motor[0].expect_angle - angle1;
		if(Err1 > (float)180.0)			    Err1 = Err2 - (float)360.0;
		else if (Err1 < (float)-180.0)		Err1 = (float)360.0 + Err1;
		output1 = pid_calculate(&motor[0].pid, Err1);
        motor_set_output(&motor[0], (int)output1);
        
        motor_get_angle(&motor[1], &angle2);
        Err2 = motor[1].expect_angle - angle2;
		if(Err2 > (float)180.0)			    Err2 = Err2 - (float)360.0;
		else if (Err2 < (float)-180.0)		Err2 = (float)360.0 + Err2;
		output2 = pid_calculate(&motor[1].pid, Err2);
        motor_set_output(&motor[1], (int)output2);
        
        LOG_D("%d,%d", (int)angle1, (int)output1);
        
        rt_thread_delay(20);
    }
}

int motor_driver_init(void)
{
    rt_err_t result = RT_EOK;
    char dev_name[RT_NAME_MAX];
    rt_thread_t tid;
    
    rt_pin_mode(M_EN_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(M_EN_PIN, PIN_HIGH);
    
    rt_strncpy(dev_name, "pwm1", RT_NAME_MAX);
    motor[0].pwm_dev = motor[1].pwm_dev = (struct rt_device_pwm *)rt_device_find(dev_name);
    if (motor[0].pwm_dev == RT_NULL)
    {
        LOG_E("find %s failed!", dev_name);
        result = -RT_EIO;
        goto _exit;
    }
    motor[0].pwm_channel = 1;
    motor[1].pwm_channel = 3;
    
    motor[0].rev_pin = M1_REV_PIN;
    motor[1].rev_pin = M2_REV_PIN;
    rt_pin_mode(M1_REV_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(M2_REV_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(M1_REV_PIN, PIN_HIGH);
    rt_pin_write(M2_REV_PIN, PIN_HIGH);

    rt_strncpy(dev_name, "capture3", RT_NAME_MAX);
    motor[0].angle_dev = rt_device_find(dev_name);
    if (motor[0].angle_dev == RT_NULL)
    {
        LOG_E("find %s failed!", dev_name);
        result = -RT_EIO;
        goto _exit;
    }
    motor[0].angle_dev->user_data = (void *)&motor[0];
    
    rt_strncpy(dev_name, "capture4", RT_NAME_MAX);
    motor[1].angle_dev = rt_device_find(dev_name);
    if (motor[1].angle_dev == RT_NULL)
    {
        LOG_E("find %s failed!", dev_name);
        result = -RT_EIO;
        goto _exit;
    }
    motor[1].angle_dev->user_data = (void *)&motor[1];
    
    motor[0].pid = motor[1].pid = pid_config_default;
    
    motor[0].expect_angle = motor[1].expect_angle = 180;

    rt_sem_init(&motor[0].ic_rx, "m1_ic_rx", 0, RT_IPC_FLAG_FIFO);
    rt_sem_init(&motor[1].ic_rx, "m2_ic_rx", 0, RT_IPC_FLAG_FIFO);
    
    tid = rt_thread_create("m_ctl", 
                           motor_ctrl_thread_entry, 
                           (void *)RT_NULL, 
                           1024, 10, 10);
                           
    if (tid == RT_NULL)
    {
        LOG_E("%s thread create failed!", "m_ctl");
        result = -RT_ERROR;
        goto _exit;
    }
    rt_thread_startup(tid);
    
_exit:
    return result;
}
INIT_APP_EXPORT(motor_driver_init);


rt_err_t motor_enable(void)
{
    rt_pin_write(M_EN_PIN, PIN_LOW);
    if (motor[0].pwm_dev != RT_NULL)
    {
        rt_pwm_enable((struct rt_device_pwm *)motor[0].pwm_dev, motor[0].pwm_channel);
    }
    if (motor[1].pwm_dev != RT_NULL)
    {
        rt_pwm_enable((struct rt_device_pwm *)motor[1].pwm_dev, motor[1].pwm_channel);
    }
    return RT_EOK;
}

rt_err_t motor_disable(void)
{
//    rt_pin_write(M_EN_PIN, PIN_HIGH);
    if (motor[0].pwm_dev != RT_NULL)
    {
        rt_pwm_disable((struct rt_device_pwm *)motor[0].pwm_dev, motor[0].pwm_channel);
    }
    if (motor[1].pwm_dev != RT_NULL)
    {
        rt_pwm_disable((struct rt_device_pwm *)motor[1].pwm_dev, motor[1].pwm_channel);
    }
    return RT_EOK;
}

rt_err_t motor1_set_angle(float angle)
{
    motor[0].expect_angle = angle;
    return RT_EOK;
}

rt_err_t motor2_set_angle(float angle)
{
    motor[1].expect_angle = angle;
    return RT_EOK;
}

rt_err_t motor1_get_angle(float *angle)
{
    return motor_get_angle(&motor[0], angle);
}

rt_err_t motor2_get_angle(float *angle)
{
    return motor_get_angle(&motor[1], angle);
}

#include <finsh.h>
static int motor_sample(int argc, char **argv)
{
    int result = 0;
    
    if(rt_strcmp(argv[1], "enable") == 0) {
        motor_enable();
    }
    else if(rt_strcmp(argv[1], "disable") == 0) {
        motor_disable();
    }
    else if(rt_strcmp(argv[1], "set_out") == 0) {
        motor_set_output(&motor[atoi(argv[2])], atoi(argv[3]));
    }
    else if(rt_strcmp(argv[1], "set_angle") == 0) {
        if(atoi(argv[2]) == 0) {
            motor1_set_angle((float)atoi(argv[3]));
        }
        else if(atoi(argv[2]) == 1) {
            motor2_set_angle((float)atoi(argv[3]));
        }
    }

    return result;
}
MSH_CMD_EXPORT(motor_sample, motor sample);
