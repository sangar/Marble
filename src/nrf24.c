#include "nrf24.h"

#include "platform/board.h"

// Register map and commands from the nRF24L01+ Product Specification v1.0, chapters 8 and 9.
enum {
    NRF24_W_REGISTER = 0x20,
    NRF24_R_RX_PAYLOAD = 0x61,
    NRF24_W_TX_PAYLOAD = 0xA0,
    NRF24_FLUSH_TX = 0xE1,
    NRF24_FLUSH_RX = 0xE2,
    NRF24_NOP = 0xFF,
};

enum {
    NRF24_CONFIG = 0x00,
    NRF24_EN_AA = 0x01,
    NRF24_EN_RXADDR = 0x02,
    NRF24_SETUP_AW = 0x03,
    NRF24_SETUP_RETR = 0x04,
    NRF24_RF_CH = 0x05,
    NRF24_RF_SETUP = 0x06,
    NRF24_STATUS = 0x07,
    NRF24_RX_ADDR_P0 = 0x0A,
    NRF24_RX_ADDR_P1 = 0x0B,
    NRF24_TX_ADDR = 0x10,
    NRF24_RX_PW_P1 = 0x12,
    NRF24_FIFO_STATUS = 0x17,
};

enum {
    NRF24_POWERED_UP = 0x0E, // CONFIG: EN_CRC, CRCO (16-bit CRC), PWR_UP
    NRF24_PRIM_RX = 0x01,
    NRF24_RX_DR = 0x40,
    NRF24_TX_DS = 0x20,
    NRF24_MAX_RT = 0x10,
    NRF24_RX_EMPTY = 0x01,
    NRF24_PIPE_0 = 0x01,
    NRF24_PIPE_1 = 0x02,
};

static uint8_t nrf24_command(uint8_t command) {
    radio_select(true);
    uint8_t status = spi_transfer(command);
    radio_select(false);
    return status;
}

static uint8_t nrf24_read_register(uint8_t reg) {
    radio_select(true);
    spi_transfer(reg);
    uint8_t value = spi_transfer(NRF24_NOP);
    radio_select(false);
    return value;
}

static void nrf24_write_register(uint8_t reg, uint8_t value) {
    radio_select(true);
    spi_transfer(NRF24_W_REGISTER | reg);
    spi_transfer(value);
    radio_select(false);
}

static void nrf24_write_bytes(uint8_t command, const uint8_t *bytes, uint8_t count) {
    radio_select(true);
    spi_transfer(command);
    for (uint8_t i = 0; i < count; i++) spi_transfer(bytes[i]);
    radio_select(false);
}

static void nrf24_listen(void) {
    // Pipe 0 still holds the last address sent to, so that its acknowledgement arrives.
    // Left open, every node would also acknowledge packets meant for the gateway.
    nrf24_write_register(NRF24_EN_RXADDR, NRF24_PIPE_1);
    nrf24_write_register(NRF24_CONFIG, NRF24_POWERED_UP | NRF24_PRIM_RX);
    radio_enable(true);
    delay_us(130);
}

void nrf24_start(const Nrf24 *radio) {
    radio_enable(false);
    delay_ms(100);

    nrf24_write_register(NRF24_SETUP_AW, 0x03);   // 5-byte addresses
    nrf24_write_register(NRF24_SETUP_RETR, 0x5F); // 15 retries, 1500 µs apart
    nrf24_write_register(NRF24_RF_CH, 76);
    nrf24_write_register(NRF24_RF_SETUP, 0x06);   // 1 Mbps, 0 dBm
    nrf24_write_register(NRF24_EN_AA, NRF24_PIPE_0 | NRF24_PIPE_1);
    nrf24_write_register(NRF24_RX_PW_P1, radio->payload_size);
    nrf24_write_bytes(NRF24_W_REGISTER | NRF24_RX_ADDR_P1, radio->address, NRF24_ADDRESS_SIZE);
    nrf24_command(NRF24_FLUSH_RX);
    nrf24_command(NRF24_FLUSH_TX);
    nrf24_write_register(NRF24_STATUS, NRF24_RX_DR | NRF24_TX_DS | NRF24_MAX_RT);
    nrf24_write_register(NRF24_CONFIG, NRF24_POWERED_UP);
    delay_ms(5);

    nrf24_listen();
}

bool nrf24_send(const Nrf24 *radio, const uint8_t to[NRF24_ADDRESS_SIZE], const uint8_t *payload) {
    radio_enable(false);
    nrf24_write_register(NRF24_CONFIG, NRF24_POWERED_UP);
    nrf24_write_register(NRF24_EN_RXADDR, NRF24_PIPE_0 | NRF24_PIPE_1);
    nrf24_write_bytes(NRF24_W_REGISTER | NRF24_TX_ADDR, to, NRF24_ADDRESS_SIZE);
    nrf24_write_bytes(NRF24_W_REGISTER | NRF24_RX_ADDR_P0, to, NRF24_ADDRESS_SIZE);
    nrf24_command(NRF24_FLUSH_TX);
    nrf24_write_bytes(NRF24_W_TX_PAYLOAD, payload, radio->payload_size);

    radio_enable(true);
    delay_us(15);
    radio_enable(false);

    // 15 retries 1500 µs apart end within 25 ms; the limit only guards against a missing radio.
    uint8_t status = 0;
    for (uint16_t waited = 0; waited < 500; waited++) {
        status = nrf24_command(NRF24_NOP);
        if (status & (NRF24_TX_DS | NRF24_MAX_RT)) break;
        delay_us(100);
    }
    nrf24_write_register(NRF24_STATUS, NRF24_TX_DS | NRF24_MAX_RT);

    nrf24_listen();
    return status & NRF24_TX_DS;
}

bool nrf24_receive(const Nrf24 *radio, uint8_t *payload) {
    if (nrf24_read_register(NRF24_FIFO_STATUS) & NRF24_RX_EMPTY) return false;

    radio_select(true);
    spi_transfer(NRF24_R_RX_PAYLOAD);
    for (uint8_t i = 0; i < radio->payload_size; i++) payload[i] = spi_transfer(NRF24_NOP);
    radio_select(false);
    nrf24_write_register(NRF24_STATUS, NRF24_RX_DR);
    return true;
}
