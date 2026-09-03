# ESP32 Bluetooth RC Car

A four-wheel RC car built on an ESP32 and a TB6612FNG dual motor driver,
controlled over BLE from a phone. Differential steering, variable speed
from a slider, plus a headlight and horn.


image 1: <img width="3024" height="4032" alt="IMG_2378" src="https://github.com/user-attachments/assets/92079863-1a7d-42f2-b333-b7cb9ce5477c" />

image 2:<img width="3024" height="4032" alt="IMG_2349" src="https://github.com/user-attachments/assets/a54f0b30-5898-4ca0-bd5f-315c76c1cfdf" />

Motor demo video : https://github.com/user-attachments/assets/e6552586-a6c7-4ddc-b1e2-4fee13466d1f


## How it works
- The ESP32 runs a BLE GATT server that advertises as `RC_CAR`. The phone
  writes short command strings to a single characteristic
- Movement commands map to a differential drive: both motors forward or
  reverse to move, opposite directions to turn in place
- The speed slider sends `Speed_0` to `Speed_100`, rescaled to the 8-bit
  PWM duty cycle the TB6612 expects. Speed changes take effect mid-drive
  because the last command is re-applied on every write
- If the BLE link drops, a disconnect callback brakes both motors, kills
  the buzzer, and restarts advertising so the phone can reconnect without
  power cycling the board

## Hardware
| Part | Detail |
|---|---|
| MCU | ESP32 DevKit v1 |
| Motor driver | TB6612FNG breakout, dual H-bridge |
| Motors | 4x DC gear motors |
| Power | 2x 18650 Li-ion in series |
| Chassis | 3D printed |
| Extras | LED headlight, piezo buzzer |
| Build | Breadboard on chassis, jumper wires |

The driver takes motor voltage on VM and logic voltage on VCC as separate
rails, with STBY held high to enable the outputs.

## Power
The battery pack feeds VM directly while the ESP32 logic runs at 3.3 V,
so the two supplies had to share a common ground for the driver's logic
inputs to read correctly. Getting this wrong was the main source of
debugging time on this build: the board would enumerate over USB and the
motors would do nothing.

## Things I'd change
- Add a watchdog in `loop()` that stops the car if no command has arrived
  in the last few hundred milliseconds. Right now a frozen app leaves the
  last command latched
- Move off the breadboard onto a soldered protoboard or a custom PCB.
  Vibration pulls jumpers loose
- Add current sensing on the motor rail to detect a stall
- Replace `tone()` with the ESP32 LEDC PWM API, which is the native way
  to drive the buzzer on this chip
