/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    lora_app.c
  * @author  MCD Application Team
  * @brief   Application of the LRWAN Middleware
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "sys_app.h"
#include "lora_app.h"
#include "stm32_seq.h"
#include "stm32_timer.h"
#include "utilities_def.h"
#include "lora_app_version.h"
#include "lorawan_version.h"
#include "subghz_phy_version.h"
#include "lora_info.h"
#include "LmHandler.h"
#include "stm32_lpm.h"
#include "adc_if.h"
#include "CayenneLpp.h"
#include "sys_sensors.h"
#include "flash_if.h"

/* USER CODE BEGIN Includes */

#include "sys_debug.h"
#include "LoRaMac.h"

/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/**
  * @brief LoRa State Machine states
  */
typedef enum TxEventType_e
{
  /**
    * @brief Appdata Transmission issue based on timer every TxDutyCycleTime
    */
  TX_ON_TIMER,
  /**
    * @brief Appdata Transmission external event plugged on OnSendEvent( )
    */
  TX_ON_EVENT
  /* USER CODE BEGIN TxEventType_t */

  /* USER CODE END TxEventType_t */
} TxEventType_t;

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/**
  * LEDs period value of the timer in ms
  */
#define LED_PERIOD_TIME 500

/**
  * Join switch period value of the timer in ms
  */
#define JOIN_TIME 2000

/*---------------------------------------------------------------------------*/
/*                             LoRaWAN NVM configuration                     */
/*---------------------------------------------------------------------------*/
/**
  * @brief LoRaWAN NVM Flash address
  * @note last 2 sector of a 128kBytes device
  */
#define LORAWAN_NVM_BASE_ADDRESS                    ((uint32_t)0x0803F000UL)

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  LoRa End Node send request
  */
static void SendTxData(void);

/**
  * @brief  TX timer callback function
  * @param  context ptr of timer context
  */
static void OnTxTimerEvent(void *context);

/**
  * @brief  join event callback function
  * @param  joinParams status of join
  */
static void OnJoinRequest(LmHandlerJoinParams_t *joinParams);

/**
  * @brief callback when LoRaWAN application has sent a frame
  * @brief  tx event callback function
  * @param  params status of last Tx
  */
static void OnTxData(LmHandlerTxParams_t *params);

/**
  * @brief callback when LoRaWAN application has received a frame
  * @param appData data received in the last Rx
  * @param params status of last Rx
  */
static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params);

/**
  * @brief callback when LoRaWAN Beacon status is updated
  * @param params status of Last Beacon
  */
static void OnBeaconStatusChange(LmHandlerBeaconParams_t *params);

/**
  * @brief callback when LoRaWAN application Class is changed
  * @param deviceClass new class
  */
static void OnClassChange(DeviceClass_t deviceClass);

/**
  * @brief  LoRa store context in Non Volatile Memory
  */
static void StoreContext(void);

/**
  * @brief  stop current LoRa execution to switch into non default Activation mode
  */
static void StopJoin(void);

/**
  * @brief  Join switch timer callback function
  * @param  context ptr of Join switch context
  */
static void OnStopJoinTimerEvent(void *context);

/**
  * @brief  Notifies the upper layer that the NVM context has changed
  * @param  state Indicates if we are storing (true) or restoring (false) the NVM context
  */
static void OnNvmDataChange(LmHandlerNvmContextStates_t state);

/**
  * @brief  Store the NVM Data context to the Flash
  * @param  nvm ptr on nvm structure
  * @param  nvm_size number of data bytes which were stored
  */
static void OnStoreContextRequest(void *nvm, uint32_t nvm_size);

/**
  * @brief  Restore the NVM Data context from the Flash
  * @param  nvm ptr on nvm structure
  * @param  nvm_size number of data bytes which were restored
  */
static void OnRestoreContextRequest(void *nvm, uint32_t nvm_size);

/**
  * Will be called each time a Radio IRQ is handled by the MAC layer
  *
  */
static void OnMacProcessNotify(void);

/**
  * @brief Change the periodicity of the uplink frames
  * @param periodicity uplink frames period in ms
  * @note Compliance test protocol callbacks
  */
