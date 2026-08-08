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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TRACK5_Pin GPIO_PIN_2
#define TRACK5_GPIO_Port GPIOE
#define TRACK8_Pin GPIO_PIN_3
#define TRACK8_GPIO_Port GPIOE
#define TRACK7_Pin GPIO_PIN_4
#define TRACK7_GPIO_Port GPIOE
#define TRACK9_Pin GPIO_PIN_6
#define TRACK9_GPIO_Port GPIOE
#define TRACK10_Pin GPIO_PIN_13
#define TRACK10_GPIO_Port GPIOC
#define TRACK6_Pin GPIO_PIN_0
#define TRACK6_GPIO_Port GPIOC
#define TRACK3_Pin GPIO_PIN_1
#define TRACK3_GPIO_Port GPIOC
#define TRACK4_Pin GPIO_PIN_2
#define TRACK4_GPIO_Port GPIOC
#define TRACK1_Pin GPIO_PIN_3
#define TRACK1_GPIO_Port GPIOC
#define PWMA_Pin GPIO_PIN_5
#define PWMA_GPIO_Port GPIOA
#define PWMB_Pin GPIO_PIN_6
#define PWMB_GPIO_Port GPIOA
#define BT_STATE_Pin GPIO_PIN_4
#define BT_STATE_GPIO_Port GPIOC
#define BT_EN_Pin GPIO_PIN_5
#define BT_EN_GPIO_Port GPIOC
#define AIN1_Pin GPIO_PIN_8
#define AIN1_GPIO_Port GPIOE
#define AIN2_Pin GPIO_PIN_9
#define AIN2_GPIO_Port GPIOE
#define BIN1_Pin GPIO_PIN_10
#define BIN1_GPIO_Port GPIOE
#define BIN2_Pin GPIO_PIN_11
#define BIN2_GPIO_Port GPIOE
#define buzzer_Pin GPIO_PIN_14
#define buzzer_GPIO_Port GPIOB
#define TRACK11_Pin GPIO_PIN_8
#define TRACK11_GPIO_Port GPIOC
#define TRACK12_Pin GPIO_PIN_3
#define TRACK12_GPIO_Port GPIOD
#define TRACK2_Pin GPIO_PIN_1
#define TRACK2_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
