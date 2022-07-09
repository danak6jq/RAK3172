# RAK3172
Port of STM32WLxx example for RAK3172

Quick update to V1.2 of ST firmware package; unit tested as a Class C mote

Branch 'relay-control' is an experimental wad that uses some custom hardware we have to control a latching relay. It implements a more sophisticated on-with-time, off protocol and a first stab at queueing message components, nothing fancy and certainly not very tested. Also included is LoRaWAN 1.0.4 support, which most notably requires enabling the "context manager" in the LoRaWAN to maintain non-volatile context (devNonce at Join needs to be non-volatile for v1.0.4).

As usual, this is provided with zero warranty for usefulness or reliability. 
