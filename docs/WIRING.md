# Wiring

## Pin map

| Peripheral | Pico 2 pin | Notes |
| --- | --- | --- |
| VS1838B OUT | GP5 | NEC infrared input |
| Button | GP15 | to ground when pressed |
| RYLR998 TX | GP8 | UART1 TX to module RX |
| RYLR998 RX | GP9 | UART1 RX from module TX |
| Red LED | GP16 | through a 220 or 330 ohm resistor |
| Yellow LED | GP17 | through a 220 or 330 ohm resistor |
| Green LED | GP18 | through a 220 or 330 ohm resistor |
| Onboard LED | GP25 | heartbeat |
| Debug Probe | SWCLK / SWDIO / GND | SWD |
| Debug Probe UART | GP0 / GP1 | UART0 console |

## Power

- VS1838B VCC to 3.3 V.
- RYLR998 VCC to 3.3 V.
- All grounds common.

## Breadboard

1. Seat the Pico 2 across the center channel.
2. Wire the three LEDs with their resistors to GP16, GP17, and GP18, cathodes
   to the ground rail.
3. Wire the VS1838B OUT to GP5, VCC to 3.3 V, GND common.
4. Wire the button between GP15 and the ground rail.
5. Wire the RYLR998 TX to GP9 and RX to GP8, VCC to 3.3 V, GND common.
6. Connect the Debug Probe to SWCLK, SWDIO, and GND, and its UART to GP0/GP1.

## Gateway

- Connect the second RYLR998 to a USB serial adapter and the computer.
- On macOS the adapter appears as `/dev/cu.usbserial-A50285BI`.
