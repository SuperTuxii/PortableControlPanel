#ifndef CONTROL_PANEL_FIRMWARE_LV_CONTROL_GRID_H
#define CONTROL_PANEL_FIRMWARE_LV_CONTROL_GRID_H
#include "lvgl.h"
#include "esp_err.h"

void lv_control_grid();
void lv_control_grid_set_layout(int rows, int columns);
void lv_control_grid_set_outer_pad(int32_t pad);
void lv_control_grid_set_row_pad(int32_t pad);
void lv_control_grid_set_column_pad(int32_t pad);
void lv_control_grid_test_fill();
void lv_control_grid_clear();
void lv_control_grid_remove(uint8_t index);
void lv_control_grid_add_image(uint8_t index, uint8_t format, uint8_t *data, uint8_t *end);
void lv_control_grid_modify_image(uint8_t index, uint8_t *data, uint8_t *end);
void lv_control_grid_remove_image(uint8_t index);
esp_err_t lv_control_grid_add_button(uint8_t index1, uint8_t index2);
uint8_t lv_control_grid_text(uint8_t index, uint8_t subIndex, uint8_t *data, uint8_t *end);
uint8_t lv_control_grid_image(uint8_t index, uint8_t subIndex, uint8_t *data, uint8_t *end);
uint8_t *lv_control_grid_set_style(uint8_t index, uint8_t subIndex, lv_style_selector_t *styleSelector, uint8_t *data, uint8_t *end);

#endif //CONTROL_PANEL_FIRMWARE_LV_CONTROL_GRID_H
