# RAK3172
Port of STM32WLxx example for RAK3172

UPDATE 9 May 2023: the FW1.3 branch is the result of letting STM32CubeIDE update to FW V1.3. It hasn't been tested yet but initial review suggests it'll work fine. If you test this before I do, please let me know I'll merge it to master.

Quick update to V1.2 of ST firmware package; unit tested as a Class C mote

Update 14 July 2022:
Reviewed merge of V1.2 ST FW and found a number of issues, mostly in the lora_app.c merge. I think I've fixed battery level reporting. Cleaned-up LED control code (though LEDs don't have a clear functionality, this could use some more app-specific attention). Added implementation of v1.0.4. Moved GPIO initialization to match ST LoRa_End_Node example. Unit-tested in Class C and Class A. Changed some defaults in the IOC file (use ST-provided EUI, v1.0.4, Class A, debugger disabled).
I'm happy to merge PRs with different defaults when there's enough user request.

NOTE: ********* bug in ST firmware requires workaround

Note there's an issue with:
```
/**
  * @brief  Perform an ADC automatic self-calibration
  *         Calibration prerequisite: ADC must be disabled (execute this
  *         function before HAL_ADC_Start() or after HAL_ADC_Stop() ).
  * @note   Calibration factor can be read after calibration, using function
  *         HAL_ADC_GetValue() (value on 7 bits: from DR[6;0]).
  * @param  hadc       ADC handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ADCEx_Calibration_Start(ADC_HandleTypeDef *hadc)
```

right now - add the lines with stars below after every Cube regen. ST HAL bug.
```
    /* Compute average */
    calibration_factor_accumulated /= calibration_index;
    /* Apply calibration factor */
    LL_ADC_Enable(hadc->Instance);

    /* add this block of code */
    while (!LL_ADC_IsActiveFlag_ADRDY(hadc->Instance))  // ***
    	/* spin */;                                        // ***
    LL_ADC_ClearFlag_ADRDY(hadc->Instance);             // ***

    LL_ADC_SetCalibrationFactor(hadc->Instance, calibration_factor_accumulated);
    LL_ADC_Disable(hadc->Instance);
```

As usual, this is provided with zero warranty for usefulness or reliability. 
