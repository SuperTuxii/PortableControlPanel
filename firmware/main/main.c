/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>

#include "sdkconfig.h"

#include "esp_log.h"
#include "tinyusb.h"
#include "tinyusb_default_config.h"
#include "tinyusb_cdc_acm.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "lv_control_grid.h"
#include "display_driver.h"
#include "protocol.h"
#include "tinyusb_console.h"

#define ALIGN_UP(num, align)    (((num) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_DOWN(num, align)  ((num) & ~((align) - 1))

static char *LVGL_TAG = "lvgl";

static esp_err_t lvgl_init() {
    ESP_LOGI(LVGL_TAG, "Initialize LVGL port");
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = LVGL_TASK_PRIORITY,
        .task_stack = LVGL_TASK_STACK_SIZE,
        .task_affinity = -1,
        .task_max_sleep_ms = LVGL_TASK_MAX_DELAY_MS,
        .task_stack_caps = MALLOC_CAP_SPIRAM,
        .timer_period_ms = LVGL_TICK_PERIOD_MS
    };
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    ESP_LOGI(LVGL_TAG, "Add MIPI DSI display");
    const lvgl_port_display_cfg_t display_cfg = {
        .io_handle = display_get_mipi_dpi_io(),
        .panel_handle = display_get_panel_handle(),
        .buffer_size = LCD_H_RES * LCD_V_RES,
        .double_buffer = true,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,
            .buff_spiram = true,
            .sw_rotate = true,
            .swap_bytes = false,
        }
    };
    const lvgl_port_display_dsi_cfg_t display_dsi_cfg = {
        //.flags.avoid_tearing = true   doesn't work for some reason. Me not know
    };
    lv_disp_t *display = lvgl_port_add_disp_dsi(&display_cfg, &display_dsi_cfg);
    lv_display_set_rotation(display, LV_DISPLAY_ROTATION_270);

    ESP_LOGI(LVGL_TAG, "Add touch");
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = display,
        .handle = touch_get_tp_handle(),
    };
    lv_indev_t* touch_handle = lvgl_port_add_touch(&touch_cfg);

    ESP_LOGI(LVGL_TAG, "Aquire lvgl port lock");
    if (lvgl_port_lock(0)) {
        ESP_LOGI(LVGL_TAG, "Add LVGL widgets");
        lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);
        lv_control_grid();
        lvgl_port_unlock();
    }
    return ESP_OK;
}

static void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event) {
    if (itf != TINYUSB_PROTOCOL_PORT)
        return;
    tinyusbReadReady();
}

static esp_err_t tinyusb_init() {
    const tinyusb_config_t tusb_cfg = TINYUSB_DEFAULT_CONFIG();

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    tinyusb_config_cdcacm_t acm_cfg = {
        .cdc_port = TINYUSB_PROTOCOL_PORT,
        .callback_rx = &tinyusb_cdc_rx_callback,
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = NULL,
        .callback_line_coding_changed = NULL
    };
    ESP_ERROR_CHECK(tinyusb_cdcacm_init(&acm_cfg));
    ESP_ERROR_CHECK(tinyusb_console_init(TINYUSB_CDC_ACM_0));
    return ESP_OK;
}

void app_main(void) {
    // Initialize display and LVGL
    ESP_ERROR_CHECK(display_brightness_init());
    ESP_ERROR_CHECK(display_backlight_on());
    ESP_ERROR_CHECK(display_init());
    ESP_ERROR_CHECK(touch_init());
    ESP_ERROR_CHECK(lvgl_init());
    // Initialize TinyUSB (route console output to high speed usb)
    ESP_ERROR_CHECK(tinyusb_init());

    // printf("Hello world!\n");
    //
    // /* Print chip information */
    // esp_chip_info_t chip_info;
    // uint32_t flash_size;
    // esp_chip_info(&chip_info);
    // printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
    //        CONFIG_IDF_TARGET,
    //        chip_info.cores,
    //        (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
    //        (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
    //        (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
    //        (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");
    //
    // unsigned major_rev = chip_info.revision / 100;
    // unsigned minor_rev = chip_info.revision % 100;
    // printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    // if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
    //     printf("Get flash size failed");
    //     return;
    // }
    //
    // printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
    //        (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    //
    // printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
    //
    // for (int i = 10; i >= 0; i--) {
    //     printf("Restarting in %d seconds...\n", i);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
    // printf("Restarting now.\n");
    // fflush(stdout);
    // display_backlight_off();
    // esp_restart();
}
