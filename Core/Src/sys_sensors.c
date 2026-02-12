/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    sys_sensors.c
 * @author  MCD Application Team
 * @brief   Manages the sensors on the application
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

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "platform.h"
#include "sys_conf.h"
#include "sys_sensors.h"
#if defined (SENSOR_ENABLED) && (SENSOR_ENABLED == 0)
#include "adc_if.h"
#endif /* SENSOR_ENABLED */

/* USER CODE BEGIN Includes */

#include "i2c.h"

#include "tim.h"

#include "ms8607.h"
#include "as3935.h"
#include "cmsis_os2.h"
#include "sys_app.h"

/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/

/* USER CODE BEGIN PD */
#define STSOP_LATTITUDE           ((float) 43.618622 )  /*!< default latitude position */
#define STSOP_LONGITUDE           ((float) 7.051415  )  /*!< default longitude position */
#define MAX_GPS_POS               ((int32_t) 8388607 )  /*!< 2^23 - 1 */
#define HUMIDITY_DEFAULT_VAL      50.0f                 /*!< default humidity */
#define TEMPERATURE_DEFAULT_VAL   18.0f                 /*!< default temperature */
#define PRESSURE_DEFAULT_VAL      1000.0f               /*!< default pressure */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

#if	LIS3DH_ENABLED
stmdev_ctx_t lis3dh_ctx;
#endif

/*
 * MS8607 present and initialized
 */
bool ms8607_present = false;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

static uint8_t as3935ReadReg(uint8_t reg);
static void as3935WriteReg(uint8_t reg, uint8_t val);
static void as3935UseTimer17(void);
static void as3935UseInterrupt(void);

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
int32_t EnvSensors_Read(sensor_t *sensor_data)
{
  /* USER CODE BEGIN EnvSensors_Read */
	float HUMIDITY_Value = HUMIDITY_DEFAULT_VAL;
	float TEMPERATURE_Value = TEMPERATURE_DEFAULT_VAL;
	float PRESSURE_Value = PRESSURE_DEFAULT_VAL;
	float tempHumidity;

	sensor_data->latitude = (int32_t) ((STSOP_LATTITUDE * MAX_GPS_POS) / 90);
	sensor_data->longitude = (int32_t) ((STSOP_LONGITUDE * MAX_GPS_POS) / 180);

	if (ms8607_present) {
		ms8607_read_temperature_pressure_humidity(&TEMPERATURE_Value,
				&PRESSURE_Value, &tempHumidity);
		ms8607_get_compensated_humidity(TEMPERATURE_Value, tempHumidity,
				&HUMIDITY_Value);
	}

	sensor_data->humidity = HUMIDITY_Value;
	sensor_data->temperature = TEMPERATURE_Value;
	sensor_data->pressure = PRESSURE_Value;
	HAL_Delay(2);
	sensor_data->as3935_status = as3935ReadReg(0x03);
	sensor_data->as3935_distance = as3935ReadReg(0x07) & 0x3f;

	return 0;
  /* USER CODE END EnvSensors_Read */
}

int32_t EnvSensors_Init(void)
{
  int32_t ret = 0;
  /* USER CODE BEGIN EnvSensors_Init */



	if (ms8607_is_connected()) {
		// initialize MS8607 and remember it

		ms8607_init();	// no error return
		ms8607_present = (ms8607_reset() == ms8607_status_ok);
	}

	// XXX: initialize the AS3935
  /* USER CODE END EnvSensors_Init */
  return ret;
}

/* USER CODE BEGIN EF */

/*
 * tune antenna
 */
