#include "util.h"

int32_t convertInt32ToLittleEndian(const uint8_t *data) {
    return data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
}

uint16_t convertUInt16ToLittleEndian(const uint8_t *data) {
    return data[0] << 8 | data[1];
}