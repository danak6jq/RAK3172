/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    sys_debug.c
  * @author  MCD Application Team
  * @brief   Enables 4 debug pins for internal signals RealTime debugging
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "sys_debug.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/

/**
  * @brief Initializes the SW probes pins and the monitor RF pins via Alternate Function
  */
void DBG_Init(void)
{
  /* USER CODE BEGIN DBG_Init_1 */

  /* USER CODE END DBG_Init_1 */

  /* SW probes */
#if defined (DEBUGGER_ENABLED) && ( DEBUGGER_ENABLED == 0 )
  HAL_DBGMCU_DisableDBGSleepMode();
  HAL_DBGMCU_DisableDBGStopMode();
  HAL_DBGMCU_DisableDBGStandbyMode();
#elif defined (DEBUGGER_ENABLED) && ( DEBUGGER_ENABLED == 1 )
  /*Debug power up request wakeup CBDGPWRUPREQ*/
  LL_EXTI_EnableIT_32_63(LL_EXTI_LINE_46);
  /* Disabled HAL_DBGMCU_  */
  HAL_DBGMCU_EnableDBGSleepMode();
  HAL_DBGMCU_EnableDBGStopMode();
  HAL_DBGMCU_EnableDBGStandbyMode();
#elif !defined (DEBUGGER_ENABLED)
#error "DEBUGGER_ENABLED not defined or out of range <0,1>"
#endif /* DEBUGGER_OFF */

#if (DEBUG_SUBGHZSPI_MONITORING_ENABLED == 1) || \
    (DEBUG_RF_NRESET_ENABLED == 1) || \
    (DEBUG_RF_HSE32RDY_ENABLED == 1) || \
    (DEBUG_RF_SMPSRDY_ENABLED == 1) || \
    (DEBUG_RF_LDORDY_ENABLED == 1) || \
    (DEBUG_RF_DTB1_ENABLED == 1) || \
    (DEBUG_RF_BUSY_ENABLED == 1)
  GPIO_InitTypeDef  GPIO_InitStruct = {0};
#endif

  /* USER CODE BEGIN DBG_Init_2 */

  /* USER CODE END DBG_Init_2 */

  /* HW alternate functions for monitoring RF */

#if (DEBUG_SUBGHZSPI_MONITORING_ENABLED == 1)
  /*spi dbg*/
  GPIO_InitStruct.Pin    = (GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
  GPIO_InitStruct.Mode   = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull   = GPIO_NOPULL;
  GPIO_InitStruct.Speed  = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate  = GPIO_AF13_DEBUG_SUBGHZSPI;
  __HAL_RCC_GPIOA_CLK_ENABLE();
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#endif /* DEBUG_SUBGHZSPI_MONITORING_ENABLED */

#if (DEBUG_RF_NRESET_ENABLED == 1)
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF13_DEBUG_RF;
  __HAL_RCC_GPIOA_CLK_ENABLE();
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#endif /* DEBUG_RF_NRESET_ENABLED */

#if (DEBUG_RF_HSE32RDY_ENABLED == 1)
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF13_DEBUG_RF;
  __HAL_RCC_GPIOA_CLK_ENABLE();
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#endif /* DEBUG_RF_HSE32RDY_ENABLED */

#if (DEBUG_RF_SMPSRDY_ENABLED == 1)
  GPIO_InitStruct.Pin    = (GPIO_PIN_2);
  GPIO_InitStruct.Mode   = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull   = GPIO_NOPULL;
  GPIO_InitStruct.Speed  = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate  = GPIO_AF13_DEBUG_RF;
  __HAL_RCC_GPIOB_CLK_ENABLE();
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif /* DEBUG_RF_SMPSRDY_ENABLED */

#if (DEBUG_RF_LDORDY_ENABLED == 1)
  GPIO_InitStruct.Pin    = (GPIO_PIN_4);
  GPIO_InitStruct.Mode   = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull   = GPIO_NOPULL;
  GPIO_InitStruct.Speed  = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate  = GPIO_AF13_DEBUG_RF;
  __HAL_RCC_GPIOB_CLK_ENABLE();
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif /* DEBUG_RF_LDORDY_ENABLED */

#if (DEBUG_RF_DTB1_ENABLED == 1)
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF13_DEBUG_RF;
  __HAL_RCC_GPIOB_CLK_ENABLE();
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif /* DEBUG_RF_DTB1_ENABLED */

#if (DEBUG_RF_BUSY_ENABLED == 1)
  /* Busy */
  GPIO_InitStruct.Pin    = (GPIO_PIN_12);
  GPIO_InitStruct.Mode   = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull   = GPIO_NOPULL;
  GPIO_InitStruct.Speed  = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate  = GPIO_AF6_RF_BUSY;
  __HAL_RCC_GPIOA_CLK_ENABLE() ;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#endif /* DEBUG_RF_BUSY_ENABLED */

  /* USER CODE BEGIN DBG_Init_3 */

  /* USER CODE END DBG_Init_3 */
}

/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/

/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */
