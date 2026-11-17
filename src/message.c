#include "message.h"

bool message_is_valid(Message message) {
    if (message.node == 0) return false;

    switch (message.kind) {
    case MESSAGE_HELLO:
        return message.value == 0;
    case MESSAGE_LEVEL:
        return message.value >= 0 && message.value <= 254;
    case MESSAGE_CONTACT:
        return message.value == 0 || message.value == 1;
    case MESSAGE_TEMPERATURE:
        return true;
    }
    return false;
}

void message_encode(Message message, uint8_t bytes[MESSAGE_SIZE]) {
    uint16_t value = (uint16_t)message.value;
    bytes[0] = message.node;
    bytes[1] = message.kind;
    bytes[2] = (uint8_t)(value & 0xFF);
    bytes[3] = (uint8_t)(value >> 8);
}

bool message_decode(const uint8_t bytes[MESSAGE_SIZE], Message *message) {
    Message decoded = {
        .node = bytes[0],
        .kind = (MessageKind)bytes[1],
        .value = (int16_t)(bytes[2] | (uint16_t)(bytes[3] << 8)),
    };
    if (!message_is_valid(decoded)) return false;

    *message = decoded;
    return true;
}

void message_address(uint8_t node, uint8_t address[NRF24_ADDRESS_SIZE]) {
    static const uint8_t prefix[NRF24_ADDRESS_SIZE - 1] = {'M', 'R', 'B', 'L'};
    for (uint8_t i = 0; i < sizeof prefix; i++) address[i] = prefix[i];
    address[NRF24_ADDRESS_SIZE - 1] = node;
}