static void OnTxPeriodicityChanged(uint32_t periodicity);

/**
  * @brief Change the confirmation control of the uplink frames
  * @param isTxConfirmed Indicates if the uplink requires an acknowledgement
  * @note Compliance test protocol callbacks
  */
static void OnTxFrameCtrlChanged(LmHandlerMsgTypes_t isTxConfirmed);

/**
  * @brief Change the periodicity of the ping slot frames
  * @param pingSlotPeriodicity ping slot frames period in ms
  * @note Compliance test protocol callbacks
  */
static void OnPingSlotPeriodicityChanged(uint8_t pingSlotPeriodicity);

/**
  * @brief Will be called to reset the system
  * @note Compliance test protocol callbacks
  */
static void OnSystemReset(void);

/* USER CODE BEGIN PFP */

static void OnSwitchTimerEvent(void *context);
static void TurnSwitchOff(void);
static void debounceInput(void);

/* USER CODE END PFP */

/* Private variables ---------------------------------------------------------*/
/**
  * @brief LoRaWAN default activation type
  */
static ActivationType_t ActivationType = LORAWAN_DEFAULT_ACTIVATION_TYPE;

/**
  * @brief LoRaWAN force rejoin even if the NVM context is restored
  */
static bool ForceRejoin = LORAWAN_FORCE_REJOIN_AT_BOOT;

/**
  * @brief LoRaWAN handler Callbacks
  */
static LmHandlerCallbacks_t LmHandlerCallbacks =
{
  .GetBatteryLevel =              GetBatteryLevel,
  .GetTemperature =               GetTemperatureLevel,
  .GetUniqueId =                  GetUniqueId,
  .GetDevAddr =                   GetDevAddr,
  .OnRestoreContextRequest =      OnRestoreContextRequest,
  .OnStoreContextRequest =        OnStoreContextRequest,
  .OnMacProcess =                 OnMacProcessNotify,
  .OnNvmDataChange =              OnNvmDataChange,
  .OnJoinRequest =                OnJoinRequest,
  .OnTxData =                     OnTxData,
  .OnRxData =                     OnRxData,
  .OnBeaconStatusChange =         OnBeaconStatusChange,
  .OnClassChange =                OnClassChange,
  .OnTxPeriodicityChanged =       OnTxPeriodicityChanged,
  .OnTxFrameCtrlChanged =         OnTxFrameCtrlChanged,
  .OnPingSlotPeriodicityChanged = OnPingSlotPeriodicityChanged,
  .OnSystemReset =                OnSystemReset,
};

/**
  * @brief LoRaWAN handler parameters
  */
static LmHandlerParams_t LmHandlerParams =
{
  .ActiveRegion =             ACTIVE_REGION,
  .DefaultClass =             LORAWAN_DEFAULT_CLASS,
  .AdrEnable =                LORAWAN_ADR_STATE,
  .IsTxConfirmed =            LORAWAN_DEFAULT_CONFIRMED_MSG_STATE,
  .TxDatarate =               LORAWAN_DEFAULT_DATA_RATE,
  .PingSlotPeriodicity =      LORAWAN_DEFAULT_PING_SLOT_PERIODICITY,
  .RxBCTimeout =              LORAWAN_DEFAULT_CLASS_B_C_RESP_TIMEOUT
};

/**
  * @brief Type of Event to generate application Tx
  */
static TxEventType_t EventType = TX_ON_TIMER;

/**
  * @brief Timer to handle the application Tx
  */
static UTIL_TIMER_Object_t TxTimer;

/**
  * @brief Tx Timer period
  */
static UTIL_TIMER_Time_t TxPeriodicity = APP_TX_DUTYCYCLE;

/**
  * @brief Join Timer period
  */
static UTIL_TIMER_Object_t StopJoinTimer;

/* USER CODE BEGIN PV */

static uint8_t switchState = false;
static uint8_t previousSwitchState = false;

static uint8_t inputState = false;
static uint8_t previousInputState = false;

static UplinkQueue_t uplinkQueue[UPLINK_QUEUE_NUM];
static uint8_t ulQueueInsert, ulQueueRemove;

static uint8_t queueEnabled = false;

