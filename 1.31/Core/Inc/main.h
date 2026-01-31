/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32f1xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */
// ========== LED 引脚定义 ==========
#define LED0_PIN        GPIO_PIN_5
#define LED0_GPIO_PORT  GPIOE

#define LED1_PIN        GPIO_PIN_5
#define LED1_GPIO_PORT  GPIOB

// ========== 蜂鸣器引脚定义 ==========
#define BUZZER_PIN        GPIO_PIN_8
#define BUZZER_GPIO_PORT  GPIOB

// ========== 按键引脚定义 ==========
#define KEY1_PIN        GPIO_PIN_3
#define KEY1_GPIO_PORT  GPIOE

#define KEY2_PIN        GPIO_PIN_4
#define KEY2_GPIO_PORT  GPIOE

#define KEY_UP_PIN      GPIO_PIN_0
#define KEY_UP_GPIO_PORT GPIOA

// ========== LED 控制宏 ==========
#define LED0_ON()       HAL_GPIO_WritePin(LED0_GPIO_PORT, LED0_PIN, GPIO_PIN_RESET)
#define LED0_OFF()      HAL_GPIO_WritePin(LED0_GPIO_PORT, LED0_PIN, GPIO_PIN_SET)
#define LED0_TOGGLE()   HAL_GPIO_TogglePin(LED0_GPIO_PORT, LED0_PIN)

#define LED1_ON()       HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_RESET)
#define LED1_OFF()      HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_SET)
#define LED1_TOGGLE()   HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_PIN)

// ========== 蜂鸣器控制宏 ==========
#define BUZZER_ON()     HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN, GPIO_PIN_SET)
#define BUZZER_OFF()    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN, GPIO_PIN_RESET)
#define BUZZER_TOGGLE() HAL_GPIO_TogglePin(BUZZER_GPIO_PORT, BUZZER_PIN)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
