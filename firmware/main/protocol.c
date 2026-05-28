#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "tinyusb_cdc_acm.h"
#include "class/cdc/cdc_device.h"
#include "esp_lvgl_port.h"
#include "display_driver.h"
#include "lv_control_grid.h"
#include "util.h"
#include "protocol.h"

#define DEBUG_READ      0
#define ECHO_COMMAND    0

static const char *COMMS_TAG = "comm_protocol";

static size_t tuReadBytes = 0;
static uint8_t tuCmd;
static uint8_t tuOperands[] = {0, 0};
static uint8_t tuDataLength = 0;
static uint8_t tuData[256];
void tinyusbReadReady() {
    size_t rx_size = 0;
    esp_err_t ret;
    if (tuReadBytes != 0) {
        size_t alreadyReadData = tuReadBytes - 1;
#if DEBUG_READ
        ESP_LOGI(COMMS_TAG, "DEBUG: Continuing read for current message");
#endif
        if ((tuCmd & (1 << 6)) > 1) { // Read operands
            if (alreadyReadData < 2) {
                ret = tinyusb_cdcacm_read(TINYUSB_PROTOCOL_PORT, tuOperands + alreadyReadData, 2 - alreadyReadData, &rx_size);
                if (ret != ESP_OK)
                    goto tinyusb_read_error;
                tuReadBytes += rx_size;
                if (rx_size != 2 - alreadyReadData) {
                    return;
                }
#if DEBUG_READ
                ESP_LOGI(COMMS_TAG, "DEBUG: Operands: %02X %02X", tuOperands[0], tuOperands[1]);
#endif
            } else {
                alreadyReadData -= 2;
            }
        }
        if ((tuCmd & (1 << 7)) > 1) { // Read data
            if (alreadyReadData == 0) {
                ret = tinyusb_cdcacm_read(TINYUSB_PROTOCOL_PORT, &tuDataLength, 1, &rx_size);
                if (ret != ESP_OK)
                    goto tinyusb_read_error;
                if (rx_size != 1)
                    return;
                tuReadBytes++;
#if DEBUG_READ
                ESP_LOGI(COMMS_TAG, "DEBUG: Data Length: %d", tuDataLength + 1);
#endif
            } else {
                alreadyReadData--;
            }
            ret = tinyusb_cdcacm_read(TINYUSB_PROTOCOL_PORT, tuData + alreadyReadData, tuDataLength + 1 - alreadyReadData, &rx_size);
#if DEBUG_READ
            ESP_LOGI(COMMS_TAG, "DEBUG: Tried to read Data (%d): %d/%d Bytes read", ret, rx_size, tuDataLength + 1 - alreadyReadData);
#endif
            if (ret != ESP_OK)
                goto tinyusb_read_error;
            tuReadBytes += rx_size;
            if (rx_size != tuDataLength + 1 - alreadyReadData)
                return;
        }
        handleCommand(tuCmd, tuOperands[0], tuOperands[1], tuData, tuDataLength);
        tuReadBytes = 0;
    }
    while (tud_cdc_n_available(TINYUSB_PROTOCOL_PORT)) {
#if DEBUG_READ
        ESP_LOGI(COMMS_TAG, "DEBUG: Starting read for new message");
#endif
        ret = tinyusb_cdcacm_read(TINYUSB_PROTOCOL_PORT, &tuCmd, 1, &rx_size);
        if (ret != ESP_OK)
            goto tinyusb_read_error;
        if (rx_size != 1)
            return;
        tuReadBytes++;
#if DEBUG_READ
        ESP_LOGI(COMMS_TAG, "DEBUG: Command: %02X", tuCmd);
#endif
        if ((tuCmd & (1 << 6)) > 1) { // Read operands
            ret = tinyusb_cdcacm_read(TINYUSB_PROTOCOL_PORT, tuOperands, 2, &rx_size);
            if (ret != ESP_OK)
                goto tinyusb_read_error;
            tuReadBytes += rx_size;
            if (rx_size != 2) {
                return;
            }
#if DEBUG_READ
            ESP_LOGI(COMMS_TAG, "DEBUG: Operands: %02X %02X", tuOperands[0], tuOperands[1]);
#endif
        }
        if ((tuCmd & (1 << 7)) > 1) { // Read data
            ret = tinyusb_cdcacm_read(TINYUSB_PROTOCOL_PORT, &tuDataLength, 1, &rx_size);
            if (ret != ESP_OK)
                goto tinyusb_read_error;
            if (rx_size != 1)
                return;
            tuReadBytes++;
#if DEBUG_READ
            ESP_LOGI(COMMS_TAG, "DEBUG: Data Length: %d", tuDataLength + 1);
#endif
            ret = tinyusb_cdcacm_read(TINYUSB_PROTOCOL_PORT, tuData, tuDataLength + 1, &rx_size);
#if DEBUG_READ
            ESP_LOGI(COMMS_TAG, "DEBUG: Tried to read Data (%d): %d/%d Bytes read", ret, rx_size, tuDataLength + 1);
#endif
            if (ret != ESP_OK)
                goto tinyusb_read_error;
            tuReadBytes += rx_size;
            if (rx_size != tuDataLength + 1)
                return;
        }
        handleCommand(tuCmd, tuOperands[0], tuOperands[1], tuData, tuDataLength);
        tuReadBytes = 0;
    }
    return;
tinyusb_read_error:
    ESP_LOGE(COMMS_TAG, "TinyUSB Read Error");
    if (tud_cdc_n_available(TINYUSB_PROTOCOL_PORT) > 0) {
        tud_cdc_n_read_flush(TINYUSB_PROTOCOL_PORT);
    }
}