static UTIL_TIMER_Object_t SwitchTimer;



/* USER CODE END PV */

/* Exported functions ---------------------------------------------------------*/
/* USER CODE BEGIN EF */

/* USER CODE END EF */

void LoRaWAN_Init(void)
{
  /* USER CODE BEGIN LoRaWAN_Init_LV */
  uint32_t feature_version = 0UL;
  /* USER CODE END LoRaWAN_Init_LV */

  /* USER CODE BEGIN LoRaWAN_Init_1 */
	BSP_RAK5005_Init();
	BSP_LED_Init(LED_BLUE);
	BSP_LED_Init(LED_GREEN);

	/* in case we have SWD enabled */
	HAL_DBGMCU_EnableDBGStandbyMode();
	HAL_DBGMCU_EnableDBGSleepMode();
	HAL_DBGMCU_EnableDBGStopMode();


	// BSP_PB_Init(BUTTON_SW1, BUTTON_MODE_EXTI);

	{
		// XXX: move this to BSP
		GPIO_InitTypeDef GPIO_InitStruct = { 0 };

		__HAL_RCC_GPIOA_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		inputState = !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11);
		previousInputState = inputState;

		HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	}

  /* Get LoRa APP version*/
  APP_LOG(TS_OFF, VLEVEL_M, "APPLICATION_VERSION: V%X.%X.%X\r\n",
          (uint8_t)(APP_VERSION_MAIN),
          (uint8_t)(APP_VERSION_SUB1),
          (uint8_t)(APP_VERSION_SUB2));

  /* Get MW LoRaWAN info */
  APP_LOG(TS_OFF, VLEVEL_M, "MW_LORAWAN_VERSION: V%X.%X.%X\r\n",
          (uint8_t)(LORAWAN_VERSION_MAIN),
          (uint8_t)(LORAWAN_VERSION_SUB1),
          (uint8_t)(LORAWAN_VERSION_SUB2));

  /* Get MW SubGhz_Phy info */
  APP_LOG(TS_OFF, VLEVEL_M, "MW_RADIO_VERSION:   V%X.%X.%X\r\n",
          (uint8_t)(SUBGHZ_PHY_VERSION_MAIN),
          (uint8_t)(SUBGHZ_PHY_VERSION_SUB1),
          (uint8_t)(SUBGHZ_PHY_VERSION_SUB2));

  /* Get LoRaWAN Link Layer info */
  LmHandlerGetVersion(LORAMAC_HANDLER_L2_VERSION, &feature_version);
  APP_LOG(TS_OFF, VLEVEL_M, "L2_SPEC_VERSION:     V%X.%X.%X\r\n",
          (uint8_t)(feature_version >> 24),
          (uint8_t)(feature_version >> 16),
          (uint8_t)(feature_version >> 8));

  /* Get LoRaWAN Regional Parameters info */
  LmHandlerGetVersion(LORAMAC_HANDLER_REGION_VERSION, &feature_version);
  APP_LOG(TS_OFF, VLEVEL_M, "RP_SPEC_VERSION:     V%X-%X.%X.%X\r\n",
          (uint8_t)(feature_version >> 24),
          (uint8_t)(feature_version >> 16),
          (uint8_t)(feature_version >> 8),
          (uint8_t)(feature_version));

  /* USER CODE END LoRaWAN_Init_1 */

  UTIL_TIMER_Create(&StopJoinTimer, JOIN_TIME, UTIL_TIMER_ONESHOT, OnStopJoinTimerEvent, NULL);

  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LmHandlerProcess), UTIL_SEQ_RFU, LmHandlerProcess);

  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), UTIL_SEQ_RFU, SendTxData);
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LoRaStoreContextEvent), UTIL_SEQ_RFU, StoreContext);
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LoRaStopJoinEvent), UTIL_SEQ_RFU, StopJoin);

  /* Init Info table used by LmHandler*/
  LoraInfo_Init();

  /* Init the Lora Stack*/
  LmHandlerInit(&LmHandlerCallbacks, APP_VERSION);

  LmHandlerConfigure(&LmHandlerParams);

  /* USER CODE BEGIN LoRaWAN_Init_2 */
	UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_SwitchTimerEvent),
			UTIL_SEQ_RFU, TurnSwitchOff);
	UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_DebounceInput),
			UTIL_SEQ_RFU, debounceInput);
  /* USER CODE END LoRaWAN_Init_2 */

  LmHandlerJoin(ActivationType, ForceRejoin);

  if (EventType == TX_ON_TIMER)
  {
    /* send every time timer elapses */
    UTIL_TIMER_Create(&TxTimer, TxPeriodicity, UTIL_TIMER_ONESHOT, OnTxTimerEvent, NULL);
    UTIL_TIMER_Start(&TxTimer);
  }
  else
  {
    /* USER CODE BEGIN LoRaWAN_Init_3 */

    /* USER CODE END LoRaWAN_Init_3 */
  }

  /* USER CODE BEGIN LoRaWAN_Init_Last */
	UTIL_TIMER_Create(&SwitchTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT,
			OnSwitchTimerEvent, NULL);
	UTIL_TIMER_SetPeriod(&SwitchTimer, APP_TX_DUTYCYCLE);
	UTIL_TIMER_Start(&SwitchTimer);

	/* wait until joined to start */
    UTIL_TIMER_Stop(&TxTimer);
  /* USER CODE END LoRaWAN_Init_Last */
}