int32_t
as3935TuneAntenna(void)
{
	uint32_t startCount, endCount, errorValue, lastErrorValue;
	int16_t errorCount;
	uint8_t tuneValue, r8, r3;
	const uint32_t targetCount = (48000000ULL * 512) /  500000ULL;

	// set the AS3935 for divide by 64 output
	// using TIM17, measure period of AS3935 output
	// TIM17 is dividing by 8, so result is fAntenna / 512
	// divide count by 2 and use as error value; iterate over range 0..15
	// for minimum error

	as3935UseTimer17();

	r3 = as3935ReadReg(0x03);
	r3 = (r3 & 0x3f) | 0x80;
	as3935WriteReg(0x03, r3);

	r8 = as3935ReadReg(0x08);
	r8 = r8 | 0x80;	// enable internal clock out on INTR

	// start TIM17 for measuring; clocked at 48MHz +/- 0.5%
	HAL_TIM_IC_Start(&htim17, TIM_CHANNEL_1);

	lastErrorValue = 1000000;
	for (tuneValue = 0; tuneValue < 16; tuneValue++) {
		r8 = (r8 & 0xf0) | tuneValue;
		as3935WriteReg(0x08, r8);

		// discard potentially stale capture
		// give antenna tuning a chance to settle down
		while (!(htim17.Instance->SR & 2))
				;
		startCount = HAL_TIM_ReadCapturedValue(&htim17, TIM_CHANNEL_1);

		// read beginning of cycle
		while (!(htim17.Instance->SR & 2))
				;
		startCount = HAL_TIM_ReadCapturedValue(&htim17, TIM_CHANNEL_1);

		// spin waiting for full cycle to complete
		while (!(htim17.Instance->SR & 2))
				;
		endCount = HAL_TIM_ReadCapturedValue(&htim17, TIM_CHANNEL_1);
		errorCount = endCount - startCount;

		// divide count by 2 to drop off indeterminate LSB
		errorValue = (targetCount - errorCount) >> 1;
		if (errorValue > lastErrorValue) {
			// done, break
      break;
  }
		lastErrorValue = errorValue;
	}

	// stop TIM17 and turn off AS3935 clock output
	HAL_TIM_IC_Stop(&htim17, TIM_CHANNEL_1);
	r8 &= ~0x80;
	as3935WriteReg(0x08, r8);
	as3935UseInterrupt();
	return (0);
}



void
as3935Init(void)
{
	uint8_t r;

	as3935WriteReg(0x3c, 0x96);	// preset default
	r = as3935ReadReg(0x01);
	r = (r & 0xf0) | 0x04;	// set WDTH
	as3935WriteReg(0x01, r);
	as3935TuneAntenna();
	as3935WriteReg(0x3d, 0x96);	// Calib RCOs
	HAL_Delay(2);
	r = as3935ReadReg(0x08);

	APP_LOG(TS_OFF, VLEVEL_M, "R8: %x\r\n", r);

	as3935UseTimer17();
	as3935WriteReg(0x08, r | 0x40);
	HAL_Delay(3);
	as3935WriteReg(0x08, r & ~0x40);
	HAL_Delay(3);
	r = as3935ReadReg(0x03);

	as3935UseInterrupt();

	r = as3935ReadReg(0x3a);
	APP_LOG(TS_OFF, VLEVEL_M, "TRCO: %x\r\n", r);

	r = as3935ReadReg(0x3b);
	APP_LOG(TS_OFF, VLEVEL_M, "SRCO: %x\r\n", r);
  }



/*
 * External Interrupt callback
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  extern osThreadId_t Thd_LoraSendProcessId;

  switch (GPIO_Pin) {
  case  GPIO_PIN_7:
	  osThreadFlagsSet(Thd_LoraSendProcessId, 2);
      break;

    default:
      break;
  }

}

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/
/* USER CODE BEGIN PrFD */

/*
 *
 */
#define	AS3935_I2C_ADDR	3

/*

 *
 */
static uint8_t
as3935ReadReg(uint8_t reg)
{
	uint8_t data_rd;

	// XXX: need to process HAL_ERROR return
	(void) HAL_I2C_Mem_Read(&hi2c1, AS3935_I2C_ADDR << 1, reg,
			I2C_MEMADD_SIZE_8BIT, &data_rd, 1, 1000);

    return (data_rd);
}

/*

 *
 */
static void
as3935WriteReg(uint8_t reg, uint8_t val)
{
	// XXX: need to process HAL_ERROR return
	HAL_I2C_Mem_Write(&hi2c1, AS3935_I2C_ADDR << 1, reg,
	  I2C_MEMADD_SIZE_8BIT, &val, 1, 1000);
}


/*
 * Disable AS3935 INTR, route to TIM17 for frequency measurement
 */
static void
as3935UseTimer17(void)
{
	  GPIO_InitTypeDef GPIO_InitStruct = {0};

	  HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
	  __HAL_RCC_GPIOA_CLK_ENABLE();
	  GPIO_InitStruct.Pin = GPIO_PIN_7;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  GPIO_InitStruct.Alternate = GPIO_AF14_TIM17;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
/*
 * Enable AS3935 INT, un-route to TIM17
 */
static void
as3935UseInterrupt(void)
{
	  GPIO_InitTypeDef GPIO_InitStruct = {0};

	  /*Configure GPIO pin : PA7 */
	  GPIO_InitStruct.Pin = GPIO_PIN_7;
	  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /* EXTI interrupt init*/
	  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
	  HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
	  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/* USER CODE END PrFD */
