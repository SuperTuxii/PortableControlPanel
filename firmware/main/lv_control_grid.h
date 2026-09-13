#ifndef CONTROL_PANEL_FIRMWARE_LV_CONTROL_GRID_H
#define CONTROL_PANEL_FIRMWARE_LV_CONTROL_GRID_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#ifdef ESP_PLATFORM
#include "esp_err.h"
#else
#define esp_err_t   bool
#endif

#define cg_act      lv_control_grid_active()


typedef struct {
    lv_obj_t *gridContainer;
    lv_coord_t *rowDsc;
    lv_coord_t *columnDsc;
    uint8_t rowCount;
    uint8_t columnCount;
    int32_t outerPad;
    int32_t rowPad;
    int32_t columnPad;
    lv_image_dsc_t images[256];
} lv_control_grid_t;

typedef enum {
    LV_USER_DATA_TYPE_GRAD_DSC,
    LV_USER_DATA_TYPE_STYLE_DATA,
} lv_user_data_type_t;

typedef struct grad_dsc_data {
    lv_style_selector_t selector;
    lv_grad_dsc_t grad_dsc;
} lv_user_data_grad_dsc_t;

typedef struct style_data {
    lv_style_selector_t selector;
    lv_style_t style;
} lv_user_data_style_t;

typedef struct node {
    void *data;
    struct node *next;
    struct node *prev;
    lv_user_data_type_t type;
} lv_user_data_node_t;

typedef struct {
    struct node *head;
    struct node *tail;
} lv_user_data_t;

lv_control_grid_t *lv_control_grid_create(lv_obj_t *parent);
void lv_control_grid_delete(const lv_control_grid_t *control_grid);
lv_control_grid_t *lv_control_grid_active();
void lv_control_grid_set_layout(lv_control_grid_t *cg, int rows, int columns);
void lv_control_grid_set_outer_pad(lv_control_grid_t *cg, int32_t pad);
void lv_control_grid_set_row_pad(lv_control_grid_t *cg, int32_t pad);
void lv_control_grid_set_column_pad(lv_control_grid_t *cg, int32_t pad);
void lv_control_grid_test_fill(const lv_control_grid_t *cg);
void lv_control_grid_clear(const lv_control_grid_t *cg);
void lv_control_grid_move(const lv_control_grid_t *cg, uint8_t fromIndex, uint8_t toIndex);
void lv_control_grid_change_size(const lv_control_grid_t *cg, uint8_t index, uint8_t index2);
void lv_control_grid_remove(const lv_control_grid_t *cg, uint8_t index, uint8_t subIndex);
void lv_control_grid_add_image(lv_control_grid_t *cg, uint8_t index, uint8_t format, const uint8_t *data, const uint8_t *end);
void lv_control_grid_modify_image(const lv_control_grid_t *cg, uint8_t index, const uint8_t *data, const uint8_t *end);
void lv_control_grid_remove_image(lv_control_grid_t *cg, uint8_t index);
esp_err_t lv_control_grid_add_button(const lv_control_grid_t *cg, uint8_t index1, uint8_t index2);
uint8_t lv_control_grid_text(const lv_control_grid_t *cg, uint8_t index, uint8_t subIndex, const uint8_t *data, const uint8_t *end);
uint8_t lv_control_grid_image(const lv_control_grid_t *cg, uint8_t index, uint8_t subIndex, const uint8_t *data, const uint8_t *end);
uint8_t *lv_control_grid_set_style(const lv_control_grid_t *cg, uint8_t index, uint8_t subIndex, lv_style_t **style, lv_style_selector_t *styleSelector, uint8_t *data, uint8_t *end);
void lv_control_grid_remove_style(const lv_control_grid_t *cg, uint8_t index, uint8_t subIndex, lv_style_selector_t styleSelector);
void lv_control_grid_remove_styles(const lv_control_grid_t *cg, uint8_t index, uint8_t subIndex);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //CONTROL_PANEL_FIRMWARE_LV_CONTROL_GRID_H
