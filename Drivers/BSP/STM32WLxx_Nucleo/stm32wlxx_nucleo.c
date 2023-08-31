/**
  ******************************************************************************
  * @file    stm32wlxx_nucleo.c
  * @author  MCD Application Team
  * @brief   This file provides set of firmware functions to manage:
  *          - LEDs and push-buttons available on STM32WLXX-Nucleo
  *            Kit from STMicroelectronics
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020(-2021) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32wlxx_nucleo.h"

#include "adc.h"

/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup STM32WLXX_NUCLEO
  * @{
  */

/** @addtogroup STM32WLXX_NUCLEO_LOW_LEVEL
  * @brief This file provides set of firmware functions to manage LEDs and push-buttons
  *        on STM32WLXX-Nucleo Kit from STMicroelectronics.
  * @{
  */


/** @defgroup STM32WLXX_NUCLEO_LOW_LEVEL_Exported_Variables LOW LEVEL Exported Variables
  * @{
  */
EXTI_HandleTypeDef hpb_exti[BUTTONn];
#if (USE_BSP_COM_FEATURE > 0)
UART_HandleTypeDef hcom_uart[COMn];
#endif /* (USE_BSP_COM_FEATURE > 0) */
/**
  * @}
  */

/** @defgroup STM32WLXX_NUCLEO_LOW_LEVEL_Private_Defines LOW LEVEL Private Defines
  * @{
  */
/**
  * @}
  */

/** @defgroup STM32WLXX_NUCLEO_LOW_LEVEL_Private_TypesDefinitions LOW LEVEL Private Typedef
  * @{
  */
typedef void (* BSP_EXTI_LineCallback) (void);
/**
  * @}
  */

/** @defgroup STM32WLXX_NUCLEO_LOW_LEVEL_Private_Variables LOW LEVEL Private Variables
  * @{
  */
static GPIO_TypeDef* BUTTON_PORT[BUTTONn] = {BUTTON_SW1_GPIO_PORT, BUTTON_SW2_GPIO_PORT, BUTTON_SW3_GPIO_PORT};

static const uint16_t BUTTON_PIN[BUTTONn] = {BUTTON_SW1_PIN, BUTTON_SW2_PIN, BUTTON_SW3_PIN};

static const IRQn_Type BUTTON_IRQn[BUTTONn] = {BUTTON_SW1_EXTI_IRQn, BUTTON_SW2_EXTI_IRQn, BUTTON_SW3_EXTI_IRQn};

#if (USE_BSP_COM_FEATURE > 0)
static USART_TypeDef*  COM_USART[COMn] = {COM1_UART};

#if (USE_COM_LOG > 0)
static COM_TypeDef COM_ActiveLogPort = COM1;
#endif

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
static uint32_t IsComMspCbValid[COMn] = {0};
#endif
#endif /* (USE_BSP_COM_FEATURE > 0) */
/**
  * @}
  */

/** @defgroup STM32WLXX_NUCLEO_LOW_LEVEL_Private_Functions LOW LEVEL Private Functions
  * @{
  */
static void BUTTON_SW1_EXTI_Callback(void);

#if (USE_BSP_COM_FEATURE > 0)
static void COM1_MspInit(UART_HandleTypeDef *huart);
static void COM1_MspDeInit(UART_HandleTypeDef *huart);
#endif /* (USE_BSP_COM_FEATURE > 0) */
/**
  * @}
  */


/** @addtogroup STM32WLXX_NUCLEO_LOW_LEVEL_Exported_Functions
  * @{
  */ 

/**
  * @brief  This method returns the STM32WLXX NUCLEO BSP Driver revision
  * @retval version: 0xXYZR (8bits for each decimal, R for RC)
  */
uint32_t BSP_GetVersion(void)
{
  return (int32_t)__STM32WLXX_NUCLEO_BSP_VERSION;
}

/**
  * @}
  */ 

/** @addtogroup STM32WLXX_NUCLEO_LOW_LEVEL_BUTTON_Functions
  * @{
  */ 

/**
  * @brief  Configures Button GPIO and EXTI Line.
  * @param  Button: Specifies the Button to be configured.
  *         This parameter can be one of following parameters:
  *           @arg BUTTON_SW1
  *           @arg BUTTON_SW2
  *           @arg BUTTON_SW3
  * @param  ButtonMode: Specifies Button mode.
  *   This parameter can be one of following parameters:   
  *     @arg BUTTON_MODE_GPIO: Button will be used as simple IO
  *     @arg BUTTON_MODE_EXTI: Button will be connected to EXTI line with interrupt
  *                            generation capability  
  * @retval BSP status
  */