void handleStyleData(uint8_t index, uint8_t subIndex, uint8_t* data, uint8_t dataLength) {
    lv_style_selector_t styleSelector = 0;
    uint8_t *end = data + dataLength;
    while (data <= end) {
        data = lv_control_grid_set_style(index, subIndex, &styleSelector, data, end);
    }
}

void handleCommand(uint8_t cmd, uint8_t operand1, uint8_t operand2, uint8_t* data, uint8_t dataLength) {
#if ECHO_COMMAND
    ESP_LOGI(COMMS_TAG, "Received Command %02X with Operands %02X & %02X and %d Bytes of additional Data", cmd, operand1, operand2, (cmd & (1 << 7)) > 1 ? dataLength + 1 : 0);
    if ((cmd & (1 << 7)) > 1) {
        printf("\tAdditional Data: ");
        for (int i = 0; i <= dataLength; ++i) {
            printf("%02X ", data[i]);
        }
        printf("\n");
    }
#endif
    if ((cmd >> 6) == 0) { // No operands or data
        switch (cmd) {
        case 0x00:
            fflush(stdout);
            display_backlight_off();
            esp_restart();
        case 0x01:
            fflush(stdout);
            display_backlight_off();
            esp_deep_sleep_start();
        case 0x02:
            if (lvgl_port_lock(0)) {
                lv_control_grid_test_fill();
                lvgl_port_unlock();
            }
            break;
        case 0x03:
            if (lvgl_port_lock(0)) {
                lv_control_grid_clear();
                lvgl_port_unlock();
            }
            break;
        default: goto undefinedCommandError;
        }
    } else if ((cmd >> 6) == 1) { // Only operands
        switch (cmd) {
        case 0x40:
            if (lvgl_port_lock(0)) {
                lv_control_grid_set_layout((operand1 + 1) / (operand2 + 1), operand2 + 1);
                lvgl_port_unlock();
            }
            break;
        case 0x41:
            if (lvgl_port_lock(0)) {
                lv_control_grid_remove(operand1);
                lvgl_port_unlock();
            }
            break;
        case 0x42:
            if (lvgl_port_lock(0)) {
                lv_control_grid_remove_image(operand1);
                lvgl_port_unlock();
            }
            break;
        default: goto undefinedCommandError;
        }
    } else if ((cmd >> 6) == 2) { // Only data
        switch (cmd) {
        case 0x80:
            if (dataLength == 3 && lvgl_port_lock(0)) {
                lv_control_grid_set_outer_pad(convertInt32ToLittleEndian(data));
                lvgl_port_unlock();
            }
            break;
        case 0x81:
            if (dataLength == 3 && lvgl_port_lock(0)) {
                lv_control_grid_set_row_pad(convertInt32ToLittleEndian(data));
                lvgl_port_unlock();
            }
            break;
        case 0x82:
            if (dataLength == 3 && lvgl_port_lock(0)) {
                lv_control_grid_set_column_pad(convertInt32ToLittleEndian(data));
                lvgl_port_unlock();
            }
            break;
        default: goto undefinedCommandError;
        }
    } else { // Operands and data
        switch (cmd) {
        case 0xC0:
            if (lvgl_port_lock(0)) {
                handleStyleData(operand1, operand2, data, dataLength);
                lvgl_port_unlock();
            }
            break;
        case 0xC1:
            if (lvgl_port_lock(0)) {
                lv_control_grid_add_image(operand1, operand2, data, data + dataLength);
                lvgl_port_unlock();
            }
            break;
        case 0xC2:
            if (lvgl_port_lock(0)) {
                lv_control_grid_modify_image(operand1, data, data + dataLength);
                lvgl_port_unlock();
            }
            break;
        case 0xC3:
            if (lvgl_port_lock(0)) {
                if (lv_control_grid_add_button(operand1, operand2) == ESP_OK)
                    handleStyleData(operand1, 0, data, dataLength);
                lvgl_port_unlock();
            }
            break;
        case 0xC4:
            if (lvgl_port_lock(0)) {
                uint8_t *end = data + dataLength;
                uint8_t *styleData = data;
                while (*styleData != 0 && styleData < end)
                    styleData++;
                if (*styleData != 0) {
                    ESP_LOGE(COMMS_TAG, "Couldn't find Zero Terminator of text");
                    return;
                }
                styleData++;
                operand2 = lv_control_grid_text(operand1, operand2, data, end);
                if (operand2 != 0) {
                    handleStyleData(operand1, operand2, styleData, dataLength - (styleData - data));
                }
                lvgl_port_unlock();
            }
            break;
        case 0xC5:
            if (lvgl_port_lock(0)) {
                operand2 = lv_control_grid_image(operand1, operand2, data, data + dataLength);
                if (operand2 != 0 && dataLength != 0)
                    handleStyleData(operand1, operand2, data + 1, dataLength - 1);
                lvgl_port_unlock();
            }
            break;
        default: goto undefinedCommandError;
        }
    }
    return;
undefinedCommandError:
    ESP_LOGE(COMMS_TAG, "Undefined Command: %02X", cmd);
}
