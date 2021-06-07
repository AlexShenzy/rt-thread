#include "pid_controller.h"


float pid_calculate(pid_controller_t *pid, float err)
{	
	//忽略低于精度值的偏差
	if(err < (float)CALCULATE_PRECISION && err > (float)-CALCULATE_PRECISION) {
		err = 0.0;
	}

	// 计算积分量，并限位
	pid->integral += err;
	pid->integral = (pid->integral > (float)10000.0) ? (float)10000.0 : pid->integral;
	pid->integral = (pid->integral < (float)-10000.0) ? (float)-10000.0 : pid->integral;
	
	// 记录偏差，用来求微分量
	pid->state[2] = pid->state[1];
	pid->state[1] = pid->state[0];
	pid->state[0] = err;
	
	return pid->Kp * err + pid->Ki * pid->integral - pid->Kd * (pid->state[1] - pid->state[0]);
//	float res = pid->Kp * err + pid->Ki * pid->integral + pid->Kd * (pid->state[1] - pid->state[0]);
//	return (res < (float)3.0 && res > (float)-3.0) ? (float)0.0 : res;
}

