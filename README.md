# RAK3172
Port of STM32WLxx example for RAK3172

Major Update 14 Jan 2026:

Branch 'RAK3172' is for the RAK3172 module

Branch 'RAK3172-SiP' is for the RAK3172-SiP package

STM32CubeMX v6.16.1

STM32CubeIDE v2.0.0

This repo is an STM32CubeMX/STM32CubeIDE project created using STM32CubeMX. It builds a LoRaWAN mote using the STM32WLxx v 1.4.0 firmware library, configured to use the 'End-node Skeleton'. You may make changes to the project using STM32CubeMX and 'generate code' to reconfigure the project. Note that I may be able to help with simple questions but do not provide support for STM32CubeMX or STM32CubeIDE - the ST community forum is an excellent place to start for that.

Updated to STM32WLxx FW 1.4.0; this uses LoRaWAN v1.0.4 by default. Note the changed behavior of Join-Request nonces.

Unit tested in Class A and Class C.

BSP dependencies have been completely removed other than that of the RAK3172 module/SiP.

I apologize to those with clones of this repo for the mayhem; I wanted to get this cleaned-up and organized in a rational way going-forward. You may need to change your git remote before a pull, and that first pull might be a bit of work if you've made changes in the Drivers/BSP directory which no longer exists. I suggest putting non-LoRaWAN board support into a new directory underneath/next to LoRaWAN/App perhaps.

As usual, this is provided with zero warranty for usefulness or reliability. 
