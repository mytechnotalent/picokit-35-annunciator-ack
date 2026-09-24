# Parts

## Bill of materials

| Qty | Part | Role |
| --- | --- | --- |
| 1 | Raspberry Pi Pico 2 (with header) | the node |
| 1 | Raspberry Pi Pico Debug Probe | SWD flash and debug, UART0 console |
| 2 | USB A to USB Micro-B cable | one for the Pico, one for the Debug Probe |
| 1 | Full-size breadboard | assembly |
| 1 | Jumper wire set (M-M, M-F, F-F) | assembly |
| 1 | VS1838B infrared receiver | remote acknowledge |
| 1 | NEC-compatible infrared remote | remote acknowledge |
| 1 | Tactile push-button | local acknowledge |
| 3 | 5 mm LEDs (red, yellow, green) | status lamps |
| 3 | 220 or 330 ohm resistors | LED current limit |
| 2 | RYLR998 LoRa module | one on the node, one on the gateway |

## Roles

- Node: Pico 2 plus every peripheral above except the second RYLR998.
- Gateway: the computer with the second RYLR998 on a USB serial adapter.
- Debug Probe: flashing, SWD debugging, and the UART0 console.

## Notes

- The servo draws current spikes; the 1000 uF capacitor across its 5 V rail is
  required to keep the Pico from browning out.
- Keep the LoRa antenna clear of the servo and its wiring.
