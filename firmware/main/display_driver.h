#ifndef CONTROL_PANEL_FIRMWARE_DISPLAY_DRIVER_H
#define CONTROL_PANEL_FIRMWARE_DISPLAY_DRIVER_H
#include "esp_lcd_st7701.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_err.h"

#define CONFIG_USE_DMA2D_COPY_FRAME     1

#define MIPI_DSI_PHY_PWR_LDO_CHAN       3
#define MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV 2500
#define MIPI_DSI_LANE_NUM               2
#define MIPI_DSI_LANE_BITRATE_MBPS      500
#define MIPI_DSI_DPI_CLK_MHZ            34
#define LCD_H_RES                       480
#define LCD_V_RES                       800
#define LCD_HSYNC                       12
#define LCD_HBP                         42
#define LCD_HFP                         42
#define LCD_VSYNC                       2
#define LCD_VBP                         8
#define LCD_VFP                         166

#define TOUCH_I2C_NUM                   I2C_NUM_1
#define TOUCH_I2C_SDA                   GPIO_NUM_7
#define TOUCH_I2C_SCL                   GPIO_NUM_8
#define TOUCH_RST                       GPIO_NUM_NC
#define TOUCH_INT                       GPIO_NUM_NC

#define LCD_BIT_PER_PIXEL               16
#define LCD_RST                         GPIO_NUM_5
#define LCD_BACKLIGHT                   GPIO_NUM_23
#define LCD_LEDC_CH                     LEDC_CHANNEL_0

#define ST7701_480_800_PANEL_DPI_CONFIG(color_format)  \
    {                                                       \
        .virtual_channel = 0,                               \
        .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,        \
        .dpi_clock_freq_mhz = MIPI_DSI_DPI_CLK_MHZ,         \
        .in_color_format = color_format,                    \
        .num_fbs = 1,                                       \
        .video_timing = {                                   \
            .h_size = LCD_H_RES,                            \
            .v_size = LCD_V_RES,                            \
            .hsync_pulse_width = LCD_HSYNC,                 \
            .hsync_back_porch = LCD_HFP,                    \
            .hsync_front_porch = LCD_HFP,                   \
            .vsync_pulse_width = LCD_VSYNC,                 \
            .vsync_back_porch = LCD_VBP,                    \
            .vsync_front_porch = LCD_VFP,                   \
        },                                                  \
    }

#define LVGL_DRAW_BUF_LINES             LCD_V_RES
#define LVGL_TICK_PERIOD_MS             2
#define LVGL_TASK_STACK_SIZE            (6 * 1024)
#define LVGL_TASK_PRIORITY              2
#define LVGL_TASK_MAX_DELAY_MS          500
#define LVGL_TASK_MIN_DELAY_MS          (1000 / CONFIG_FREERTOS_HZ)

