# Roadmap

## Vision

Someone with a drawer of Arduinos and nRF24L01+ radios flashes them with Marble. They plug one into a Mac or Raspberry Pi as the gateway, and the rest form a network on their own: nodes out of the gateway's range join through other nodes. They all show up in Apple Home, Google Home or Home Assistant as lights, buttons and temperature sensors. A web page shows the network and what happens on it. New firmware reaches the nodes over the radio, so a node never has to come down from the wall.

It is not a Matter device on its own: the 328 is far too small for Matter, so the bridge speaks Matter on the nodes' behalf. This file covers this repository: the firmware in `src/` and the bridge in `bridge/`.

Checked on 2026-11-17

## Focus

The firmware, gateway and bridge are written and pass `./nob ci`, but they have not yet run on real boards. This is a research project, so the order follows the riskiest assumption first:

1. **Real hardware:** a gateway and one node paired with a home app. See [Proof on hardware](#proof-on-hardware).
2. **Updates over the radio:** reflash a node without USB. See [Over-the-air updates](#over-the-air-updates).
3. **Mesh:** nodes out of the gateway's range join through other nodes. See [Mesh network](#mesh-network).
4. **Network page:** see the tree and an event log in a browser. See [Network page](#network-page).

Everything else waits.

## Principles

- **Research first, security later.** The chance of a stranger joining the network is close to zero at this stage. Keep the radio protocol open, and leave signing and join keys under [Feature ideas](#feature-ideas) until Marble runs somewhere that matters.
- **The bootloader is never overwritten over the radio.** Whatever fails, a node can always retry.
- **Nodes stay dumb, the gateway side decides.** Matter state, the network tree and routing live in the gateway and bridge. A node applies levels, reports what it senses and remembers its parent.

## Where we are

Marble was rewritten from the 2012 C++ library as C23. The GPL nRF24 driver was replaced with our own. The rewrite fixed two bugs: `AnalogIn` read an uninitialized pin, and Timer1 was set up with Timer2's bit.

| Part | State | Tests |
|------|-------|-------|
| Firmware (`src/`) | Builds: node 2.0 KB, gateway 2.8 KB | Host tests for the protocol with ASan/UBSan |
| Bridge (`bridge/`) | Runs against a fake gateway on a pty | `node --test`, `tsc` |

## Proof on hardware

Now. The radio driver, timing and wiring are untested on real chips.

- [ ] **A light, button and temperature work from a home app.**
      1. [ ] **Flash the gateway and node 1:** `./nob flash gateway PORT` and `./nob flash node 1 PORT`.
      2. [ ] **Pair the bridge:** run `node src/bridge.ts PORT 1` and pair it in Apple Home or Home Assistant.
      3. [ ] **Check each feature from the app:** dim the light, press the button, read the temperature.
      4. [ ] **Check range and recovery:** move the node out of range, then back, and check that it goes "No Response" and recovers.
      5. [ ] **Measure range:** note the distance and walls one hop covers. The mesh design depends on it.

## Over-the-air updates

After [Proof on hardware](#proof-on-hardware), and before the mesh, so OTA is not built twice. Today, reflashing a node needs a USB cable. The bar is MySensors' MYSBootloader, which updates 328 nodes over nRF24 but uses its own protocol. Done looks like `./nob ota node 3 PORT` reflashing a node across the room, and a transfer interrupted halfway still leaving the node running.

- [ ] **Reflash a node over the radio.**
      1. [ ] **Radio bootloader:** about 300 lines of C in the 328's boot section, taking 2–4 KB. On every reset it asks the gateway whether there is new firmware for node N.
         - If there is, it receives the image in 32-byte packets into the free upper half of flash (the app is about 2 KB of 32 KB). It checks the CRC, then copies the image over the app 128 bytes (one flash page) at a time using SPM, the chip's self-programming instruction.
         - It starts the app only if the app's CRC is valid. Otherwise it keeps asking.
      2. [ ] **Keep USB flashing:** add serial (STK500) support to the bootloader, for about 0.5 KB, so `./nob flash` keeps working.
      3. [ ] **Reboot into the bootloader:** add a new message kind that makes the running app reset into the bootloader.
      4. [ ] **Gateway and `nob`:**
         - The gateway takes `ota` lines from the serial port and passes the image to the node.
         - `./nob ota node ID PORT` sends `out/node-ID.hex` to the gateway.
      5. [ ] **Install the bootloader:** flash it once on each board with an ISP programmer (a spare Uno running ArduinoISP), and set the BOOTSZ and BOOTRST fuses.
      6. [ ] **Plan for hops:** once the mesh exists, every image packet crosses each hop, so transfer time grows with depth. Keep packets numbered and resumable from the start.

## Mesh network

After [Over-the-air updates](#over-the-air-updates). Today a node must be in direct range of the gateway. The bar is Zigbee and Thread: a network grows outward from the coordinator, routes through powered nodes and repairs itself, as RPL does for low-power IPv6 networks. Done looks like this: a node two rooms away joins through a node in between without any configuration, and moving it rebuilds only its branch.

The idea: the gateway finds the nodes it can hear and locks them as its children. Each locked node then looks for open nodes the gateway cannot reach, and so on outward. Each node gets one parent and the tree settles. Messages follow a routing table, and when a device is moved, its part of the network settles again.

- [ ] **Nodes out of range join through other nodes.**
      1. [ ] **Discovery address:** open nodes listen on a shared address with auto-ack switched off, since the nRF24 has no broadcast with acks. A node waits a random time before answering a scan, so answers don't collide.
      2. [ ] **Gateway-led discovery in waves:** the gateway tells one locked node at a time to scan, collects what it heard, and assigns parents.
         - Running it from the gateway rules out loops, keeps the only routing table on the gateway side, and gives one place to debug.
         - When several locked nodes hear an open node, pick the fewest hops first, then the fewest retries. Retries are counted in the `OBSERVE_TX` register. The radio's 1-bit signal-strength flag (strong/weak) is too crude.
      3. [ ] **Locked means saved:** a node stores its parent in EEPROM, so a power cut doesn't rebuild the network.
      4. [ ] **Routing:**
         - **Up:** a node sends everything to its parent.
         - **Down:** the gateway puts the whole path in the packet (source routing). Four hops fit in 32 bytes.
         - **Packets:** they grow from 4 to about 8 bytes plus the path: source, destination, hop count and sequence number.
      5. [ ] **End-to-end acknowledgement:** the radio's ack covers one hop only. The destination node acks the whole trip, so `lost` and "No Response" stay true.
      6. [ ] **Resettle:**
         - **Repair only the broken branch.** After 5 failed sends to its parent, a node becomes open again and the gateway re-adopts its branch. Its children follow, because they only know their parent.
         - **Manual resettle:** `./nob resettle` re-runs discovery for nodes that drifted onto a poor link that still works.

## Network page

After [Mesh network](#mesh-network), though the event log can start earlier. Today, the only view of the network is the bridge's console output. The bar is Zigbee2MQTT's network map and Home Assistant's logbook: a live graph of who routes through whom, link quality on each edge, and a filterable log of events. Done looks like opening a page and seeing the tree change as a node is unplugged.

The 328 gateway can't serve a web page, so the bridge serves the page on the local network.

- [ ] **See the network and its events in a browser.**
      1. [ ] **Event log:** the bridge records joins, parent changes, messages, `lost` lines, OTA progress and gateway restarts with timestamps, and streams them to the page.
      2. [ ] **Network graph:** the gateway at the root, nodes as circles marked reachable or silent, edges labelled with retry counts. It updates live.
      3. [ ] **Node details:** last heard, temperature, light level, firmware version and parent.
      4. [ ] **Actions:** resettle, start an OTA update and identify a node by blinking its LED.

## Feature ideas

**Worth doing soon**

- **Calibrated temperature:** store a per-node offset in EEPROM. Uncalibrated chips are off by up to ±10 °C.

**Not now**

Revisit these when Marble runs somewhere that matters:

- **Signed OTA images:** the radio is unencrypted, so anyone nearby could flash a node. Sign images with a shared key, using HMAC-SHA256 or an XTEA-based check, both small enough for the 328. One key for all nodes or one per node is still open.
- **Join key:** only nodes that know the shared key may join the mesh.
- **Login on the network page:** today it is only reachable on the local network.

**Not doing**

- **Matter OTA:** Matter's update protocol does not reach bridged devices.
- **DualOptiboot with external SPI flash:** the internal flash already has room for two images.
- **Flooding instead of routing:** it is simple, but with about 10 nodes the duplicates and collisions waste the little airtime there is.

## Open questions

- **Range per hop:** how far one hop reaches decides how deep the tree gets. To be measured in [Proof on hardware](#proof-on-hardware).
- **Who starts a scan wave:** the gateway on its own on a timer, or only when the network page or `nob` asks? Leaning on request only, plus at gateway start. Decide when building discovery.
