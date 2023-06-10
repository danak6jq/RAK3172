# RAK3172
Port of STM32WLxx example for RAK3172

Initial port of ST example code for RAK3172 to RAK3172-SiP. Performed a bit of cleaning-up of left-over code in the example - it's still not entirely cleaned-up but the LoRaWAN function is complete. Differences between RAK3172 and RAK3172-SiP:
- RAK3172-SiP uses PA0/PA1 for RF switch internally.
- RAK3172-SiP contain a TCXO rather than xtal of RAK3172. This implicitly uses PB0 to switch TCXO power.

I found I needed to configure the ADC specifically as in another application to see minimum STOP2 current; it's possible I'm mistreating the ADC otherwise.

With this project I'm seeing under 2uA STOP2 current (typically around 1.3uA) though I wonder how that's influenced by decoupling capacitors on the RAK3172-SiP breakout board. More than once I've briefly disconnected the external 3.3V supply to the breakout board during sleep without interrupting MCU operation :).

My test hardware uses a Microchip MCP1702 three-pin 3v3 LDO with 1u decoupling caps.

Quick update to V1.2 of ST firmware package; unit tested as a Class C mote

Update 14 July 2022:
Reviewed merge of V1.2 ST FW and found a number of issues, mostly in the lora_app.c merge. I think I've fixed battery level reporting. Cleaned-up LED control code (though LEDs don't have a clear functionality, this could use some more app-specific attention). Added implementation of v1.0.4. Moved GPIO initialization to match ST LoRa_End_Node example. Unit-tested in Class C and Class A. Changed some defaults in the IOC file (use ST-provided EUI, v1.0.4, Class A, debugger disabled).
I'm happy to merge PRs with different defaults when there's enough user request.

As usual, this is provided with zero warranty for usefulness or reliability. 
