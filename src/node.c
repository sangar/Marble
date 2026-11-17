// A radio node: reports its button and temperature to the gateway and sets its light
// to the level the gateway sends. Built once per node with -DNODE_ID=1..255.

#include "message.c"
#include "nrf24.c"
#include "platform/board.c"
#include "platform/node_io.c"

static_assert(NODE_ID >= 1 && NODE_ID <= 255, "build with -DNODE_ID=1..255");

static const uint32_t DEBOUNCE_TICKS = CLOCK_TICKS_PER_SECOND / 50;
static const uint32_t RETRY_TICKS = CLOCK_TICKS_PER_SECOND;
static const uint32_t TEMPERATURE_TICKS = 60UL * CLOCK_TICKS_PER_SECOND;

typedef struct Node {
    Nrf24 radio;
    uint8_t gateway[NRF24_ADDRESS_SIZE];
    Clock clock;
    bool hello_acknowledged;
    bool button_sampled;
    uint32_t button_sampled_since;
    bool button_pressed;
    bool contact_reported;
    uint32_t retry_at;
    uint32_t temperature_at;
} Node;

static bool node_reached(uint32_t now, uint32_t deadline) {
    return (int32_t)(now - deadline) >= 0;
}

static bool node_send(const Node *node, MessageKind kind, int16_t value) {
    uint8_t bytes[MESSAGE_SIZE];
    message_encode((Message){.node = NODE_ID, .kind = kind, .value = value}, bytes);
    return nrf24_send(&node->radio, node->gateway, bytes);
}

static void node_receive(const Node *node) {
    uint8_t bytes[MESSAGE_SIZE];
    Message message;
    while (nrf24_receive(&node->radio, bytes)) {
        if (!message_decode(bytes, &message)) continue;
        if (message.node != NODE_ID || message.kind != MESSAGE_LEVEL) continue;
        light_set((uint8_t)message.value);
    }
}

static void node_debounce_button(Node *node, uint32_t now) {
    bool sampled = button_is_pressed();
    if (sampled != node->button_sampled) {
        node->button_sampled = sampled;
        node->button_sampled_since = now;
    } else if (now - node->button_sampled_since >= DEBOUNCE_TICKS) {
        node->button_pressed = sampled;
    }
}

// Sends hello until the gateway hears it, then each button change; retries every second.
static void node_report_state(Node *node, uint32_t now) {
    if (!node_reached(now, node->retry_at)) return;

    bool sent = true;
    if (!node->hello_acknowledged) {
        sent = node->hello_acknowledged = node_send(node, MESSAGE_HELLO, 0);
    } else if (node->button_pressed != node->contact_reported) {
        sent = node_send(node, MESSAGE_CONTACT, node->button_pressed);
        if (sent) node->contact_reported = node->button_pressed;
    }
    if (!sent) node->retry_at = now + RETRY_TICKS;
}

static void node_report_temperature(Node *node, uint32_t now) {
    if (!node_reached(now, node->temperature_at)) return;

    node_send(node, MESSAGE_TEMPERATURE, temperature_read());
    node->temperature_at = now + TEMPERATURE_TICKS;
}

int main(void) {
    board_start();
    node_io_start();

    // contact_reported starts opposite the button, so its state is sent after hello.
    Node node = {.radio.payload_size = MESSAGE_SIZE, .contact_reported = true, .clock = clock_start()};
    message_address(NODE_ID, node.radio.address);
    message_address(MESSAGE_GATEWAY, node.gateway);
    nrf24_start(&node.radio);

    for (;;) {
        uint32_t now = clock_now(&node.clock);
        node_receive(&node);
        node_debounce_button(&node, now);
        node_report_state(&node, now);
        node_report_temperature(&node, now);
    }
}
