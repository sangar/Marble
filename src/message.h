#pragma once

#include <stdint.h>

#include "nrf24.h"

// Every radio packet is exactly this many bytes.
enum { MESSAGE_SIZE = 4 };

// The gateway listens at the address of node 0.
enum { MESSAGE_GATEWAY = 0 };

typedef enum MessageKind : uint8_t {
    MESSAGE_HELLO = 1,       // node to gateway after boot, asking for its light level; value 0
    MESSAGE_LEVEL = 2,       // gateway to node: light level 0 (off) to 254 (full)
    MESSAGE_CONTACT = 3,     // node to gateway: 1 while the button is pressed, else 0
    MESSAGE_TEMPERATURE = 4, // node to gateway: hundredths of a degree Celsius
} MessageKind;

// A message is always about one node: the sender, or the receiver of MESSAGE_LEVEL.
typedef struct Message {
    uint8_t node;
    MessageKind kind;
    int16_t value;
} Message;

// True when node is 1-255, kind is known and value is in range for that kind.
[[nodiscard]] bool message_is_valid(Message message);

// Writes message as node, kind, value (little-endian).
void message_encode(Message message, uint8_t bytes[MESSAGE_SIZE]);

// Reads a received packet. Returns false when it is not a valid message.
[[nodiscard]] bool message_decode(const uint8_t bytes[MESSAGE_SIZE], Message *message);

// The radio address where node listens; MESSAGE_GATEWAY for the gateway.
void message_address(uint8_t node, uint8_t address[NRF24_ADDRESS_SIZE]);
