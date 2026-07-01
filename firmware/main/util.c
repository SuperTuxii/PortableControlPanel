#include "util.h"

int32_t inline convertDataToInt32(const uint8_t *data) {
    return data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
}

uint16_t inline convertDataToUInt16(const uint8_t *data) {
    return data[0] << 8 | data[1];
}