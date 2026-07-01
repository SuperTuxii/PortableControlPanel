#ifndef CONTROL_PANEL_FIRMWARE_PROTOCOL_H
#define CONTROL_PANEL_FIRMWARE_PROTOCOL_H
#include <inttypes.h>
#include "protocol_macros.h"

#define TINYUSB_PROTOCOL_PORT       TINYUSB_CDC_ACM_0

PROTOCOL_COMMANDS_ENUM
PROTOCOL_STYLE_KEYS_ENUM

void tinyusbReadReady();
void handleStyleData(uint8_t index, uint8_t subIndex, uint8_t* data, uint8_t dataLength);
void handleCommand(uint8_t cmd, uint8_t operand1, uint8_t operand2, uint8_t *data, uint8_t dataLength);

#endif //CONTROL_PANEL_FIRMWARE_PROTOCOL_H
