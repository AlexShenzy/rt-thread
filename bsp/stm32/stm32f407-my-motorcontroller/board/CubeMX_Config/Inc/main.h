/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define M1_Speed_Pin GPIO_PIN_3
#define M1_Speed_GPIO_Port GPIOA
#define CAN1_ERR_Pin GPIO_PIN_5
#define CAN1_ERR_GPIO_Port GPIOA
#define CAN1_EN_Pin GPIO_PIN_6
#define CAN1_EN_GPIO_Port GPIOA
#define CAN1_STB_Pin GPIO_PIN_7
#define CAN1_STB_GPIO_Port GPIOA
#define M_EN_Pin GPIO_PIN_4
#define M_EN_GPIO_Port GPIOC
#define M1_Angle_Pin GPIO_PIN_0
#define M1_Angle_GPIO_Port GPIOB
#define M1_OUT_Pin GPIO_PIN_9
#define M1_OUT_GPIO_Port GPIOE
#define M2_Speed_Pin GPIO_PIN_10
#define M2_Speed_GPIO_Port GPIOB
#define CAN2_STB_Pin GPIO_PIN_14
#define CAN2_STB_GPIO_Port GPIOB
#define M1_REV_Pin GPIO_PIN_15
#define M1_REV_GPIO_Port GPIOB
#define RS232_TX_Pin GPIO_PIN_8
#define RS232_TX_GPIO_Port GPIOD
#define RS232_RX_Pin GPIO_PIN_9
#define RS232_RX_GPIO_Port GPIOD
#define M2_Angle_Pin GPIO_PIN_12
#define M2_Angle_GPIO_Port GPIOD
#define M2_REV_Pin GPIO_PIN_14
#define M2_REV_GPIO_Port GPIOD
#define UART_TX_Pin GPIO_PIN_6
#define UART_TX_GPIO_Port GPIOC
#define UART_RX_Pin GPIO_PIN_7
#define UART_RX_GPIO_Port GPIOC
#define BT_RST_Pin GPIO_PIN_8
#define BT_RST_GPIO_Port GPIOA
#define BT_STA_Pin GPIO_PIN_9
#define BT_STA_GPIO_Port GPIOA
#define M2_OUT_Pin GPIO_PIN_10
#define M2_OUT_GPIO_Port GPIOA
#define BT_TX_Pin GPIO_PIN_10
#define BT_TX_GPIO_Port GPIOC
#define BT_RX_Pin GPIO_PIN_11
#define BT_RX_GPIO_Port GPIOC
#define JDY_S7_Pin GPIO_PIN_12
#define JDY_S7_GPIO_Port GPIOC
#define JDY_S6_Pin GPIO_PIN_0
#define JDY_S6_GPIO_Port GPIOD
#define JDY_S5_Pin GPIO_PIN_1
#define JDY_S5_GPIO_Port GPIOD
#define JDY_S8_Pin GPIO_PIN_2
#define JDY_S8_GPIO_Port GPIOD
#define JDY_S4_Pin GPIO_PIN_7
#define JDY_S4_GPIO_Port GPIOD
#define JDY_S3_Pin GPIO_PIN_3
#define JDY_S3_GPIO_Port GPIOB
#define JDY_S2_Pin GPIO_PIN_4
#define JDY_S2_GPIO_Port GPIOB
#define JDY_S1_Pin GPIO_PIN_5
#define JDY_S1_GPIO_Port GPIOB
#define JDY_TX_Pin GPIO_PIN_6
#define JDY_TX_GPIO_Port GPIOB
#define JDY_RX_Pin GPIO_PIN_7
#define JDY_RX_GPIO_Port GPIOB
#define JDY_SET_Pin GPIO_PIN_0
#define JDY_SET_GPIO_Port GPIOE
#define JDY_CS_Pin GPIO_PIN_1
#define JDY_CS_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