/* USER CODE BEGIN PB_Callbacks */

/* USER CODE END PB_Callbacks */

/* Private functions ---------------------------------------------------------*/
/* USER CODE BEGIN PrFD */

/*
 * UL Queue management
 */

// add element to queue

static void
startUplink(void)
{
	UplinkQueue_t *q = uplinkQueue + ulQueueRemove;
	UTIL_TIMER_Time_t nextTxIn = 0;

	/* make sure we're not empty */
	if (ulQueueInsert == ulQueueRemove) {
		return;
	}

	if (LoRaMacIsBusy()) {
		return;
	}

	q->upLinkPayload[0] = q->uplinkType;
	q->appData.Buffer = q->upLinkPayload;

	switch (q->uplinkType) {
	case UL_ID_ID:
		q->upLinkPayload[1] = 0x00;
		q->upLinkPayload[2] = 0x01;
		q->upLinkPayload[3] = 0x01;
		q->upLinkPayload[4] = 0x00;
		q->appData.Port = UL_PORT_ID;
		q->appData.BufferSize = 5;
		break;

	case UL_ID_STATUS:
		q->upLinkPayload[1] = (switchState ? 0x01 : 0x00) |
		  (inputState ? 0x02 : 0x00) |
		  (previousSwitchState != switchState ? 0x10 : 0x00) |
		  (previousInputState != inputState ? 0x20 : 0x00);
		previousSwitchState = switchState;
		previousInputState = inputState;
		q->appData.Port = UL_PORT_STATUS;
		q->appData.BufferSize = 2;
		break;

	case UL_ID_ERROR:
		q->upLinkPayload[1] = 0x00;
		q->appData.Port = UL_PORT_ERROR;
		q->appData.BufferSize = 2;
		break;

	default:
		q->appData.BufferSize = 0;
		break;
	}

	/* delete invalid entry */
	if (q->appData.BufferSize == 0) {
		ulQueueRemove = UplinkQueueNext(ulQueueRemove);
		return;
	}

	if (LORAMAC_HANDLER_SUCCESS == LmHandlerSend(&q->appData,
	  q->upLinkConfirmedCount ? LORAMAC_HANDLER_CONFIRMED_MSG : LORAMAC_HANDLER_UNCONFIRMED_MSG,
	   false)) {
		APP_LOG(TS_ON, VLEVEL_L, "SEND REQUEST\r\n");
	} else if (nextTxIn > 0) {
		APP_LOG(TS_ON, VLEVEL_L, "Next Tx in  : ~%d second(s)\r\n",
				(nextTxIn / 1000));
	}

}

