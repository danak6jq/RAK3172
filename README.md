# RAK2270-FreeRTOS
# UNDER CONSTRUCTION
Port of STM32WLxx example for RAK3172 using FreeRTOS



STM32CubeMX v6.16.1

STM32CubeIDE v2.0.0

This repo is an STM32CubeMX/STM32CubeIDE project created using STM32CubeMX. It builds a LoRaWAN mote using the STM32WLxx v 1.4.0 firmware library, configured to use the 'End-node Skeleton'. You may make changes to the project using STM32CubeMX and 'generate code' to reconfigure the project. Note that I may be able to help with simple questions but do not provide support for STM32CubeMX or STM32CubeIDE - the ST community forum is an excellent place to start for that.

Updated to STM32WLxx FW 1.4.0; this uses LoRaWAN v1.0.4 by default. Note the changed behavior of Join-Request nonces.

Unit tested in Class A and Class C.

BSP dependencies have been completely removed other than that of the RAK3172 module/SiP.

As usual, this is provided with zero warranty for usefulness or reliability. 
