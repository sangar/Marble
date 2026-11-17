#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/line.c"
#include "../src/message.c"

static bool parses(const char *text, Message *message) {
    return line_parse(text, strlen(text), message);
}

static void test_message_round_trips_through_bytes(void) {
    Message sent = {.node = 7, .kind = MESSAGE_TEMPERATURE, .value = -1250};
    uint8_t bytes[MESSAGE_SIZE];
    message_encode(sent, bytes);

    uint8_t expected[MESSAGE_SIZE] = {7, 4, 0x1E, 0xFB};
    assert(memcmp(bytes, expected, MESSAGE_SIZE) == 0);

    Message received;
    assert(message_decode(bytes, &received));
    assert(received.node == 7 && received.kind == MESSAGE_TEMPERATURE && received.value == -1250);
}

static void test_message_decode_rejects_invalid_packets(void) {
    Message message;
    assert(!message_decode((uint8_t[]){0, MESSAGE_LEVEL, 10, 0}, &message));
    assert(!message_decode((uint8_t[]){3, 9, 0, 0}, &message));
    assert(!message_decode((uint8_t[]){3, MESSAGE_LEVEL, 255, 0}, &message));
    assert(!message_decode((uint8_t[]){3, MESSAGE_CONTACT, 2, 0}, &message));
}

static void test_line_parse_accepts_messages(void) {
    Message message;
    assert(parses("3 2 128", &message));
    assert(message.node == 3 && message.kind == MESSAGE_LEVEL && message.value == 128);

    assert(parses("255 4 -32768", &message));
    assert(message.node == 255 && message.value == -32768);
}

static void test_line_parse_rejects_everything_else(void) {
    const char *invalid[] = {
        "", "3 2", "3 2 128 9", "3 2 128 ", "3  2 128", "0 2 1", "3 9 1", "3 2 255",
        "3 4 32768", "256 2 1", "3 2 -", "3 2 1x", "-3 2 1", "3 4 9999999",
    };
    Message message;
    for (size_t i = 0; i < sizeof invalid / sizeof invalid[0]; i++) {
        assert(!parses(invalid[i], &message));
    }
}

static void test_line_format_writes_what_parse_reads(void) {
    char out[LINE_CAPACITY];
    Message longest = {.node = 255, .kind = MESSAGE_TEMPERATURE, .value = -32768};
    size_t length = line_format(longest, out);
    assert(length == strlen("255 4 -32768\n"));
    assert(memcmp(out, "255 4 -32768\n", length) == 0);

    Message parsed;
    assert(line_parse(out, length - 1, &parsed));
    assert(parsed.node == 255 && parsed.value == -32768);
}

static void test_line_reader_splits_lines_and_drops_long_ones(void) {
    const char *stream = "3 2 1\r\n0123456789abcdefXYZ\n4 2 9\n";
    LineReader reader = {0};
    char lines[2][LINE_CAPACITY + 1] = {0};
    size_t count = 0;
    for (const char *at = stream; *at; at++) {
        if (!line_reader_push(&reader, *at)) continue;
        assert(count < 2);
        memcpy(lines[count++], reader.text, reader.length);
    }
    assert(count == 2);
    assert(strcmp(lines[0], "3 2 1") == 0);
    assert(strcmp(lines[1], "4 2 9") == 0);
}

int main(void) {
    test_message_round_trips_through_bytes();
    test_message_decode_rejects_invalid_packets();
    test_line_parse_accepts_messages();
    test_line_parse_rejects_everything_else();
    test_line_format_writes_what_parse_reads();
    test_line_reader_splits_lines_and_drops_long_ones();
    puts("tests passed");
    return 0;
}