static void
enqueueUplink(uint8_t ulType, uint8_t ulConfirmed)
{
	uint8_t empty;

	if (!queueEnabled) {
		return;
	}

	empty = ulQueueInsert == ulQueueRemove;
	if (UplinkQueueNext(ulQueueInsert) == ulQueueRemove) {
		// XXX: Queue is full!
		APP_LOG(TS_OFF, VLEVEL_M,
		  "\r\n###### ========== UPLINK QUEUE OVERFLOW ==========\r\n");
		return;
	}

	uplinkQueue[ulQueueInsert].uplinkType = ulType;
	uplinkQueue[ulQueueInsert].upLinkConfirmedCount = ulConfirmed ? 4 : 0;
	ulQueueInsert = UplinkQueueNext(ulQueueInsert);;

	/* implication that LoRaMAC is not busy when queue is empty */
	if (empty) {
		startUplink();
	}
}


// TxData done processing
static void
processUplinks(LmHandlerTxParams_t *params)
{

	if (ulQueueInsert == ulQueueRemove) {
		// XXX: Queue is empty!
		APP_LOG(TS_OFF, VLEVEL_M,
		  "\r\n###### ========== UPLINK QUEUE UNDERFLOW ==========\r\n");
		return;
	}

	/* update switch state after possible confirmed UL */
	previousSwitchState = switchState;

	ulQueueRemove = UplinkQueueNext(ulQueueRemove);
	startUplink();
}

static void
OnSwitchTimerEvent(void *context)
{

  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SwitchTimerEvent), CFG_SEQ_Prio_0);
}

static void
TurnSwitchOff(void)
{

	APP_LOG(TS_OFF, VLEVEL_M, "Timer: Switch OFF\r\n");
	BSP_RAK5005_Relay_Off();
	switchState = false;
	enqueueUplink(UL_ID_STATUS, true);
}

static void
debounceInput(void)
{
	uint16_t count;
	uint16_t thisTime, lastTime;

	// debounce input with timer
	count = 0;
	lastTime = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11);
	do {
		HAL_Delay(2);
		thisTime = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11);
		if (thisTime != lastTime) {
			count = 0;
			lastTime = thisTime;
		} else {
			count += 1;
		}
	} while (count < 25);

	inputState = !thisTime;

	APP_LOG(TS_ON, VLEVEL_L, "Pin Event: %u\r\n", inputState);

	// XXX: rate limit here?
	// need different filtering for flowmeter, GPIO

	// enqueue state change
	if (previousInputState != inputState) {
		enqueueUplink(UL_ID_STATUS, true);
	}

	// re-enable the IRQ
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void
HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	// kick the debounce task into action
	if (GPIO_Pin == GPIO_PIN_11) {
		// disable the IRQ
		HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
		UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_DebounceInput), CFG_SEQ_Prio_0);
	}
}

/* USER CODE END PrFD */

static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
  /* USER CODE BEGIN OnRxData_1 */

	if ((appData != NULL) || (params != NULL)) {
		static const char *slotStrings[] = { "1", "2", "C", "C Multicast",
				"B Ping-Slot", "B Multicast Ping-Slot" };

		APP_LOG(TS_OFF, VLEVEL_M,
				"\r\n###### ========== MCPS-Indication ==========\r\n");
		APP_LOG(TS_OFF, VLEVEL_H,
				"###### D/L FRAME:%04d | SLOT:%s | PORT:%d | DR:%d | RSSI:%d | SNR:%d | DemodMargin: %u | NbGWs: %u\r\n",
				params->DownlinkCounter, slotStrings[params->RxSlot],
				appData->Port, params->Datarate, params->Rssi, params->Snr,
				params->DemodMargin, params->NbGateways);

		switch (appData->Port) {
		case DL_PORT_SWITCH:
			if ((appData->Buffer[0] == DL_ID_SWITCH) &&
			  ( ((appData->Buffer[1] & 1) && (appData->BufferSize == 5)) ||
			    (((appData->Buffer[1] & 1) == 0) && (appData->BufferSize == 2))) ) {
				uint32_t seconds;

				seconds = (appData->Buffer[2] << 16) |
				  (appData->Buffer[3] << 8) | appData->Buffer[4];

				// set switch state
				if (appData->Buffer[1] & 1) {
					if (seconds < 30) {
						enqueueUplink(UL_ID_ERROR, false);
						break;
					}

					APP_LOG(TS_OFF, VLEVEL_M, "Switch ON %u\r\n", seconds);
					BSP_RAK5005_Relay_On();
					switchState = true;
					// start safety timer
					UTIL_TIMER_StartWithPeriod(&SwitchTimer, seconds * 1000UL);
				} else {
					APP_LOG(TS_OFF, VLEVEL_M, "Switch OFF\r\n");
					BSP_RAK5005_Relay_Off();
					switchState = false;
					UTIL_TIMER_Stop(&SwitchTimer);
				}

				// queue STATUS
				enqueueUplink(UL_ID_STATUS, false);

			} else  {
				// queue ERROR
				enqueueUplink(UL_ID_ERROR, false);
			}
			break;

		default:
			// XXX: UL an ERROR
			break;
		}
	}
  /* USER CODE END OnRxData_1 */
}

