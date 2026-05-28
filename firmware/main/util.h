#ifndef CONTROLPANELFIRMWARE_UTIL_H
#define CONTROLPANELFIRMWARE_UTIL_H
#include <inttypes.h>

int32_t convertInt32ToLittleEndian(const uint8_t *data);
uint16_t convertUInt16ToLittleEndian(const uint8_t *data);

#endif //CONTROLPANELFIRMWARE_UTIL_H
