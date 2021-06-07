#ifndef __MOTOR_DRIVER_H__
#define __MOTOR_DRIVER_H__

#include <rtthread.h>
#include <rtdevice.h>

#include "pid_controller.h"

//#ifndef MOTOR_DRV_NAME
//#define MOTOR_DRV_NAME                "motor"
//#endif
//#ifndef MOTOR_DRV_THREAD_PRIORITY
//#define MOTOR_DRV_THREAD_PRIORITY      20
//#endif
//#ifndef MOTOR_DRV_THREAD_STACK_SIZE
//#define MOTOR_DRV_THREAD_STACK_SIZE    1024
//#endif

#define m_abs(x)	((x)>0?(x):-(x))

#define DEAD_ZONE_DUTY	        80      //电机死区占空比
#define MOTOR_PWM_PERIOD_NS     50000UL   //pwm波周期，50us=20kHZ
#define MOTOR_PWM_DUTY_RANGE    1000UL    //pwm波占空比范围，0‰-1000‰


#define M_EN_PIN    GET_PIN(C, 4)
#define M1_REV_PIN  GET_PIN(B, 15)
#define M2_REV_PIN  GET_PIN(D, 14)

typedef struct my_motor_driver
{
    struct rt_device_pwm    *pwm_dev;
    rt_uint32_t             pwm_channel;
    rt_base_t               rev_pin;
    
    rt_device_t             angle_dev;
    struct rt_semaphore     ic_rx;
    
    pid_controller_t        pid;
    float                   expect_angle;
} my_motor_driver_t;


rt_err_t motor_enable(void);
rt_err_t motor_disable(void);
rt_err_t motor1_set_angle(float angle);
rt_err_t motor2_set_angle(float angle);
rt_err_t motor1_get_angle(float *angle);
rt_err_t motor2_get_angle(float *angle);

#endif
