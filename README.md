# RAK3172
Port of STM32WLxx example for RAK3172

May-Queen branch: cleanup of ST LoRaWAN_End_Node example code. Produced by generating a new project from the IOC file, resulting in a 'clean' end-node framework without the ST example code. To this empty framework, the RAK3172-specific BSP files are added-in (renamed from the ST Nucleo files in master) and minimum necessary hooks are added back to lora_app.c (improved Join behavior, lengthened JOIN timer)

This branch will eventually merge back to master.

As usual, this is provided with zero warranty for usefulness or reliability. 