static void SendTxData(void)
{
  /* USER CODE BEGIN SendTxData_1 */

	APP_LOG(TS_OFF, VLEVEL_M, "SendTxData\r\n");

	LmHandlerLinkCheckReq();
	enqueueUplink(UL_ID_STATUS, true);

  /* USER CODE END SendTxData_1 */
}

static void OnTxTimerEvent(void *context)
{
  /* USER CODE BEGIN OnTxTimerEvent_1 */

  /* USER CODE END OnTxTimerEvent_1 */
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), CFG_SEQ_Prio_0);

  /*Wait for next tx slot*/
  UTIL_TIMER_Start(&TxTimer);
  /* USER CODE BEGIN OnTxTimerEvent_2 */

  /* USER CODE END OnTxTimerEvent_2 */
}

/* USER CODE BEGIN PrFD_LedEvents */

/* USER CODE END PrFD_LedEvents */

static void OnTxData(LmHandlerTxParams_t *params)
{
  /* USER CODE BEGIN OnTxData_1 */
	if ((params != NULL)) {
		/* Process Tx event only if its a mcps response to prevent some internal events (mlme) */
		if (params->IsMcpsConfirm != 0) {

			APP_LOG(TS_OFF, VLEVEL_M,
					"\r\n###### ========== MCPS-Confirm =============\r\n");
			APP_LOG(TS_OFF, VLEVEL_H,
					"###### U/L FRAME:%04d | PORT:%d | DR:%d | PWR:%d",
					params->UplinkCounter, params->AppData.Port,
					params->Datarate, params->TxPower);

			APP_LOG(TS_OFF, VLEVEL_H, " | MSG TYPE:");

			if (params->MsgType == LORAMAC_HANDLER_CONFIRMED_MSG) {
				APP_LOG(TS_OFF, VLEVEL_M, "CONFIRMED [%s]\r\n",
						(params->AckReceived != 0) ? "ACK" : "NACK");
				// XXX: if NACK, possibly re-join
			} else {
				APP_LOG(TS_OFF, VLEVEL_H, "UNCONFIRMED\r\n");
			}

			APP_LOG(TS_OFF, VLEVEL_M, "Buffer: %x\r\n", params->AppData.Buffer);

			// kick UL Queue
			processUplinks(params);
		}
	}
  /* USER CODE END OnTxData_1 */
}

static void OnJoinRequest(LmHandlerJoinParams_t *joinParams)
{
  /* USER CODE BEGIN OnJoinRequest_1 */

	if (joinParams != NULL) {
		if (joinParams->Status == LORAMAC_HANDLER_SUCCESS) {
			APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### = JOINED = ");
			if (joinParams->Mode == ACTIVATION_TYPE_ABP) {
				APP_LOG(TS_OFF, VLEVEL_M, "ABP ======================\r\n");
			} else {
				APP_LOG(TS_OFF, VLEVEL_M, "OTAA =====================\r\n");
			}

			/* start periodic updates */
			UTIL_TIMER_Start(&TxTimer);

			/* send ID report */
			LmHandlerLinkCheckReq();
			queueEnabled = true;
			enqueueUplink(UL_ID_ID, false);
			enqueueUplink(UL_ID_STATUS, false);
			UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaStoreContextEvent), CFG_SEQ_Prio_0);
		} else {
			APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### = JOIN FAILED\r\n");

			if (joinParams->Mode == ACTIVATION_TYPE_OTAA) {
				APP_LOG(TS_OFF, VLEVEL_M,
						"\r\n###### = RE-TRYING OTAA JOIN\r\n");
				/* re-try the OTAA join */
				LmHandlerJoin(ActivationType, LORAWAN_FORCE_REJOIN_AT_BOOT);
			}
		}
	}
  /* USER CODE END OnJoinRequest_1 */
}