int32_t BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode)
{
  GPIO_InitTypeDef gpio_init_structure = {0};
  static BSP_EXTI_LineCallback button_callback[BUTTONn] = {BUTTON_SW1_EXTI_Callback, NULL, NULL};
  static uint32_t button_interrupt_priority[BUTTONn] = {BSP_BUTTON_USER_IT_PRIORITY, BSP_BUTTON_USER_IT_PRIORITY, BSP_BUTTON_USER_IT_PRIORITY};
  static const uint32_t button_exti_line[BUTTONn] = {BUTTON_SW1_EXTI_LINE, BUTTON_SW2_EXTI_LINE, BUTTON_SW3_EXTI_LINE};

  if (button_callback[Button] == NULL) {
	  return BSP_ERROR_NONE;
  }

  /* Enable the BUTTON Clock */
  BUTTONx_GPIO_CLK_ENABLE(Button);
  
  gpio_init_structure.Pin = BUTTON_PIN[Button];
  gpio_init_structure.Pull = GPIO_PULLUP;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;

  if(ButtonMode == BUTTON_MODE_GPIO)
  {
    /* Configure Button pin as input */
    gpio_init_structure.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(BUTTON_PORT[Button], &gpio_init_structure);
  }
  else /* (ButtonMode == BUTTON_MODE_EXTI) */
  {
    /* Configure Button pin as input with External interrupt */
    gpio_init_structure.Mode = GPIO_MODE_IT_FALLING;

    HAL_GPIO_Init(BUTTON_PORT[Button], &gpio_init_structure);

    (void)HAL_EXTI_GetHandle(&hpb_exti[Button], button_exti_line[Button]);
    (void)HAL_EXTI_RegisterCallback(&hpb_exti[Button],  HAL_EXTI_COMMON_CB_ID, button_callback[Button]);

    /* Enable and set Button EXTI Interrupt to the lowest priority */
    HAL_NVIC_SetPriority((BUTTON_IRQn[Button]), button_interrupt_priority[Button], 0x00);
    HAL_NVIC_EnableIRQ((BUTTON_IRQn[Button]));
  }

  return BSP_ERROR_NONE;
}

/**
  * @brief  Push Button DeInit.
  * @param  Button: Button to be configured
  *         This parameter can be one of following parameters:
  *           @arg BUTTON_SW1
  *           @arg BUTTON_SW2
  *           @arg BUTTON_SW3
  * @note PB DeInit does not disable the GPIO clock
  * @retval BSP status
  */
int32_t BSP_PB_DeInit(Button_TypeDef Button)
{
  HAL_NVIC_DisableIRQ((BUTTON_IRQn[Button]));
  HAL_GPIO_DeInit(BUTTON_PORT[Button], BUTTON_PIN[Button]);

  return BSP_ERROR_NONE;
}

/**
  * @brief  Returns the selected Button state.
  * @param  Button: Specifies the Button to be checked.
  *         This parameter can be one of following parameters:
  *           @arg BUTTON_SW1
  *           @arg BUTTON_SW2
  *           @arg BUTTON_SW3
  * @retval The Button GPIO pin value.
  */
int32_t BSP_PB_GetState(Button_TypeDef Button)
{
  return (int32_t)HAL_GPIO_ReadPin(BUTTON_PORT[Button], BUTTON_PIN[Button]);
}

/**
  * @brief  This function handles Push-Button interrupt requests.
  * @param  Button Specifies the pin connected EXTI line
  * @retval None
  */
void BSP_PB_IRQHandler(Button_TypeDef Button)
{
  HAL_EXTI_IRQHandler(&hpb_exti[Button]);
}

/**
  * @brief  BSP Push Button callback
  * @param  Button: Specifies the Button to be checked.
  *         This parameter can be one of following parameters:
  *           @arg BUTTON_SW1
  *           @arg BUTTON_SW2
  *           @arg BUTTON_SW3
  * @retval None.
  */
__weak void BSP_PB_Callback(Button_TypeDef Button)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Button);

  /* This function should be implemented by the user application.
     It is called into this driver when an event on Button is triggered. */
}


/*
 *
 */
static uint32_t
BSP_ADC_ReadChannels(uint32_t channel)
{
  uint32_t ADCxConvertedValues = 0;
  ADC_ChannelConfTypeDef sConfig = {0};

  MX_ADC_Init();

  /* Start Calibration */
  if (HAL_ADCEx_Calibration_Start(&hadc) != HAL_OK)
  {
    Error_Handler();
  }

  /* Configure Regular Channel */
  sConfig.Channel = channel;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_ADC_Start(&hadc) != HAL_OK)
  {
    /* Start Error */
    Error_Handler();
  }
  /** Wait for end of conversion */
  HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);

  /** Wait for end of conversion */
  HAL_ADC_Stop(&hadc) ;   /* it calls also ADC_Disable() */

  ADCxConvertedValues = HAL_ADC_GetValue(&hadc);

  HAL_ADC_DeInit(&hadc);

  return ADCxConvertedValues;
}