static const st7701_lcd_init_cmd_t vendor_specific_init[] = {
    {0xFF, (uint8_t []){0x77,0x01,0x00,0x00,0x13},5,0},
    {0xEF, (uint8_t []){0x08}, 1, 0},
    {0xFF, (uint8_t []){0x77,0x01,0x00,0x00,0x10},5,0},
    {0xC0, (uint8_t []){0x63, 0x00}, 2, 0},
    {0xC1, (uint8_t []){0x0D, 0x02}, 2, 0},
    {0xC2, (uint8_t []){0x10, 0x08}, 2, 0},
    {0xCC, (uint8_t []){0x10}, 1, 0},

    {0xB0, (uint8_t []){0x80, 0x09, 0x53, 0x0C, 0xD0, 0x07, 0x0C, 0x09, 0x09, 0x28, 0x06, 0xD4, 0x13, 0x69, 0x2B, 0x71}, 16, 0},
    {0xB1, (uint8_t []){0x80, 0x94, 0x5A, 0x10, 0xD3, 0x06, 0x0A, 0x08, 0x08, 0x25, 0x03, 0xD3, 0x12, 0x66, 0x6A, 0x0D}, 16, 0},
    {0xFF, (uint8_t []){0x77, 0x01, 0x00, 0x00, 0x11}, 5, 0},

    {0xB0, (uint8_t []){0x5D}, 1, 0},
    {0xB1, (uint8_t []){0x58}, 1, 0},
    {0xB2, (uint8_t []){0x87}, 1, 0},
    {0xB3, (uint8_t []){0x80}, 1, 0},
    {0xB5, (uint8_t []){0x4E}, 1, 0},
    {0xB7, (uint8_t []){0x85}, 1, 0},
    {0xB8, (uint8_t []){0x21}, 1, 0},
    {0xB9, (uint8_t []){0x10, 0x1F}, 2, 0},
    {0xBB, (uint8_t []){0x03}, 1,0},
    {0xBC, (uint8_t []){0x00}, 1,0},

    {0xC1, (uint8_t []){0x78}, 1, 0},
    {0xC2, (uint8_t []){0x78}, 1, 0},
    {0xD0, (uint8_t []){0x88}, 1, 0},

    {0xE0, (uint8_t []){0x00, 0x3A, 0x02}, 3, 0},
    {0xE1, (uint8_t []){0x04, 0xA0, 0x00, 0xA0, 0x05,0xA0, 0x00, 0xA0, 0x00, 0x40, 0x40}, 11, 0},
    {0xE2, (uint8_t []){0x30, 0x00, 0x40, 0x40, 0x32, 0xA0, 0x00, 0xA0, 0x00, 0xA0, 0x00, 0xA0, 0x00}, 13, 0},
    {0xE3, (uint8_t []){0x00, 0x00, 0x33, 0x33}, 4, 0},
    {0xE4, (uint8_t []){0x44, 0x44}, 2, 0},
    {0xE5, (uint8_t []){0x09, 0x2E, 0xA0, 0xA0, 0x0B, 0x30, 0xA0, 0xA0, 0x05, 0x2A, 0xA0, 0xA0, 0x07, 0x2C, 0xA0, 0xA0}, 16, 0},
    {0xE6, (uint8_t []){0x00, 0x00, 0x33, 0x33}, 4, 0},
    {0xE7, (uint8_t []){0x44, 0x44}, 2, 0},
    {0xE8, (uint8_t []){0x08, 0x2D, 0xA0, 0xA0, 0x0A, 0x2F, 0xA0, 0xA0, 0x04, 0x29, 0xA0, 0xA0, 0x06, 0x2B, 0xA0, 0xA0}, 16, 0},

    {0xEB, (uint8_t []){0x00, 0x00, 0x4E, 0x4E, 0x00, 0x00, 0x00}, 7, 0},
    {0xEC, (uint8_t []){0x08, 0x01}, 2, 0},

    {0xED, (uint8_t []){0xB0, 0x2B, 0x98, 0xA4, 0x56, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0x65, 0x4A, 0x89, 0xB2, 0x0B}, 16, 0},
    {0xEF, (uint8_t []){0x08, 0x08, 0x08, 0x45, 0x3F, 0x54}, 6, 0},
    {0xFF, (uint8_t []){0x77, 0x01, 0x00, 0x00, 0x00}, 5, 0},

    // {0x3A, (uint8_t []){0x66}, 1, 0},
    {0x11, (uint8_t []){0x00}, 1, 120},
    {0x29, (uint8_t []){0x00}, 1, 20},
};

esp_err_t display_brightness_init();
esp_err_t display_init();
esp_err_t touch_init();

esp_err_t display_brightness_set(int brightness);
esp_err_t display_backlight_off();
esp_err_t display_backlight_on();

esp_lcd_panel_io_handle_t display_get_mipi_dpi_io();
esp_lcd_panel_handle_t display_get_panel_handle();
esp_lcd_touch_handle_t touch_get_tp_handle();

#endif //CONTROL_PANEL_FIRMWARE_DISPLAY_DRIVER_H
