/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    lora_app.h
  * @author  MCD Application Team
  * @brief   Header of application of the LRWAN Middleware
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
#ifndef __LORA_APP_H__
#define __LORA_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "stdint.h"
#include "LmHandlerTypes.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/

/* LoraWAN application configuration (Mw is configured by lorawan_conf.h) */
#define ACTIVE_REGION                               LORAMAC_REGION_US915

/*!
 * CAYENNE_LPP is myDevices Application server.
 */
/*#define CAYENNE_LPP*/

/*!
 * Defines the application data transmission duty cycle. 10s, value in [ms].
 */
#define APP_TX_DUTYCYCLE                            7200000

/*!
 * LoRaWAN User application port
 * @note do not use 224. It is reserved for certification
 */
#define LORAWAN_USER_APP_PORT                       2

/*!
 * LoRaWAN Switch class application port
 * @note do not use 224. It is reserved for certification
 */
#define LORAWAN_SWITCH_CLASS_PORT                   3

/*!
 * LoRaWAN default class
 */
#define LORAWAN_DEFAULT_CLASS                       CLASS_C

/*!
 * LoRaWAN default confirm state
 */
#define LORAWAN_DEFAULT_CONFIRMED_MSG_STATE         LORAMAC_HANDLER_UNCONFIRMED_MSG

/*!
 * LoRaWAN Adaptive Data Rate
 * @note Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_STATE                           LORAMAC_HANDLER_ADR_ON

/*!
 * LoRaWAN Default data Rate Data Rate
 * @note Please note that LORAWAN_DEFAULT_DATA_RATE is used only when LORAWAN_ADR_STATE is disabled
 */
#define LORAWAN_DEFAULT_DATA_RATE                   DR_0

/*!
 * LoRaWAN default activation type
 */
#define LORAWAN_DEFAULT_ACTIVATION_TYPE             ACTIVATION_TYPE_OTAA

/*!
 * LoRaWAN force rejoin even if the NVM context is restored
 * @note useful only when context management is enabled by CONTEXT_MANAGEMENT_ENABLED
 */
#define LORAWAN_FORCE_REJOIN_AT_BOOT                true

/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_BUFFER_MAX_SIZE            242

/*!
 * Default Unicast ping slots periodicity
 *
 * \remark periodicity is equal to 2^LORAWAN_DEFAULT_PING_SLOT_PERIODICITY seconds
 *         example: 2^4 = 16 seconds. The end-device will open an Rx slot every 16 seconds.
 */
#define LORAWAN_DEFAULT_PING_SLOT_PERIODICITY       4

/*!
 * Default response timeout for class b and class c confirmed
 * downlink frames in milli seconds.
 *
 * The value shall not be smaller than RETRANSMIT_TIMEOUT plus
 * the maximum time on air.
 */
#define LORAWAN_DEFAULT_CLASS_B_C_RESP_TIMEOUT      8000

/* USER CODE BEGIN EC */

/*
 * UL formats
 *
 * STATUS (port 4)
 * 	byte 0: UL_ID_STATUS (0x01)
 * 	byte 1:
 * 		bit 0: switch state
 * 		bit 1: input state
 * 		bit 2: RFU
 * 		bit 3: RFU
 * 		bit 4: switch state changed
 * 		bit 5: input state changed
 * 		bit 6: RFU
 * 		bit 7: RFU
 *
 * ERROR (port 5)
 * 	byte 0: UL_ID_ERROR (0x03)
 * 	byte 1: error code RFU
 *
 * ID/VERSION (port 240)
 * 	byte 0: UL_ID_ID (0x00)
 * 	byte 1, 2: MOTE_ID (Big-endian)
 * 	byte 2, 3: FW_VERSION (Big-endian)
 *
 * OTA configuration: RFU
 *
 * DL formats
 *
 * SWITCH (port 4)
 * 	byte 0: DL_ID_SWITCH (0x00)
 * 	byte 1:
 * 		bit 0: switch state
 * 		bit 1-7: RFU
 * 	If switch is being turned-on, include the period
 * 	byte 2..4: 24-bit on time in seconds (BE)
 *
 *	If switch is being turned-off, do not include the period
 *
 * OTA configuration: RFU
 *
 * Periodic UL:
 *   UL_ID_STATUS every 2 hours, confirmed, Link Check Req
 *
 * UL Queue
 *
 */

/*
 * Uplink Queue element types
 */
#define	UL_ID_ID			0		/* FW ID/version payload */
#define	UL_ID_STATUS		1		/* current status bits */
#define	UL_ID_ERROR			3		/* unknown DL received */

#define	UL_PORT_ID			240
#define	UL_PORT_STATUS		4
#define	UL_PORT_ERROR		5

#define	DL_ID_SWITCH		0		/* set switch state */

#define	DL_PORT_SWITCH		4

#define	UL_CONFIRMED		1		/* UL confirmed */
#define	UL_UNCONFIRMED		0		/* UL unconfirmed */

#define	UL_MAX_PAYLOAD		5		/* for ID/VERSION */

#define	UPLINK_QUEUE_NUM	11		/* one entry is sentinel */

#define	UplinkQueueNext(x)	(((x) + 1) >= UPLINK_QUEUE_NUM ? 0 : (x) + 1)

typedef struct {
	uint8_t		uplinkType;
	/* confirmedCount: if 0, unconfirmed, otherwise max re-sends */
	/* re-send happens when payload size is too large for fOpts/DR */
	uint8_t		upLinkConfirmedCount;
	/* private buffer, filled in when sent */
	uint8_t		upLinkPayload[UL_MAX_PAYLOAD];
	LmHandlerAppData_t appData;
} UplinkQueue_t;


/* USER CODE END EC */

/* Exported macros -----------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/**
  * @brief  Init Lora Application
  */
void LoRaWAN_Init(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /*__LORA_APP_H__*/