uint16_t
BSP_RAK5005_GetBatteryLevel(void)
{
  uint16_t batteryLevelmV = 0;
  uint32_t measuredLevel = 0;

  measuredLevel = BSP_ADC_ReadChannels(ADC_CHANNEL_2);

  if (measuredLevel == 0) {
    batteryLevelmV = 0;
  } else {
    batteryLevelmV = (measuredLevel * 25000U) / 15000U;
  }

  return batteryLevelmV;
}

/**
  * @}
  */ 

#if (USE_BSP_COM_FEATURE > 0)
/** @addtogroup STM32WLXX_NUCLEO_LOW_LEVEL_COM_Functions
  * @{
  */
/**
  * @brief  Configures COM port.
  * @param  COM COM port to be configured.
  *         This parameter can be COM1
  * @param  COM_Init Pointer to a UART_HandleTypeDef structure that contains the
  *                  configuration information for the specified USART peripheral.
  * @retval BSP error code
  */
int32_t BSP_COM_Init(COM_TypeDef COM, COM_InitTypeDef *COM_Init)
{
  int32_t ret = BSP_ERROR_NONE;

  if(COM > COMn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_HAL_UART_REGISTER_CALLBACKS == 0)
    /* Init the UART Msp */
    COM1_MspInit(&hcom_uart[COM]);
#else
    if(IsComMspCbValid == 0U)
    {
      if(BSP_COM_RegisterDefaultMspCallbacks(COM) != BSP_ERROR_NONE)
      {
        return BSP_ERROR_MSP_FAILURE;
      }
    }
#endif

    if(MX_LPUART1_Init(&hcom_uart[COM], COM_Init) != HAL_OK)
    {
      return BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  DeInit COM port.
  * @param  COM COM port to be configured.
  *          This parameter can be COM1
  * @retval BSP status
  */
int32_t BSP_COM_DeInit(COM_TypeDef COM)
{
  int32_t ret = BSP_ERROR_NONE;

  if(COM >= COMn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* USART configuration */
    hcom_uart[COM].Instance = COM_USART[COM];

#if (USE_HAL_UART_REGISTER_CALLBACKS == 0)
    COM1_MspDeInit(&hcom_uart[COM]);
#endif /* (USE_HAL_UART_REGISTER_CALLBACKS == 0) */

    if(HAL_UART_DeInit(&hcom_uart[COM]) != HAL_OK)
    {
      return BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  Configures COM port.
  * @param  huart USART handle
  * @param  COM_Init Pointer to a UART_HandleTypeDef structure that contains the
  *                  configuration information for the specified USART peripheral.
  * @retval HAL error code
  */
__weak HAL_StatusTypeDef MX_LPUART1_Init(UART_HandleTypeDef *huart, MX_UART_InitTypeDef *COM_Init)
{
  /* USART configuration */
 huart->Instance          = COM_USART[COM1];
 huart->Init.BaudRate     = COM_Init->BaudRate;
 huart->Init.Mode         = UART_MODE_TX_RX;
 huart->Init.Parity       = (uint32_t)COM_Init->Parity;
 huart->Init.WordLength   = COM_Init->WordLength;
 huart->Init.StopBits     = (uint32_t)COM_Init->StopBits;
 huart->Init.HwFlowCtl    = (uint32_t)COM_Init->HwFlowCtl;
 huart->Init.OverSampling = UART_OVERSAMPLING_8;

 return HAL_UART_Init(huart);
}

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
/**
  * @brief Register Default COM Msp Callbacks
  * @param  COM COM port to be configured.
  *          This parameter can be COM1
  * @retval BSP status
  */
int32_t BSP_COM_RegisterDefaultMspCallbacks(COM_TypeDef COM)
{
  int32_t ret = BSP_ERROR_NONE;

  if(COM >= COMn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    __HAL_UART_RESET_HANDLE_STATE(&hcom_uart[COM]);

    /* Register default MspInit/MspDeInit Callback */
    if(HAL_UART_RegisterCallback(&hcom_uart[COM], HAL_UART_MSPINIT_CB_ID, COM1_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_UART_RegisterCallback(&hcom_uart[COM], HAL_UART_MSPDEINIT_CB_ID, COM1_MspDeInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      IsComMspCbValid[COM] = 1U;
    }
  }

  /* BSP status */
  return ret;
}

/**
  * @brief Register COM Msp Callback registering
  * @param  COM COM port to be configured.
  *          This parameter can be COM1
  * @param Callbacks     pointer to COM1 MspInit/MspDeInit callback functions
  * @retval BSP status
  */
int32_t BSP_COM_RegisterMspCallbacks(COM_TypeDef COM , BSP_COM_Cb_t *Callback)
{
  int32_t ret = BSP_ERROR_NONE;

  if(COM >= COMn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    __HAL_UART_RESET_HANDLE_STATE(&hcom_uart[COM]);

    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_UART_RegisterCallback(&hcom_uart[COM], HAL_UART_MSPINIT_CB_ID, Callback->pMspInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_UART_RegisterCallback(&hcom_uart[COM], HAL_UART_MSPDEINIT_CB_ID, Callback->pMspDeInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      IsComMspCbValid[COM] = 1U;
    }
  }
  /* BSP status */
  return ret;
}
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */

#if (USE_COM_LOG > 0)
/**
  * @brief  Select the active COM port.
  * @param  COM COM port to be activated.
  *          This parameter can be COM1
  * @retval BSP status
  */
int32_t BSP_COM_SelectLogPort(COM_TypeDef COM)
{
  if(COM_ActiveLogPort != COM)
  {
    COM_ActiveLogPort = COM;
  }
  return BSP_ERROR_NONE;
}

/**
  * @brief  Redirect console output to COM
  */
#ifdef __GNUC__
int __io_putchar (int ch)
#else
int fputc (int ch, FILE *f)
#endif /* __GNUC__ */
{
  (void) HAL_UART_Transmit(&hcom_uart [COM_ActiveLogPort], (uint8_t *) &ch, 1, COM_POLL_TIMEOUT);
  return ch;
}
#endif /* USE_COM_LOG */
/**
  * @}
  */ 
#endif /* (USE_BSP_COM_FEATURE > 0) */

/*
 *
 */
int32_t
BSP_RAK5005_Init(void)
{
	GPIO_InitTypeDef gpio_init_structure = {0};

	/* Enable the GPIO Clocks */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/* Configure the GPIO pins */
	gpio_init_structure.Pin = GPIO_PIN_8;
	gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_init_structure.Pull = GPIO_NOPULL;
	gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;

	HAL_GPIO_Init(GPIOA, &gpio_init_structure);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);

	gpio_init_structure.Pin = GPIO_PIN_11;

	HAL_GPIO_Init(GPIOA, &gpio_init_structure);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);

	return BSP_ERROR_NONE;
}

int32_t			BSP_RAK5005_DeInit(void);

/**
  * @}
  */

/** @addtogroup STM32WLXX_NUCLEO_LOW_LEVEL_Private_Functions
  * @{
  */

/**
  * @brief  Button SW1 EXTI line detection callback.
  * @retval None
  */
static void BUTTON_SW1_EXTI_Callback(void)
{
  BSP_PB_Callback(BUTTON_SW1);
}


#if (USE_BSP_COM_FEATURE > 0)
/**
  * @brief  Initializes COM1 MSP.
  * @param  huart UART handle
  * @retval BSP status
  */
static void COM1_MspInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef gpio_init_structure;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);

  /* Enable GPIO clock */
  COM1_TX_GPIO_CLK_ENABLE();
  COM1_RX_GPIO_CLK_ENABLE();

  /* Enable USART clock */
  COM1_CLK_ENABLE();

  /* Configure USART Tx as alternate function */
  gpio_init_structure.Pin       = COM1_TX_PIN;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_HIGH;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Alternate = COM1_TX_AF;
  HAL_GPIO_Init(COM1_TX_GPIO_PORT, &gpio_init_structure);

  /* Configure USART Rx as alternate function */
  gpio_init_structure.Pin = COM1_RX_PIN;
  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Alternate = COM1_RX_AF;
  HAL_GPIO_Init(COM1_RX_GPIO_PORT, &gpio_init_structure);
}

/**
  * @brief  DeInitialize COM1 MSP part
  * @param  huart UART handle
  * @retval BSP status
  */
static void COM1_MspDeInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef gpio_init_structure;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);

  /* COM GPIO pin configuration */
  gpio_init_structure.Pin  = COM1_TX_PIN;
  HAL_GPIO_DeInit(COM1_TX_GPIO_PORT, gpio_init_structure.Pin);

  gpio_init_structure.Pin  = COM1_RX_PIN;
  HAL_GPIO_DeInit(COM1_RX_GPIO_PORT, gpio_init_structure.Pin);

  /* Disable USART clock */
  COM1_CLK_DISABLE();
}
#endif /* (USE_BSP_COM_FEATURE > 0) */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */    

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
