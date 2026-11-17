#include "line.h"

bool line_reader_push(LineReader *reader, char byte) {
    if (reader->complete) {
        reader->length = 0;
        reader->complete = false;
    }
    if (byte == '\r') return false;

    if (byte == '\n') {
        bool fits = !reader->overflowed;
        reader->complete = fits;
        reader->overflowed = false;
        if (!fits) reader->length = 0;
        return fits;
    }

    if (reader->length == LINE_CAPACITY) {
        reader->overflowed = true;
        return false;
    }
    reader->text[reader->length++] = byte;
    return false;
}

// Reads an optionally negative decimal of at most six digits, followed by a space or the end.
static bool line_parse_number(const char **cursor, const char *end, int32_t *number) {
    const char *at = *cursor;
    bool negative = at < end && *at == '-';
    if (negative) at++;

    const char *digits = at;
    int32_t value = 0;
    while (at < end && *at >= '0' && *at <= '9' && at - digits < 6) {
        value = value * 10 + (*at - '0');
        at++;
    }
    if (at == digits) return false;
    if (at < end && *at != ' ') return false;
    if (at < end) at++;

    *cursor = at;
    *number = negative ? -value : value;
    return true;
}

bool line_parse(const char *text, size_t length, Message *message) {
    const char *at = text;
    const char *end = text + length;
    int32_t node, kind, value;
    if (!line_parse_number(&at, end, &node)) return false;
    if (!line_parse_number(&at, end, &kind)) return false;
    if (!line_parse_number(&at, end, &value)) return false;
    if (at != end || text[length - 1] == ' ') return false;
    if (node < 0 || node > UINT8_MAX || kind < 0 || kind > UINT8_MAX) return false;
    if (value < INT16_MIN || value > INT16_MAX) return false;

    Message parsed = {.node = (uint8_t)node, .kind = (MessageKind)kind, .value = (int16_t)value};
    if (!message_is_valid(parsed)) return false;

    *message = parsed;
    return true;
}

static size_t line_format_number(int32_t number, char *out) {
    char digits[6];
    size_t count = 0;
    size_t length = 0;
    if (number < 0) out[length++] = '-';

    uint32_t magnitude = number < 0 ? (uint32_t)-number : (uint32_t)number;
    do {
        digits[count++] = (char)('0' + magnitude % 10);
        magnitude /= 10;
    } while (magnitude > 0);

    while (count > 0) out[length++] = digits[--count];
    return length;
}

size_t line_format(Message message, char out[LINE_CAPACITY]) {
    size_t length = line_format_number(message.node, out);
    out[length++] = ' ';
    length += line_format_number(message.kind, out + length);
    out[length++] = ' ';
    length += line_format_number(message.value, out + length);
    out[length++] = '\n';
    return length;
}
