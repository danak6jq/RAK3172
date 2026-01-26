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
#include "lis3dh_reg.h"
#include "cmsis_os2.h"

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

stmdev_ctx_t lis3dh_ctx;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

// configure LIS3DH
static void lis3dh_setReg();

// LIS3DH interrupt handler
static void lis3dh_InterruptHandler();

static int32_t
lis3dh_platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len);

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t
lis3dh_platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);


/*
 * @brief  platform specific delay (platform dependent)
 *
 * @param  ms        delay in ms
 *
 */
static void
lis3dh_platform_delay(uint32_t ms);

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
int32_t EnvSensors_Read(sensor_t *sensor_data)
{
  /* USER CODE BEGIN EnvSensors_Read */
  float HUMIDITY_Value = HUMIDITY_DEFAULT_VAL;
  float TEMPERATURE_Value = TEMPERATURE_DEFAULT_VAL;
  float PRESSURE_Value = PRESSURE_DEFAULT_VAL;

  // XXX:
  TEMPERATURE_Value = (SYS_GetTemperatureLevel() >> 8);

  sensor_data->humidity    = HUMIDITY_Value;
  sensor_data->temperature = TEMPERATURE_Value;
  sensor_data->pressure    = PRESSURE_Value;

  sensor_data->latitude  = (int32_t)((STSOP_LATTITUDE  * MAX_GPS_POS) / 90);
  sensor_data->longitude = (int32_t)((STSOP_LONGITUDE  * MAX_GPS_POS) / 180);

  return 0;
  /* USER CODE END EnvSensors_Read */
}

int32_t EnvSensors_Init(void)
{
  int32_t ret = 0;
  /* USER CODE BEGIN EnvSensors_Init */
  lis3dh_reg_t reg;

  /* Initialize mems driver interface */
  lis3dh_ctx.write_reg = lis3dh_platform_write;
  lis3dh_ctx.read_reg = lis3dh_platform_read;
  lis3dh_ctx.mdelay = lis3dh_platform_delay;
  lis3dh_ctx.handle = &hi2c2;

  /* Wait sensor boot time */
  lis3dh_platform_delay(5);

  /* Check device ID */
  lis3dh_device_id_get(&lis3dh_ctx, &reg.byte);

  // Initialize LIS3DH as motion detector
  lis3dh_setReg();

  /* USER CODE END EnvSensors_Init */
  return ret;
}

/* USER CODE BEGIN EF */

/*
 * External Interrupt callback
 * Only EXTI used on RAK2270 is for the LIS3DH, so it's best to locate this here
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch (GPIO_Pin) {
  case  LIS3D_INT1_Pin:
	  // Only need to call one interrupt handler for LIS3DH
	  // (both INT1 and INT2 come in on the same IRQ vector)
	  lis3dh_InterruptHandler();
      break;

  case  LIS3D_INT2_Pin:
	  // check to see if source is active
      break;

    default:
      break;
  }
}

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/
/* USER CODE BEGIN PrFD */


/*
 * Generally taken from RAK2270 firmware
 */
static void
lis3dh_setReg()
{
  uint16_t md_threshold = 400;	// 400mg XXX: make configurable
  uint16_t md_sample_rate = LIS3DH_ODR_1Hz;	// XXX: make configurable
  lis3dh_reg_t lis3dh_reg;

  lis3dh_operating_mode_set(&lis3dh_ctx, LIS3DH_HR_12bit);
  lis3dh_data_rate_set(&lis3dh_ctx, md_sample_rate);
  lis3dh_high_pass_int_conf_set(&lis3dh_ctx, LIS3DH_ON_INT1_GEN);
  lis3dh_high_pass_on_outputs_set(&lis3dh_ctx, PROPERTY_ENABLE);

  lis3dh_pin_int1_config_get(&lis3dh_ctx, &lis3dh_reg.ctrl_reg3);
  lis3dh_reg.ctrl_reg3.i1_ia1 = PROPERTY_ENABLE;
  lis3dh_pin_int1_config_set(&lis3dh_ctx, &lis3dh_reg.ctrl_reg3);

  // Forgive me, I love the ?: construct
  lis3dh_full_scale_set(&lis3dh_ctx,
		  md_threshold >= 8000 ? LIS3DH_16g :
		  md_threshold >= 4000 ? LIS3DH_8g :
		  md_threshold >= 2000 ? LIS3DH_4g : LIS3DH_2g);

  lis3dh_int1_pin_notification_mode_set(&lis3dh_ctx, LIS3DH_INT1_LATCHED);

  lis3dh_int1_gen_conf_get(&lis3dh_ctx, &lis3dh_reg.int1_cfg);
  lis3dh_reg.int1_cfg.xhie = PROPERTY_ENABLE;
  lis3dh_reg.int1_cfg.yhie = PROPERTY_ENABLE;
  lis3dh_reg.int1_cfg.zhie = PROPERTY_ENABLE;
  lis3dh_int1_gen_conf_set(&lis3dh_ctx, &lis3dh_reg.int1_cfg);

  uint8_t val = md_threshold >= 8000 ? md_threshold / 125 :
	md_threshold >= 4000 ? md_threshold / 63 :
    md_threshold >= 2000 ? md_threshold / 31 :
    md_threshold / 16;

  if (val > 127) {
	  val = 127;
  }

  lis3dh_int1_gen_threshold_set(&lis3dh_ctx, val);
  lis3dh_int1_pin_notification_mode_set(&lis3dh_ctx, LIS3DH_INT1_LATCHED);

  return ;
}

// check to see if source is active
// (both INT1 and INT2 come in on the same IRQ vector)
// edge-triggered, NVIC has been cleared but LIS3DH needs attention
// HAL_GPIO_ReadPin(LIS3D_INT1_GPIO_Port, LIS3D_INT1_Pin);
// XXX: osThreadFlagsSet(Thd_LoraSendProcessId, 1);

uint8_t i1history[16];
uint8_t i1ndx;

static void
lis3dh_InterruptHandler()
{
  lis3dh_reg_t reg;

  // We may have been in STOP mode, need to re-init
  // ** INTERRUPT_CONTEXT **

  lis3dh_int1_gen_source_get(&lis3dh_ctx, &reg.int1_src);

  i1history[i1ndx++] = reg.byte;
  if (i1ndx >= 16) {
	  i1ndx = 0;
  }

}



/*
 * LIS3DH platform interface via I2C
 */

/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t
lis3dh_platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len)
{
  /* Write multiple command */
  reg |= 0x80;
  HAL_I2C_Mem_Write(handle, LIS3DH_I2C_ADD_H, reg,
                    I2C_MEMADD_SIZE_8BIT, (uint8_t*) bufp, len, 1000);

  return (0);
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t
lis3dh_platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
  /* Read multiple command */
  reg |= 0x80;
  HAL_I2C_Mem_Read(handle, LIS3DH_I2C_ADD_H, reg,
                   I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
  return (0);
}


/*
 * @brief  platform specific delay (platform dependent)
 *
 * @param  ms        delay in ms
 *
 */
static void
lis3dh_platform_delay(uint32_t ms)
{
  osDelay(ms);
}


/* USER CODE END PrFD */
