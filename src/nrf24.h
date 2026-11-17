#pragma once

#include <stdint.h>

// Driver for the Nordic nRF24L01+ radio: fixed-size payloads with automatic
// acknowledgement and retries (Enhanced ShockBurst) on channel 76 at 1 Mbps.

enum { NRF24_ADDRESS_SIZE = 5 };

typedef struct Nrf24 {
    uint8_t address[NRF24_ADDRESS_SIZE]; // where this radio listens
    uint8_t payload_size;                // 1 to 32 bytes, the same on every radio
} Nrf24;

// Powers the radio up and starts listening. Takes about 105 ms.
void nrf24_start(const Nrf24 *radio);

// Sends payload_size bytes to the radio listening at `to` and listens again.
// Returns true when the receiver acknowledged them, within 15 retries.
[[nodiscard]] bool nrf24_send(const Nrf24 *radio, const uint8_t to[NRF24_ADDRESS_SIZE], const uint8_t *payload);

// Copies the oldest received payload into payload. Returns false when none is waiting.
[[nodiscard]] bool nrf24_receive(const Nrf24 *radio, uint8_t *payload);
