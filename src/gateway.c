// The gateway: passes messages between the radio nodes and the bridge on the USB serial port.
// Each received message becomes a line "<node> <kind> <value>"; each such line from the bridge
// is sent to its node, and answered with "lost <node> <kind> <value>" if the node did not hear it.
// It writes "ready" on start.

#include "line.c"
#include "message.c"
#include "nrf24.c"
#include "platform/board.c"
#include "platform/uart.c"

static void gateway_write(const char *prefix, size_t prefix_length, Message message) {
    char line[LINE_CAPACITY];
    uart_write(prefix, prefix_length);
    uart_write(line, line_format(message, line));
}

static void gateway_forward_to_bridge(const Nrf24 *radio) {
    uint8_t bytes[MESSAGE_SIZE];
    Message message;
    while (nrf24_receive(radio, bytes)) {
        if (message_decode(bytes, &message)) gateway_write("", 0, message);
    }
}

static void gateway_forward_to_nodes(const Nrf24 *radio, LineReader *reader) {
    uint8_t byte;
    Message message;
    while (uart_read(&byte)) {
        if (!line_reader_push(reader, (char)byte)) continue;
        if (!line_parse(reader->text, reader->length, &message)) continue;

        uint8_t bytes[MESSAGE_SIZE];
        uint8_t address[NRF24_ADDRESS_SIZE];
        message_encode(message, bytes);
        message_address(message.node, address);
        if (!nrf24_send(radio, address, bytes)) gateway_write("lost ", 5, message);
    }
}

int main(void) {
    board_start();
    uart_start();

    Nrf24 radio = {.payload_size = MESSAGE_SIZE};
    message_address(MESSAGE_GATEWAY, radio.address);
    nrf24_start(&radio);
    uart_write("ready\n", 6);

    LineReader reader = {0};
    for (;;) {
        gateway_forward_to_bridge(&radio);
        gateway_forward_to_nodes(&radio, &reader);
    }
}
