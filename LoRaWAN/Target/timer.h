/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    timer.h
  * @author  MCD Application Team
  * @brief   Wrapper to timer server
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIMER_H__
#define __TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32_timer.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/**
  * @brief Max timer mask
  */
#define TIMERTIME_T_MAX ( ( uint32_t )~0 )

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macro ------------------------------------------------------------*/
/**
  * @brief Timer value on 32 bits
  */
#define TimerTime_t UTIL_TIMER_Time_t

/**
  * @brief Timer object description
  */
#define TimerEvent_t UTIL_TIMER_Object_t

/**
  * @brief Create the timer object
  */
#define TimerInit(HANDLE, CB) do {\
                                   UTIL_TIMER_Create( HANDLE, TIMERTIME_T_MAX, UTIL_TIMER_ONESHOT, CB, NULL);\
                                 } while(0)

/**
  * @brief update the period and start the timer
  */
#define TimerSetValue(HANDLE, TIMEOUT) do{ \
                                           UTIL_TIMER_SetPeriod(HANDLE, TIMEOUT);\
                                         } while(0)

/**
  * @brief Start and adds the timer object to the list of timer events
  */
#define TimerStart(HANDLE)   do {\
                                  UTIL_TIMER_Start(HANDLE);\
                                } while(0)

/**
  * @brief Stop and removes the timer object from the list of timer events
  */
#define TimerStop(HANDLE)   do {\
                                 UTIL_TIMER_Stop(HANDLE);\
                               } while(0)

/**
  * @brief return the current time
  */
#define TimerGetCurrentTime  UTIL_TIMER_GetCurrentTime

/**
  * @brief return the elapsed time
  */
#define TimerGetElapsedTime UTIL_TIMER_GetElapsedTime

/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __TIMER_H__*/
