![picokit-35-annunciator-ack](https://raw.githubusercontent.com/mytechnotalent/picokit-35-annunciator-ack/main/picokit-35-annunciator-ack.png)

<br>

## FREE Reverse Engineering Self-Study Course [HERE](https://github.com/mytechnotalent/reverse-engineering)
## FREE Embedded Hacking Course [HERE](https://github.com/mytechnotalent/Embedded-Hacking)

<br>

# PICOKIT-35 ANNUNCIATOR ACK

### Latching Alarm LED, Local Acknowledge, and an Authenticated Heartbeat
#### Lesson 35 of the Picokit Series

<br>

***
**LEGAL DISCLAIMER:**
The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only.

You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with.

By using this repository and course, you acknowledge and agree that:

1. Any illegal, unauthorized, or malicious use of this information is solely your responsibility.
2. The author(s) and contributor(s) of this repository and course shall not be held liable for any damages, legal repercussions, criminal charges, or unauthorized actions resulting from the use, misuse, or abuse of the contents herein.
3. You will comply with all applicable local, state, national, and international laws regarding cybersecurity and computer fraud.

**IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**
***

<br>
<br>

## Overview

The thirty-fifth Picokit lesson. The node raises a latching annunciator from a
plaintext gateway notice and drives the red lamp. The annunciator holds until
an operator acknowledges it on the button or the infrared remote, and the
authenticated heartbeat reports the latched state.

<br>

## What it teaches

- Raising a latching annunciator from a gateway notice.
- Driving an alarm lamp from the latched state.
- Acknowledging from a local button or the remote key.
- Reporting the annunciator state in the heartbeat body.

<br>

## Hardware

| Peripheral | Pico 2 pin | Role |
| --- | --- | --- |
| VS1838B | GP5 | remote acknowledge |
| Button | GP15 | local acknowledge |
| Red / Yellow / Green | GP16 / GP17 / GP18 | annunciator state |
| Onboard LED | GP25 | heartbeat, one blink per transmit |
| RYLR998 | GP8 TX / GP9 RX | LoRa notice and heartbeat |
| Debug Probe | SWCLK/SWDIO/GND, GP0/GP1 | SWD and the console |

<br>

## How it works

The node runs `monitor_step` in a loop. An inbound `+RCV` line whose payload is
`ALARM` latches the annunciator and drives the red lamp. The button or the
remote `0x45` key clears the latch and drives the green lamp. Every 5 seconds
the node transmits an authenticated heartbeat whose body is
`{"n":35,"s":<seq>,"s":<state>}` sealed with the field key.

<br>

## Build and flash

```bash
cd firmware
cmake -S . -B build -G Ninja -DPICO_BOARD=pico2 -DPICO_PLATFORM=rp2350-arm-s
cmake --build build
openocd -f interface/cmsis-dap.cfg -f target/rp2350.cfg \
  -c "program build/picokit_35_annunciator_ack.elf verify reset exit"
```

<br>

## Watch the node

Open the console at 115200 and reset:

```text
BOOT
=== PICOKIT-35 ANNUNCIATOR ACK // LATCHED LED + AUTHENTICATED HEARTBEAT ===
RX from 0x0001, 5 bytes
ALARM
ACK
```

<br>

## The gateway

```bash
cd gateway
python3 -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt
python3 listen.py --port /dev/cu.usbserial-A50285BI --hub 0001 --network 18 --db gateway.db
```

It prints `OK node=35 rssi=...` per authenticated heartbeat. The terminal
dashboard `python3 tui.py --db gateway.db` and the web dashboard
`python3 web/app.py --db gateway.db` show the same rows.

<br>

## Verify

```bash
python3 .opencode/skill/embedded-c-standard/audit_c_standard.py
python3 .opencode/skill/embedded-python-standard/audit_python_standard.py
python3 .opencode/skill/iot-readme-standard/validate_readme.py
python3 .opencode/skill/iot-banner-standard/validate_banner.py
python3 scripts/run_tests.py
python3 scripts/check_coverage.py
```

<br>

# Next
[picokit-36-scheduler](https://github.com/mytechnotalent/picokit-36-scheduler)

<br>

# License
[MIT License](https://github.com/mytechnotalent/picokit-35-annunciator-ack/blob/main/LICENSE)
