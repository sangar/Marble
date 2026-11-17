# Marble

ATmega328 + nRF24L01+ radio nodes in Apple Home, Google Home or Home Assistant, through Matter.

```text
[node: 328 + nRF24] ──radio──▶ [gateway: 328 + nRF24] ──USB──▶ [bridge: matter.js] ──Wi-Fi/Ethernet──▶ Matter controller
```

Each node shows up as three accessories:

- **Light:** an LED on D6, dimmable.
- **Button:** a button from D2 to ground, shown as a contact sensor that is closed while pressed.
- **Temperature:** the chip's own sensor, reported every minute. It is rough: up to ±10 °C until calibrated.

## Wiring

Gateway and nodes are Arduino Unos or Nanos wired the same way.

| nRF24L01+ | Arduino |
|-----------|---------|
| VCC | 3.3V, never 5V. Put a 10 µF capacitor between VCC and GND. |
| GND | GND |
| CE | D9 |
| CSN | D10 |
| SCK | D13 |
| MOSI | D11 |
| MISO | D12 |
| IRQ | not connected |

On a node, connect an LED with a resistor from D6 to GND, and a button from D2 to GND.

## Build and flash

You need avr-gcc 15, avrdude and Node.js 24:

```sh
brew tap osx-cross/avr && brew install osx-cross/avr/avr-gcc@15 avrdude
```

Build and check everything, then flash each board:

```sh
cc -std=c23 -o nob tools/build.c
./nob ci
./nob flash gateway /dev/cu.usbmodem1101
./nob flash node 1 /dev/cu.usbserial-110
```

Give each node its own ID from 1 to 255. A Nano with the old bootloader flashes at 57600 baud: `BAUD=57600 ./nob flash node 2 PORT`.

## Run the bridge

Start the bridge with the gateway's port and the ID of every node:

```sh
cd bridge && npm ci
node src/bridge.ts /dev/cu.usbmodem1101 1 2 3
```

On first start it prints a manual pairing code; enter it in your home app. The bridge uses a test vendor ID, so Apple Home warns that the accessory is uncertified: choose Add Anyway.

## Protocol

**Radio:** channel 76, 1 Mbps, with automatic acknowledgement and up to 15 retries.

- **Packets:** 4 bytes each: node ID, kind, then a little-endian signed 16-bit value.
- **Addresses:** each node listens at `MRBL` followed by its ID byte. The gateway listens as node 0.

**Serial:** 115200 baud, one message per line as `<node> <kind> <value>`. The gateway also writes:

- `ready` when it starts
- `lost <node> <kind> <value>` when a node did not acknowledge a message

| Kind | Name | Direction | Value |
|------|------|-----------|-------|
| 1 | hello | node → gateway | 0; the node just started and asks for its light level |
| 2 | level | gateway → node | 0 (off) to 254 (full) |
| 3 | contact | node → gateway | 1 while the button is pressed, else 0 |
| 4 | temperature | node → gateway | hundredths of a degree Celsius |

The radio link is not encrypted: anyone nearby with an nRF24 can read and fake messages.

## Profile

```text
Language:   C23
Compilers:  avr-gcc >= 15 (firmware), clang >= 18 (host tests)
Level:      Modern C Level 2, with the exceptions below
```

The firmware runs on a microcontroller with 2 KB of RAM and no operating system, so it departs from the contract in these ways:

- **No libmc:** libmc needs an operating system. The firmware allocates nothing, and functions return `bool` where libmc would return `Error`.
- **One global:** the serial receive buffer, which is filled by an interrupt handler.
- **One firmware target:** the ATmega328P. The only host build is the tests.
- **No install or uninstall:** the firmware is flashed with `./nob flash` instead.
