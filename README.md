# RAK3172
Port of STM32WLxx example for RAK3172

Quick update to V1.2 of ST firmware package; unit tested as a Class C mote

Update 14 July 2022:
Reviewed merge of V1.2 ST FW and found a number of issues, mostly in the lora_app.c merge. I think I've fixed battery level reporting. Cleaned-up LED control code (though LEDs don't have a clear functionality, this could use some more app-specific attention). Added implementation of v1.0.4. Moved GPIO initialization to match ST LoRa_End_Node example. Unit-tested in Class C and Class A. Changed some defaults in the IOC file (use ST-provided EUI, v1.0.4, Class A, debugger disabled).
I'm happy to merge PRs with different defaults when there's enough user request.

Branch 'relay-control' is an experimental wad that uses some custom hardware we have to control a latching relay. It implements a more sophisticated on-with-time, off protocol and a first stab at queueing message components, nothing fancy and certainly not very tested. Also included is LoRaWAN 1.0.4 support, which most notably requires enabling the "context manager" in the LoRaWAN to maintain non-volatile context (devNonce at Join needs to be non-volatile for v1.0.4).

As usual, this is provided with zero warranty for usefulness or reliability. 