static void OnBeaconStatusChange(LmHandlerBeaconParams_t *params)
{
  /* USER CODE BEGIN OnBeaconStatusChange_1 */
  /* USER CODE END OnBeaconStatusChange_1 */
}

static void OnClassChange(DeviceClass_t deviceClass)
{
  /* USER CODE BEGIN OnClassChange_1 */
  /* USER CODE END OnClassChange_1 */
}

static void OnMacProcessNotify(void)
{
  /* USER CODE BEGIN OnMacProcessNotify_1 */

  /* USER CODE END OnMacProcessNotify_1 */
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LmHandlerProcess), CFG_SEQ_Prio_0);

  /* USER CODE BEGIN OnMacProcessNotify_2 */

  /* USER CODE END OnMacProcessNotify_2 */
}

static void OnTxPeriodicityChanged(uint32_t periodicity)
{
  /* USER CODE BEGIN OnTxPeriodicityChanged_1 */

  /* USER CODE END OnTxPeriodicityChanged_1 */
  TxPeriodicity = periodicity;

  if (TxPeriodicity == 0)
  {
    /* Revert to application default periodicity */
    TxPeriodicity = APP_TX_DUTYCYCLE;
  }

  /* Update timer periodicity */
  UTIL_TIMER_Stop(&TxTimer);
  UTIL_TIMER_SetPeriod(&TxTimer, TxPeriodicity);
  UTIL_TIMER_Start(&TxTimer);
  /* USER CODE BEGIN OnTxPeriodicityChanged_2 */

  /* USER CODE END OnTxPeriodicityChanged_2 */
}

static void OnTxFrameCtrlChanged(LmHandlerMsgTypes_t isTxConfirmed)
{
  /* USER CODE BEGIN OnTxFrameCtrlChanged_1 */

  /* USER CODE END OnTxFrameCtrlChanged_1 */
  LmHandlerParams.IsTxConfirmed = isTxConfirmed;
  /* USER CODE BEGIN OnTxFrameCtrlChanged_2 */

  /* USER CODE END OnTxFrameCtrlChanged_2 */
}

static void OnPingSlotPeriodicityChanged(uint8_t pingSlotPeriodicity)
{
  /* USER CODE BEGIN OnPingSlotPeriodicityChanged_1 */

  /* USER CODE END OnPingSlotPeriodicityChanged_1 */
  LmHandlerParams.PingSlotPeriodicity = pingSlotPeriodicity;
  /* USER CODE BEGIN OnPingSlotPeriodicityChanged_2 */

  /* USER CODE END OnPingSlotPeriodicityChanged_2 */
}

static void OnSystemReset(void)
{
  /* USER CODE BEGIN OnSystemReset_1 */

  /* USER CODE END OnSystemReset_1 */
  if ((LORAMAC_HANDLER_SUCCESS == LmHandlerHalt()) && (LmHandlerJoinStatus() == LORAMAC_HANDLER_SET))
  {
    NVIC_SystemReset();
  }
  /* USER CODE BEGIN OnSystemReset_Last */

  /* USER CODE END OnSystemReset_Last */
}

