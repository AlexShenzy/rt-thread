#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include "stm32f4xx_hal.h"

#define CALCULATE_PRECISION 1.0 	//PID计算精度，单位：度

typedef struct pid_controller
{
	float integral; /**< The integral . */
	float state[3]; /**< The state array of length 3. */
	float Kp; /**< The proportional gain. */
	float Ki; /**< The integral gain. */
	float Kd; /**< The derivative gain. */
} pid_controller_t;


//extern void pid_Init(void);
extern float pid_calculate(pid_controller_t *pid, float err);


#endif