static void StopJoin(void)
{
  /* USER CODE BEGIN StopJoin_1 */

  /* USER CODE END StopJoin_1 */

  UTIL_TIMER_Stop(&TxTimer);

  if (LORAMAC_HANDLER_SUCCESS != LmHandlerStop())
  {
    APP_LOG(TS_OFF, VLEVEL_M, "LmHandler Stop on going ...\r\n");
  }
  else
  {
    APP_LOG(TS_OFF, VLEVEL_M, "LmHandler Stopped\r\n");
    if (LORAWAN_DEFAULT_ACTIVATION_TYPE == ACTIVATION_TYPE_ABP)
    {
      ActivationType = ACTIVATION_TYPE_OTAA;
      APP_LOG(TS_OFF, VLEVEL_M, "LmHandler switch to OTAA mode\r\n");
    }
    else
    {
      ActivationType = ACTIVATION_TYPE_ABP;
      APP_LOG(TS_OFF, VLEVEL_M, "LmHandler switch to ABP mode\r\n");
    }
    LmHandlerConfigure(&LmHandlerParams);
    LmHandlerJoin(ActivationType, true);
    UTIL_TIMER_Start(&TxTimer);
  }
  UTIL_TIMER_Start(&StopJoinTimer);
  /* USER CODE BEGIN StopJoin_Last */

  /* USER CODE END StopJoin_Last */
}

static void OnStopJoinTimerEvent(void *context)
{
  /* USER CODE BEGIN OnStopJoinTimerEvent_1 */

  /* USER CODE END OnStopJoinTimerEvent_1 */
  if (ActivationType == LORAWAN_DEFAULT_ACTIVATION_TYPE)
  {
    UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaStopJoinEvent), CFG_SEQ_Prio_0);
  }
  /* USER CODE BEGIN OnStopJoinTimerEvent_Last */

  /* USER CODE END OnStopJoinTimerEvent_Last */
}

static void StoreContext(void)
{
  LmHandlerErrorStatus_t status = LORAMAC_HANDLER_ERROR;

  /* USER CODE BEGIN StoreContext_1 */

  /* USER CODE END StoreContext_1 */
  status = LmHandlerNvmDataStore();

  if (status == LORAMAC_HANDLER_NVM_DATA_UP_TO_DATE)
  {
    APP_LOG(TS_OFF, VLEVEL_M, "NVM DATA UP TO DATE\r\n");
  }
  else if (status == LORAMAC_HANDLER_ERROR)
  {
    APP_LOG(TS_OFF, VLEVEL_M, "NVM DATA STORE FAILED\r\n");
  }
  /* USER CODE BEGIN StoreContext_Last */

  /* USER CODE END StoreContext_Last */
}

static void OnNvmDataChange(LmHandlerNvmContextStates_t state)
{
  /* USER CODE BEGIN OnNvmDataChange_1 */

  /* USER CODE END OnNvmDataChange_1 */
  if (state == LORAMAC_HANDLER_NVM_STORE)
  {
    APP_LOG(TS_OFF, VLEVEL_M, "NVM DATA STORED\r\n");
  }
  else
  {
    APP_LOG(TS_OFF, VLEVEL_M, "NVM DATA RESTORED\r\n");
  }
  /* USER CODE BEGIN OnNvmDataChange_Last */

  /* USER CODE END OnNvmDataChange_Last */
}

static void OnStoreContextRequest(void *nvm, uint32_t nvm_size)
{
  /* USER CODE BEGIN OnStoreContextRequest_1 */

  /* USER CODE END OnStoreContextRequest_1 */
  /* store nvm in flash */
  if (HAL_FLASH_Unlock() == HAL_OK)
  {
    if (FLASH_IF_EraseByPages(PAGE(LORAWAN_NVM_BASE_ADDRESS), 1, 0U) == FLASH_OK)
    {
      FLASH_IF_Write(LORAWAN_NVM_BASE_ADDRESS, (uint8_t *)nvm, nvm_size, NULL);
    }
    HAL_FLASH_Lock();
  }
  /* USER CODE BEGIN OnStoreContextRequest_Last */

  /* USER CODE END OnStoreContextRequest_Last */
}

static void OnRestoreContextRequest(void *nvm, uint32_t nvm_size)
{
  /* USER CODE BEGIN OnRestoreContextRequest_1 */

  /* USER CODE END OnRestoreContextRequest_1 */
  UTIL_MEM_cpy_8(nvm, (void *)LORAWAN_NVM_BASE_ADDRESS, nvm_size);
  /* USER CODE BEGIN OnRestoreContextRequest_Last */

  /* USER CODE END OnRestoreContextRequest_Last */
}

