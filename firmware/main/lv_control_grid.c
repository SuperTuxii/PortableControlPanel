#include "lvgl.h"
#include "src/core/lv_obj_private.h"
#ifdef ESP_PLATFORM
#include "esp_log.h"
#include "esp_heap_caps.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif
#include "util.h"
#include "lv_control_grid.h"
#include "protocol.h"

#ifndef ESP_PLATFORM
#define ESP_LOGE(tag, format, ...)          fprintf(stderr, format, __VA_ARGS__);fprintf(stderr, "\n")
#define ESP_LOGW                            ESP_LOGE
#define ESP_ERR_NOT_ALLOWED                 false
#define ESP_OK                              true
#define heap_caps_malloc(size, caps)        malloc(size)
#define heap_caps_free                      free
#endif

#define BACKGROUND_COLOR                lv_color_hex(0x000000)

static const char *CONTROL_GRID_TAG = "lv_control_grid";

lv_control_grid_t *active = NULL;

static lv_obj_t *findCellChildByIndex(lv_control_grid_t *cg, const uint8_t index) {
    const uint8_t row = index / cg->columnCount;
    const uint8_t column = index % cg->columnCount;
    const uint32_t childCount = lv_obj_get_child_count(cg->gridContainer);
    for (int i = 0; i < childCount; ++i) {
        lv_obj_t *child = lv_obj_get_child(cg->gridContainer, i);
        const int32_t rowSpan = row - lv_obj_get_style_grid_cell_row_pos(child, 0);
        const int32_t columnSpan = column - lv_obj_get_style_grid_cell_column_pos(child, 0);
        if (rowSpan >= 0 && rowSpan < lv_obj_get_style_grid_cell_row_span(child, 0) &&
            columnSpan >= 0 && columnSpan < lv_obj_get_style_grid_cell_column_span(child, 0)) {
            return child;
        }
    }
    return NULL;
}

static void alignPivotForScaleForced(lv_obj_t *object, lv_style_selector_t styleSelector) {
    lv_align_t align = lv_obj_get_style_align(object, styleSelector);
    if (align == LV_ALIGN_TOP_LEFT || align == LV_ALIGN_LEFT_MID || align == LV_ALIGN_BOTTOM_LEFT)
        lv_obj_set_style_transform_pivot_x(object, 0, styleSelector);
    if (align == LV_ALIGN_TOP_MID || align == LV_ALIGN_CENTER || align == LV_ALIGN_BOTTOM_MID)
        lv_obj_set_style_transform_pivot_x(object, lv_obj_get_width(object) / 2, styleSelector);
    if (align == LV_ALIGN_TOP_RIGHT || align == LV_ALIGN_RIGHT_MID || align == LV_ALIGN_BOTTOM_RIGHT)
        lv_obj_set_style_transform_pivot_x(object, lv_obj_get_width(object), styleSelector);
    if (align == LV_ALIGN_TOP_LEFT || align == LV_ALIGN_TOP_MID || align == LV_ALIGN_TOP_RIGHT)
        lv_obj_set_style_transform_pivot_y(object, 0, styleSelector);
    if (align == LV_ALIGN_LEFT_MID || align == LV_ALIGN_CENTER || align == LV_ALIGN_RIGHT_MID)
        lv_obj_set_style_transform_pivot_y(object, lv_obj_get_height(object) / 2, styleSelector);
    if (align == LV_ALIGN_BOTTOM_LEFT || align == LV_ALIGN_BOTTOM_MID || align == LV_ALIGN_BOTTOM_RIGHT)
        lv_obj_set_style_transform_pivot_y(object, lv_obj_get_height(object), styleSelector);
}

static void alignPivotForScale(lv_obj_t *object, lv_style_selector_t styleSelector) {
    lv_align_t align = lv_obj_get_style_align(object, styleSelector);
    if (lv_obj_get_style_transform_scale_x(object, styleSelector) != LV_SCALE_NONE) {
        if (align == LV_ALIGN_TOP_LEFT || align == LV_ALIGN_LEFT_MID || align == LV_ALIGN_BOTTOM_LEFT)
            lv_obj_set_style_transform_pivot_x(object, 0, styleSelector);
        if (align == LV_ALIGN_TOP_MID || align == LV_ALIGN_CENTER || align == LV_ALIGN_BOTTOM_MID)
            lv_obj_set_style_transform_pivot_x(object, lv_obj_get_width(object) / 2, styleSelector);
        if (align == LV_ALIGN_TOP_RIGHT || align == LV_ALIGN_RIGHT_MID || align == LV_ALIGN_BOTTOM_RIGHT)
            lv_obj_set_style_transform_pivot_x(object, lv_obj_get_width(object), styleSelector);
    }
    if (lv_obj_get_style_transform_scale_y(object, styleSelector) != LV_SCALE_NONE) {
        if (align == LV_ALIGN_TOP_LEFT || align == LV_ALIGN_TOP_MID || align == LV_ALIGN_TOP_RIGHT)
            lv_obj_set_style_transform_pivot_y(object, 0, styleSelector);
        if (align == LV_ALIGN_LEFT_MID || align == LV_ALIGN_CENTER || align == LV_ALIGN_RIGHT_MID)
            lv_obj_set_style_transform_pivot_y(object, lv_obj_get_height(object) / 2, styleSelector);
        if (align == LV_ALIGN_BOTTOM_LEFT || align == LV_ALIGN_BOTTOM_MID || align == LV_ALIGN_BOTTOM_RIGHT)
            lv_obj_set_style_transform_pivot_y(object, lv_obj_get_height(object), styleSelector);
    }
}

static void alignPivotForScaleCB(lv_event_t *event) {
    if (lv_event_get_code(event) != LV_EVENT_SIZE_CHANGED)
        return;
    lv_obj_t *object = lv_event_get_target(event);
    alignPivotForScale(object, 0);
    for (uint32_t selector = 1; selector <= 0x0FFFFF; selector <<= 1) {
        alignPivotForScale(object, selector);
    }
}

static void alignPivotForScaleForcedCB(lv_event_t *event) {
    if (lv_event_get_code(event) != LV_EVENT_SIZE_CHANGED)
        return;
    lv_obj_t *object = lv_event_get_target(event);
    alignPivotForScaleForced(object, 0);
    for (uint32_t selector = 1; selector <= 0x0FFFFF; selector <<= 1) {
        alignPivotForScaleForced(object, selector);
    }
}

static void freeUserDataOnDelete(lv_event_t *event) {
    lv_user_data_t *list = lv_obj_get_user_data(lv_event_get_target(event));
    if (lv_event_get_code(event) != LV_EVENT_DELETE || list == NULL)
        return;
    lv_user_data_node_t *node = list->head;
    do {
        lv_user_data_node_t* next = node->next;
        free(node->data);
        free(node);
        node = next;
    } while (node != NULL);
    free(list);
}

static void addUserData(lv_obj_t *object, lv_user_data_type_t type, void *data) {
    lv_user_data_t *list = lv_obj_get_user_data(object);
    if (list == NULL) {
        list = malloc(sizeof(lv_user_data_t));
        lv_obj_set_user_data(object, list);
        lv_obj_add_event_cb(object, freeUserDataOnDelete, LV_EVENT_DELETE, NULL);
        list->head = NULL;
        list->tail = NULL;
    }
    lv_user_data_node_t *node = malloc(sizeof(lv_user_data_node_t));
    node->data = data;
    node->type = type;
    node->next = NULL;
    node->prev = list->tail;
    if (list->tail != NULL)
        list->tail->next = node;
    else
        list->head = node;
    list->tail = node;
}

static lv_grad_dsc_t *getBgGradDsc(lv_obj_t *object, lv_style_selector_t styleSelector, bool createIfNotFound) {
    lv_grad_dsc_t *gradDsc = NULL;
    const lv_user_data_t *userData = lv_obj_get_user_data(object);
    if (userData != NULL) {
        lv_user_data_node_t *node = userData->head;
        while (node != NULL) {
            if (node->type == LV_USER_DATA_TYPE_GRAD_DSC) {
                lv_user_data_grad_dsc_t *gradDscData = node->data;
                if (styleSelector == gradDscData->selector)
                    return &gradDscData->grad_dsc;
                if (createIfNotFound && gradDsc == NULL && ((styleSelector & gradDscData->selector) != 0 ||
                    ((gradDscData->selector & 0xFFFF) == 0 && (styleSelector & 0xFF0000) == (gradDscData->selector & 0xFF0000))))
                    gradDsc = &gradDscData->grad_dsc;
            }
            node = node->next;
        }
    }
    if (!createIfNotFound)
        return NULL;
    lv_user_data_grad_dsc_t *gradDscData = malloc(sizeof(lv_user_data_grad_dsc_t));
    gradDscData->selector = styleSelector;
    if (gradDsc != NULL) {
        memcpy(&gradDscData->grad_dsc, gradDsc, sizeof(lv_grad_dsc_t));
        gradDsc = &gradDscData->grad_dsc;
    } else {
        gradDsc = &gradDscData->grad_dsc;
        memset(gradDsc, 0, sizeof(lv_grad_dsc_t));
        gradDsc->stops[0].color = lv_obj_get_style_bg_color(object, styleSelector);
        gradDsc->stops[0].opa = lv_obj_get_style_bg_main_opa(object, styleSelector);
        gradDsc->stops[0].frac = lv_obj_get_style_bg_main_stop(object, styleSelector);
        gradDsc->stops[1].color = lv_obj_get_style_bg_grad_color(object, styleSelector);
        gradDsc->stops[1].opa = lv_obj_get_style_bg_grad_opa(object, styleSelector);
        gradDsc->stops[1].frac = lv_obj_get_style_bg_grad_stop(object, styleSelector);
        gradDsc->stops_count = LV_GRADIENT_MAX_STOPS;
        gradDsc->dir = lv_obj_get_style_bg_grad_dir(object, styleSelector);
        gradDsc->params.radial.end_extent.x = INT32_MIN;
    }
    addUserData(object, LV_USER_DATA_TYPE_GRAD_DSC, gradDscData);
    lv_obj_set_style_bg_grad(object, gradDsc, styleSelector);
    return gradDsc;
}

static void freeControlGridOnDelete(lv_event_t *event) {
    if (lv_event_get_code(event) != LV_EVENT_DELETE)
        return;
    lv_control_grid_t *control_grid = lv_event_get_user_data(event);
    free(control_grid->rowDsc);
    free(control_grid->columnDsc);
    free(control_grid);
}

static void set_grid_size(lv_control_grid_t *cg) {
    lv_obj_t *screen = lv_obj_get_parent(cg->gridContainer);
    const int cell_width = ((lv_obj_get_width(screen) - (2 * cg->outerPad) + cg->columnPad) / cg->columnCount) - cg->columnPad;
    const int cell_height = ((lv_obj_get_height(screen) - (2 * cg->outerPad) + cg->rowPad) / cg->rowCount) - cg->rowPad;
    const int cell_size = cell_height < cell_width ? cell_height : cell_width;
    lv_obj_set_size(cg->gridContainer, ((cell_size + cg->columnPad) * cg->columnCount) - cg->columnPad + (2 * cg->outerPad), ((cell_size + cg->rowPad) * cg->rowCount) - cg->rowPad + (2 * cg->outerPad));
}

lv_control_grid_t *lv_control_grid_create(lv_obj_t* parent) {
    active = malloc(sizeof(lv_control_grid_t));
    active->gridContainer = lv_obj_create(parent);
    active->rowDsc = NULL;
    active->columnDsc = NULL;
    active->rowCount = 0;
    active->columnCount = 0;
    active->outerPad = 5;
    active->rowPad = 5;
    active->columnPad = 5;
    memset(active->images, 0, sizeof(active->images));
    lv_obj_null_on_delete(&active->gridContainer);
    lv_obj_add_event_cb(active->gridContainer, freeControlGridOnDelete, LV_EVENT_DELETE, active);
    lv_obj_set_scrollbar_mode(active->gridContainer, LV_SCROLLBAR_MODE_OFF);
    lv_control_grid_set_layout(active, 3, 5);
    lv_control_grid_test_fill(active);
    return active;
}

void lv_control_grid_delete(lv_control_grid_t *control_grid) {
    if (control_grid->gridContainer) {
        lv_control_grid_clear(control_grid);
        lv_obj_delete(control_grid->gridContainer);
    }
}

lv_control_grid_t* lv_control_grid_active() {
    return active;
}

void lv_control_grid_set_layout(lv_control_grid_t *cg, int rows, int columns) {
    lv_control_grid_clear(cg);
    if (cg->rowDsc != NULL)
        free(cg->rowDsc);
    if (cg->columnDsc != NULL)
        free(cg->columnDsc);
    cg->rowDsc = malloc((rows + 1) * sizeof(lv_coord_t));
    cg->columnDsc = malloc((columns + 1) * sizeof(lv_coord_t));
    cg->rowCount = rows;
    cg->columnCount = columns;

    for (int i = 0; i < rows; ++i) {
        cg->rowDsc[i] = LV_GRID_FR(1);
    }
    cg->rowDsc[rows] = LV_GRID_TEMPLATE_LAST;
    for (int i = 0; i < columns; ++i) {
        cg->columnDsc[i] = LV_GRID_FR(1);
    }
    cg->columnDsc[columns] = LV_GRID_TEMPLATE_LAST;
    lv_obj_set_grid_dsc_array(cg->gridContainer, cg->columnDsc, cg->rowDsc);

    set_grid_size(cg);
    lv_obj_center(cg->gridContainer);
    lv_obj_set_style_bg_color(cg->gridContainer, BACKGROUND_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(cg->gridContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(cg->gridContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cg->gridContainer, cg->outerPad, LV_PART_MAIN);
    lv_obj_set_style_pad_row(cg->gridContainer, cg->rowPad, LV_PART_MAIN);
    lv_obj_set_style_pad_column(cg->gridContainer, cg->columnPad, LV_PART_MAIN);
}

void lv_control_grid_set_outer_pad(lv_control_grid_t *cg, int32_t pad) {
    cg->outerPad = pad;
    set_grid_size(cg);
    lv_obj_set_style_pad_all(cg->gridContainer, cg->outerPad, LV_PART_MAIN);
}

void lv_control_grid_set_row_pad(lv_control_grid_t *cg, int32_t pad) {
    cg->rowPad = pad;
    set_grid_size(cg);
    lv_obj_set_style_pad_row(cg->gridContainer, cg->rowPad, LV_PART_MAIN);
}

void lv_control_grid_set_column_pad(lv_control_grid_t *cg, int32_t pad) {
    cg->columnPad = pad;
    set_grid_size(cg);
    lv_obj_set_style_pad_column(cg->gridContainer, cg->columnPad, LV_PART_MAIN);
}

void lv_control_grid_test_fill(lv_control_grid_t *cg) {
    for (int row = 0; row < cg->rowCount; ++row) {
        for (int column = 0; column < cg->columnCount; ++column) {
            lv_obj_t* obj = lv_button_create(cg->gridContainer);
            lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, column, 1,
                                 LV_GRID_ALIGN_STRETCH, row, 1);

            if (((row * cg->columnCount) + column) % 2 == 0) {
                lv_obj_t* label = lv_label_create(obj);
                lv_label_set_text_fmt(label, "c%d, r%d", column, row);
                lv_obj_set_align(label, LV_ALIGN_BOTTOM_RIGHT);
                lv_obj_set_style_text_font(label, &lv_font_montserrat_48, LV_PART_MAIN);
                lv_obj_set_style_transform_scale(label, 75, LV_PART_MAIN);
                lv_obj_add_event_cb(label, alignPivotForScaleCB, LV_EVENT_SIZE_CHANGED, NULL);
            } else {
                lv_obj_t* label = lv_label_create(obj);
                lv_label_set_text_fmt(label, "c%d, r%d", column, row);
                lv_obj_set_align(label, LV_ALIGN_BOTTOM_RIGHT);
                lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN);
            }
        }
    }
}

void lv_control_grid_clear(lv_control_grid_t *cg) {
    while (lv_obj_get_child_count(cg->gridContainer) > 0) {
        lv_obj_delete(lv_obj_get_child(cg->gridContainer, 0));
    }
}

void lv_control_grid_move(lv_control_grid_t* cg, uint8_t fromIndex, uint8_t toIndex) {
    lv_obj_t *object = findCellChildByIndex(cg, fromIndex);
    if (object == NULL)
        return;
    int32_t row = toIndex / cg->columnCount;
    int32_t column = toIndex % cg->columnCount;
    lv_obj_set_style_grid_cell_row_pos(object, row, LV_PART_MAIN);
    lv_obj_set_style_grid_cell_column_pos(object, column, LV_PART_MAIN);
}

void lv_control_grid_change_size(lv_control_grid_t* cg, uint8_t index, uint8_t index2) {
    lv_obj_t *object = findCellChildByIndex(cg, index);
    if (object == NULL)
        return;
    int32_t row = index / cg->columnCount;
    int32_t column = index % cg->columnCount;
    int32_t rowSpan = (index2 / cg->columnCount) - row;
    int32_t columnSpan = (index2 % cg->columnCount) - column;
    if (rowSpan < 0) {
        row += rowSpan;
        rowSpan = -rowSpan;
    }
    if (columnSpan < 0) {
        column += columnSpan;
        columnSpan = -columnSpan;
    }
    rowSpan++;
    columnSpan++;
    lv_obj_set_style_grid_cell_row_pos(object, row, LV_PART_MAIN);
    lv_obj_set_style_grid_cell_column_pos(object, column, LV_PART_MAIN);
    lv_obj_set_style_grid_cell_row_span(object, rowSpan, LV_PART_MAIN);
    lv_obj_set_style_grid_cell_column_span(object, columnSpan, LV_PART_MAIN);
}

void lv_control_grid_remove(lv_control_grid_t *cg, uint8_t index, uint8_t subIndex) {
    lv_obj_t *object = findCellChildByIndex(cg, index);
    if (object == NULL)
        return;
    if (subIndex != 0)
        object = lv_obj_get_child(object, subIndex - 1);
    if (object == NULL)
        return;
    lv_obj_delete(object);
}

void lv_control_grid_add_image(lv_control_grid_t *cg, uint8_t index, uint8_t format, uint8_t* data, uint8_t* end) {
    if (cg->images[index].data != NULL)
        lv_control_grid_remove_image(cg, index);
    uint16_t width = convertDataToUInt16(data);
    uint16_t height = convertDataToUInt16(data + 2);
    uint8_t bytesPerPixel = LV_COLOR_FORMAT_GET_SIZE(format);
    uint8_t *imageData = heap_caps_malloc(bytesPerPixel * width * height, MALLOC_CAP_SPIRAM);
    data += 4;
    if (imageData == NULL) {
        ESP_LOGE(CONTROL_GRID_TAG, "Couldn't allocate memory for image with index %d (format: 0x%02X; width: %d; height: %d)", index, format, width, height);
        return;
    }
    lv_image_dsc_t imageDsc = {
        .header = {
            .magic = LV_IMAGE_HEADER_MAGIC,
            .cf = format,
            .flags = 0,
            .w = width,
            .h = height,
            .stride = width * bytesPerPixel,
            .reserved_2 = 0
        },
        .data_size = width * height * bytesPerPixel,
        .data = imageData
    };
    cg->images[index] = imageDsc;
    memcpy(imageData, data, (end - data) + 1);
}

void lv_control_grid_modify_image(lv_control_grid_t *cg, uint8_t index, uint8_t* data, uint8_t* end) {
    if (cg->images[index].data == NULL) {
        ESP_LOGE(CONTROL_GRID_TAG, "Image with index %d can't be modified, because it is not valid", index);
        return;
    }
    uint32_t pixelIndex = convertDataToInt32(data);
    data += 4;
    uint8_t bytesPerPixel = LV_COLOR_FORMAT_GET_SIZE(cg->images[index].header.cf);
    memcpy((void *) cg->images[index].data + (pixelIndex * bytesPerPixel), data, (end - data) + 1);
}

void lv_control_grid_remove_image(lv_control_grid_t *cg, uint8_t index) {
    if (cg->images[index].data == NULL) {
        ESP_LOGW(CONTROL_GRID_TAG, "The image with index %d has already been removed", index);
        return;
    }
    heap_caps_free((void *) cg->images[index].data);
    cg->images[index].data_size = 0;
    cg->images[index].data = NULL;
}

esp_err_t lv_control_grid_add_button(lv_control_grid_t *cg, uint8_t index1, uint8_t index2) {
    int32_t row = index1 / cg->columnCount;
    int32_t column = index1 % cg->columnCount;
    int32_t rowSpan = (index2 / cg->columnCount) - row;
    int32_t columnSpan = (index2 % cg->columnCount) - column;
    if (rowSpan < 0) {
        row += rowSpan;
        rowSpan = -rowSpan;
    }
    if (columnSpan < 0) {
        column += columnSpan;
        columnSpan = -columnSpan;
    }
    rowSpan++;
    columnSpan++;

    const uint32_t childCount = lv_obj_get_child_count(cg->gridContainer);
    for (int i = 0; i < childCount; ++i) {
        lv_obj_t *child = lv_obj_get_child(cg->gridContainer, i);
        int32_t childRow = lv_obj_get_style_grid_cell_row_pos(child, 0);
        int32_t childColumn = lv_obj_get_style_grid_cell_column_pos(child, 0);
        int32_t childRow2 = childRow + lv_obj_get_style_grid_cell_row_span(child, 0);
        int32_t childColumn2 = childColumn + lv_obj_get_style_grid_cell_column_span(child, 0);
        if (childColumn < (column + columnSpan) && childColumn2 > column &&
            childRow < (row + rowSpan) && childRow2 > row) {
            ESP_LOGE(CONTROL_GRID_TAG, "Tried to create button from cell %d to cell %d, but is overlapping with %d", index1, index2, (childRow * cg->columnCount) + childColumn);
            return ESP_ERR_NOT_ALLOWED;
        }
    }

    lv_obj_t* obj = lv_button_create(cg->gridContainer);
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, column, columnSpan,
                         LV_GRID_ALIGN_STRETCH, row, rowSpan);
    return ESP_OK;
}

uint8_t lv_control_grid_text(lv_control_grid_t *cg, const uint8_t index, const uint8_t subIndex, uint8_t* data, uint8_t* end) {
    if (data > end)
        goto textFormatError;
    lv_obj_t *object = findCellChildByIndex(cg, index);
    if (object == NULL)
        goto textFormatError;

    if (subIndex == 0) {
        lv_obj_t *label = lv_label_create(object);
        lv_obj_center(label);
        lv_label_set_text(label, (const char *) data);
        return lv_obj_get_child_count(object);
    } else {
        lv_obj_t *label = lv_obj_get_child(object, subIndex - 1);
        if (label == NULL)
            goto textFormatError;
        lv_label_set_text(label, (const char *) data);
        return subIndex;
    }
textFormatError:
    ESP_LOGE(CONTROL_GRID_TAG, "Couldn't set text for cell with index %d (sub: %d; dataLength: %ld)", index, subIndex, end - data + 1);
    return 0;
}

uint8_t lv_control_grid_image(lv_control_grid_t *cg, uint8_t index, uint8_t subIndex, uint8_t* data, uint8_t* end) {
    if (data > end)
        goto imageFormatError;
    lv_obj_t *object = findCellChildByIndex(cg, index);
    if (object == NULL)
        goto imageFormatError;

    if (subIndex == 0) {
        lv_obj_t *image = lv_image_create(object);
        lv_image_set_src(image, cg->images + *data);
        lv_obj_set_size(image, lv_obj_get_width(object), lv_obj_get_height(object));
        lv_image_set_inner_align(image, LV_IMAGE_ALIGN_STRETCH);
        return lv_obj_get_child_count(object);
    } else {
        lv_obj_t *image = lv_obj_get_child(object, subIndex - 1);
        if (image == NULL)
            goto imageFormatError;
        lv_image_set_src(image, cg->images + *data);
        return subIndex;
    }
imageFormatError:
    ESP_LOGE(CONTROL_GRID_TAG, "Couldn't set image for cell with index %d (sub: %d; dataLength: %ld)", index, subIndex, end - data + 1);
    return 0;
}

uint8_t *lv_control_grid_set_style(lv_control_grid_t *cg, const uint8_t index, const uint8_t subIndex, lv_style_selector_t *styleSelector, uint8_t *data, uint8_t *end) {
    if (data > end)
        goto styleFormatError;
    lv_obj_t *object = findCellChildByIndex(cg, index);
    if (object == NULL)
        goto styleFormatError;
    if (subIndex != 0) {
        object = lv_obj_get_child(object, subIndex - 1);
        if (object == NULL)
            goto styleFormatError;
    }

    lv_grad_dsc_t *gradDsc = NULL;
    bool considerScale = (*data == ConsiderScale);
    if (considerScale)
        data++;
    int styleDataSize = end - data;
    if (*data <= NumberStyleKeyMax) { // Number Keys
        if (styleDataSize < 4)
            goto styleFormatError;
        int32_t styleData = convertDataToInt32(data + 1);
        if (considerScale) {
            if (*data == PadTop || *data == PadBottom || *data == MarginTop || *data == MarginBottom ||
                *data == ShadowOffsetY || *data == TextLineSpace || *data == DropShadowOffsetY || (*data >= Height && *data <= MaxHeight)) {
                styleData = (styleData << 8) / lv_obj_get_style_transform_scale_y(object, *styleSelector);
            } else {
                styleData = (styleData << 8) / lv_obj_get_style_transform_scale_x(object, *styleSelector);
            }
        }

        switch (*data) {
        case SetStyleSelector:
            *styleSelector = styleData;
            break;
        case PadAll:
            lv_obj_set_style_pad_all(object, styleData, *styleSelector);
            break;
        case PadTop:
            lv_obj_set_style_pad_top(object, styleData, *styleSelector);
            break;
        case PadBottom:
            lv_obj_set_style_pad_bottom(object, styleData, *styleSelector);
            break;
        case PadLeft:
            lv_obj_set_style_pad_left(object, styleData, *styleSelector);
            break;
        case PadRight:
            lv_obj_set_style_pad_right(object, styleData, *styleSelector);
            break;
        case MarginAll:
            lv_obj_set_style_margin_all(object, styleData, *styleSelector);
            break;
        case MarginTop:
            lv_obj_set_style_margin_top(object, styleData, *styleSelector);
            break;
        case MarginBottom:
            lv_obj_set_style_margin_bottom(object, styleData, *styleSelector);
            break;
        case MarginLeft:
            lv_obj_set_style_margin_left(object, styleData, *styleSelector);
            break;
        case MarginRight:
            lv_obj_set_style_margin_right(object, styleData, *styleSelector);
            break;
        case BorderWidth:
            lv_obj_set_style_border_width(object, styleData, *styleSelector);
            break;
        case OutlineWidth:
            lv_obj_set_style_outline_width(object, styleData, *styleSelector);
            break;
        case OutlinePad:
            lv_obj_set_style_outline_pad(object, styleData, *styleSelector);
            break;
        case ShadowWidth:
            lv_obj_set_style_shadow_width(object, styleData, *styleSelector);
            break;
        case ShadowOffsetX:
            lv_obj_set_style_shadow_offset_x(object, styleData, *styleSelector);
            break;
        case ShadowOffsetY:
            lv_obj_set_style_shadow_offset_y(object, styleData, *styleSelector);
            break;
        case ShadowSpread:
            lv_obj_set_style_shadow_spread(object, styleData, *styleSelector);
            break;
        case TextLetterSpace:
            lv_obj_set_style_text_letter_space(object, styleData, *styleSelector);
            break;
        case TextLineSpace:
            lv_obj_set_style_text_line_space(object, styleData, *styleSelector);
            break;
        case TextOutlineStrokeWidth:
            lv_obj_set_style_text_outline_stroke_width(object, styleData, *styleSelector);
            break;
        case BlurRadius:
            lv_obj_set_style_blur_radius(object, styleData, *styleSelector);
            break;
        case DropShadowRadius:
            lv_obj_set_style_drop_shadow_radius(object, styleData, *styleSelector);
            break;
        case DropShadowOffsetX:
            lv_obj_set_style_drop_shadow_offset_x(object, styleData, *styleSelector);
            break;
        case DropShadowOffsetY:
            lv_obj_set_style_drop_shadow_offset_y(object, styleData, *styleSelector);
            break;
        case Width:
            lv_obj_set_style_width(object, styleData, *styleSelector);
            break;
        case MinWidth:
            lv_obj_set_style_min_width(object, styleData, *styleSelector);
            break;
        case MaxWidth:
            lv_obj_set_style_max_width(object, styleData, *styleSelector);
            break;
        case Height:
            lv_obj_set_style_height(object, styleData, *styleSelector);
            break;
        case MinHeight:
            lv_obj_set_style_min_height(object, styleData, *styleSelector);
            break;
        case MaxHeight:
            lv_obj_set_style_max_height(object, styleData, *styleSelector);
            break;
        case Length:
            lv_obj_set_style_length(object, styleData, *styleSelector);
            break;
        case X:
            lv_obj_set_style_x(object, styleData, *styleSelector);
            break;
        case Y:
            lv_obj_set_style_y(object, styleData, *styleSelector);
            break;
        case TransformWidth:
            lv_obj_set_style_transform_width(object, styleData, *styleSelector);
            break;
        case TransformHeight:
            lv_obj_set_style_transform_height(object, styleData, *styleSelector);
            break;
        case TranslateX:
            lv_obj_set_style_translate_x(object, styleData, *styleSelector);
            break;
        case TranslateY:
            lv_obj_set_style_translate_y(object, styleData, *styleSelector);
            break;
        case TranslateRadial:
            lv_obj_set_style_translate_radial(object, styleData, *styleSelector);
            break;
        case TranslateScale:
            lv_obj_set_style_transform_scale(object, styleData, *styleSelector);
            lv_obj_remove_event_cb(object, alignPivotForScaleCB);
            lv_obj_add_event_cb(object, alignPivotForScaleCB, LV_EVENT_SIZE_CHANGED, NULL);
            alignPivotForScale(object, *styleSelector);
            break;
        case TranslateScaleX:
            lv_obj_set_style_transform_scale_x(object, styleData, *styleSelector);
            lv_obj_remove_event_cb(object, alignPivotForScaleCB);
            lv_obj_add_event_cb(object, alignPivotForScaleCB, LV_EVENT_SIZE_CHANGED, NULL);
            alignPivotForScale(object, *styleSelector);
            break;
        case TranslateScaleY:
            lv_obj_set_style_transform_scale_y(object, styleData, *styleSelector);
            lv_obj_remove_event_cb(object, alignPivotForScaleCB);
            lv_obj_add_event_cb(object, alignPivotForScaleCB, LV_EVENT_SIZE_CHANGED, NULL);
            alignPivotForScale(object, *styleSelector);
            break;
        case TransformRotation:
            lv_obj_set_style_transform_rotation(object, styleData, *styleSelector);
            break;
        case TransformPivotX:
            lv_obj_set_style_transform_pivot_x(object, styleData, *styleSelector);
            break;
        case TransformPivotY:
            lv_obj_set_style_transform_pivot_y(object, styleData, *styleSelector);
            break;
        case TransformSkewX:
            lv_obj_set_style_transform_skew_x(object, styleData, *styleSelector);
            break;
        case TransformSkewY:
            lv_obj_set_style_transform_skew_y(object, styleData, *styleSelector);
            break;
        case Radius:
            lv_obj_set_style_radius(object, styleData, *styleSelector);
            break;
        case RadiusOffset:
            lv_obj_set_style_radial_offset(object, styleData, *styleSelector);
            break;
        case RotarySensitivity:
            lv_obj_set_style_rotary_sensitivity(object, styleData, *styleSelector);
            break;
        case BackgroundGradParams1:
            gradDsc = getBgGradDsc(object, *styleSelector, true);
            if (gradDsc->params.radial.focal_extent.x >= gradDsc->params.radial.focal.x)
                gradDsc->params.radial.focal_extent.x = styleData + (gradDsc->params.radial.focal_extent.x - gradDsc->params.radial.focal.x);
            gradDsc->params.linear.start.x = styleData;
            break;
        case BackgroundGradParams2:
            gradDsc = getBgGradDsc(object, *styleSelector, true);
            if (gradDsc->params.radial.focal_extent.x >= gradDsc->params.radial.focal.x)
                gradDsc->params.radial.focal_extent.y = styleData;
            gradDsc->params.linear.start.y = styleData;
            break;
        case BackgroundGradLinearEndX:
            getBgGradDsc(object, *styleSelector, true)->params.linear.end.x = styleData;
            break;
        case BackgroundGradLinearEndY:
            getBgGradDsc(object, *styleSelector, true)->params.linear.end.y = styleData;
            break;
        case BackgroundGradRadialEndX:
            gradDsc = getBgGradDsc(object, *styleSelector, true);
            if (gradDsc->params.radial.end_extent.x > gradDsc->params.radial.end.x)
                gradDsc->params.radial.end_extent.x = styleData + (gradDsc->params.radial.end_extent.x - gradDsc->params.radial.end.x);
            gradDsc->params.radial.end.x = styleData;
            break;
        case BackgroundGradRadialEndY:
            gradDsc = getBgGradDsc(object, *styleSelector, true);
            if (gradDsc->params.radial.end_extent.x > gradDsc->params.radial.end.x)
                gradDsc->params.radial.end_extent.y = styleData;
            gradDsc->params.radial.end.y = styleData;
            break;
        case BackgroundGradRadialFocalRadius:
            gradDsc = getBgGradDsc(object, *styleSelector, true);
            styleData = abs(styleData);
            if (styleData == abs(gradDsc->params.radial.end_extent.x - gradDsc->params.radial.end.x)
                && gradDsc->params.radial.focal.x == gradDsc->params.radial.end.x
                && gradDsc->params.radial.focal.y == gradDsc->params.radial.end.y)
                styleData++;
            gradDsc->params.radial.focal_extent.x = gradDsc->params.radial.focal.x + styleData;
            gradDsc->params.radial.focal_extent.y = gradDsc->params.radial.focal.y;
            break;
        case BackgroundGradRadialEndRadius:
            gradDsc = getBgGradDsc(object, *styleSelector, true);
            styleData = abs(styleData);
            if (styleData == 0 || (styleData == abs(gradDsc->params.radial.focal_extent.x - gradDsc->params.radial.focal.x)
                && gradDsc->params.radial.focal.x == gradDsc->params.radial.end.x
                && gradDsc->params.radial.focal.y == gradDsc->params.radial.end.y))
                styleData++;
            gradDsc->params.radial.end_extent.x = gradDsc->params.radial.end.x + styleData;
            gradDsc->params.radial.end_extent.y = gradDsc->params.radial.end.y;
            break;
        default: ;
        }
        return data + 5;
    }
    if (*data <= ColorOpacityStyleKeyMax) { // Color + Opacity Keys
        if (styleDataSize < 4)
            goto styleFormatError;
        uint32_t rawData = (uint32_t) convertDataToInt32(data + 1);
        lv_color_t color = lv_color_hex(rawData >> 8);
        lv_opa_t opa = rawData & 0xFF;
        switch (*data) {
        case BackgroundColor:
            lv_obj_set_style_bg_color(object, color, *styleSelector);
            lv_obj_set_style_bg_opa(object, opa, *styleSelector);
            gradDsc = getBgGradDsc(object, *styleSelector, false);
            if (gradDsc != NULL)
                gradDsc->stops[0].color = color;
            break;
        case BackgroundGradColor:
            lv_obj_set_style_bg_grad_color(object, color, *styleSelector);
            lv_obj_set_style_bg_grad_opa(object, opa, *styleSelector);
            gradDsc = getBgGradDsc(object, *styleSelector, false);
            if (gradDsc != NULL) {
                gradDsc->stops[1].color = color;
                gradDsc->stops[1].opa = opa;
            }
            break;
        case BackgroundImageRecolor:
            lv_obj_set_style_bg_image_recolor(object, color, *styleSelector);
            lv_obj_set_style_bg_image_recolor_opa(object, opa, *styleSelector);
            break;
        case BorderColor:
            lv_obj_set_style_border_color(object, color, *styleSelector);
            lv_obj_set_style_border_opa(object, opa, *styleSelector);
            break;
        case OutlineColor:
            lv_obj_set_style_outline_color(object, color, *styleSelector);
            lv_obj_set_style_outline_opa(object, opa, *styleSelector);
            break;
        case ShadowColor:
            lv_obj_set_style_shadow_color(object, color, *styleSelector);
            lv_obj_set_style_shadow_opa(object, opa, *styleSelector);
            break;
        case TextColor:
            lv_obj_set_style_text_color(object, color, *styleSelector);
            lv_obj_set_style_text_opa(object, opa, *styleSelector);
            break;
        case TextOutlineStrokeColor:
            lv_obj_set_style_text_outline_stroke_color(object, color, *styleSelector);
            lv_obj_set_style_text_outline_stroke_opa(object, opa, *styleSelector);
            break;
        case DropShadowColor:
            lv_obj_set_style_drop_shadow_color(object, color, *styleSelector);
            lv_obj_set_style_drop_shadow_opa(object, opa, *styleSelector);
            break;
        case Recolor:
            lv_obj_set_style_recolor(object, color, *styleSelector);
            lv_obj_set_style_recolor_opa(object, opa, *styleSelector);
            break;
        case ImageRecolor:
            lv_obj_set_style_image_recolor(object, color, *styleSelector);
            lv_obj_set_style_image_recolor_opa(object, opa, *styleSelector);
            break;
        default: ;
        }
        return data + 5;
    }
    if (*data <= Number16StyleKeyMax) { // Number (16) Keys
        if (styleDataSize < 2)
            goto styleFormatError;
        int16_t value = (int16_t) convertDataToUInt16(data + 1);
        switch (*data) {
        case BackgroundGradConicalStartAngle:
            getBgGradDsc(object, *styleSelector, true)->params.conical.start_angle = value;
            break;
        case BackgroundGradConicalEndAngle:
            getBgGradDsc(object, *styleSelector, true)->params.conical.end_angle = value;
            break;
        default: ;
        }
        return data + 3;
    }
    if (*data <= ByteStyleKeyMax) { // Byte Keys
        if (styleDataSize < 1)
            goto styleFormatError;
        uint8_t value = *(data + 1);
        switch (*data) {
        case BackgroundImageIndex:
            lv_obj_set_style_bg_image_src(object, cg->images + value, *styleSelector);
            break;
        case BackgroundMainOpacity:
            lv_obj_set_style_bg_main_opa(object, value, *styleSelector);
            gradDsc = getBgGradDsc(object, *styleSelector, false);
            if (gradDsc != NULL)
                gradDsc->stops[0].opa = value;
            break;
        case BackgroundMainStop:
            lv_obj_set_style_bg_main_stop(object, value, *styleSelector);
            gradDsc = getBgGradDsc(object, *styleSelector, false);
            if (gradDsc != NULL)
                gradDsc->stops[0].frac = value;
            break;
        case BackgroundGradStop:
            lv_obj_set_style_bg_grad_stop(object, value, *styleSelector);
            gradDsc = getBgGradDsc(object, *styleSelector, false);
            if (gradDsc != NULL)
                gradDsc->stops[1].frac = value;
            break;
        case BackgroundImageOpacity:
            lv_obj_set_style_bg_image_opa(object, value, *styleSelector);
            break;
        case FontSizeScaled:
            lv_obj_set_style_text_font(object, &lv_font_montserrat_48, *styleSelector);
            lv_obj_set_style_transform_scale(object, (value << 8) / 48, *styleSelector | LV_PART_ANY);
            lv_obj_remove_event_cb(object, alignPivotForScaleCB);
            lv_obj_add_event_cb(object, alignPivotForScaleCB, LV_EVENT_SIZE_CHANGED, NULL);
            alignPivotForScale(object, *styleSelector);
            break;
        case Opacity:
            lv_obj_set_style_opa(object, value, *styleSelector);
            break;
        case OpacityLayered:
            lv_obj_set_style_opa_layered(object, value, *styleSelector);
            break;
        case ColorFilterOpacity:
            lv_obj_set_style_color_filter_opa(object, value, *styleSelector);
            break;
        case ImageOpacity:
            lv_obj_set_style_image_opa(object, value, *styleSelector);
            break;
        default: ;
        }
        return data + 2;
    }
    // Non Type Keys
    if (*data >= BackgroundGradDirNone && *data <= BackgroundGradDirConical) {
        lv_obj_set_style_bg_grad_dir(object, *data - BackgroundGradDirNone, *styleSelector);
        if (*data >= BackgroundGradDirLinear) gradDsc = getBgGradDsc(object, *styleSelector, true);
        if (gradDsc != NULL)
            gradDsc->dir = *data - BackgroundGradDirNone;
        goto nonTypeSwitchEnd;
    }
    if (*data >= BackgroundGradExtendPad && *data <= BackgroundGradExtendReflect) {
        getBgGradDsc(object, *styleSelector, true)->extend = *data - BackgroundGradExtendPad;
        goto nonTypeSwitchEnd;
    }
    if (*data >= TextAlignAuto && *data <= TextAlignRight) {
        lv_obj_set_style_text_align(object, *data - LV_TEXT_ALIGN_AUTO, *styleSelector);
        goto nonTypeSwitchEnd;
    }
    if (*data == BlurBackdropOff || *data == BlurBackdropOn) {
        lv_obj_set_style_blur_backdrop(object, *data - BlurBackdropOff, *styleSelector);
        goto nonTypeSwitchEnd;
    }
    if (*data >= BlurQualityAuto && *data <= BlurQualityPrecision) {
        lv_obj_set_style_blur_quality(object, *data - BlurQualityAuto, *styleSelector);
        goto nonTypeSwitchEnd;
    }
    if (*data >= DropShadowQualityAuto && *data <= DropShadowQualityPrecision) {
        lv_obj_set_style_drop_shadow_quality(object, *data - DropShadowQualityAuto, *styleSelector);
        goto nonTypeSwitchEnd;
    }
    if (*data >= AlignTopLeft && *data <= AlignRightMid) {
        lv_obj_set_style_align(object, *data - AlignTopLeft + 1, *styleSelector);
        alignPivotForScale(object, *styleSelector);
        goto nonTypeSwitchEnd;
    }
    if (*data == BackgroundImageTiledOff || *data == BackgroundImageTiledOn) {
        lv_obj_set_style_bg_image_tiled(object, *data - BackgroundImageTiledOff, *styleSelector);
        goto nonTypeSwitchEnd;
    }
    if (*data == ClipCornerOff || *data == ClipCornerOn) {
        lv_obj_set_style_clip_corner(object, *data - ClipCornerOff, *styleSelector);
        goto nonTypeSwitchEnd;
    }
    if (*data >= BlendModeNormal && *data <= BlendModeDifference) {
        lv_obj_set_style_blend_mode(object, *data - BlendModeNormal, *styleSelector);
        goto nonTypeSwitchEnd;
    }
    if (*data >= BaseDirLTR && *data <= BaseDirAuto) {
        lv_obj_set_style_base_dir(object, *data - BaseDirLTR, *styleSelector);
        goto nonTypeSwitchEnd;
    }
    switch (*data) {
    case BorderSideNone:
        lv_obj_set_style_border_side(object, LV_BORDER_SIDE_NONE, *styleSelector);
        break;
    case BorderSideBottom:
        lv_obj_set_style_border_side(object, lv_obj_get_style_border_side(object, *styleSelector) | LV_BORDER_SIDE_BOTTOM, *styleSelector);
        break;
    case BorderSideTop:
        lv_obj_set_style_border_side(object, lv_obj_get_style_border_side(object, *styleSelector) | LV_BORDER_SIDE_TOP, *styleSelector);
        break;
    case BorderSideLeft:
        lv_obj_set_style_border_side(object, lv_obj_get_style_border_side(object, *styleSelector) | LV_BORDER_SIDE_LEFT, *styleSelector);
        break;
    case BorderSideRight:
        lv_obj_set_style_border_side(object, lv_obj_get_style_border_side(object, *styleSelector) | LV_BORDER_SIDE_RIGHT, *styleSelector);
        break;
    case BorderSideFull:
        lv_obj_set_style_border_side(object, LV_BORDER_SIDE_FULL, *styleSelector);
        break;
    case TextDecorNone:
        lv_obj_set_style_text_decor(object, LV_TEXT_DECOR_NONE, *styleSelector);
        break;
    case TextDecorUnderline:
        lv_obj_set_style_text_decor(object, lv_obj_get_style_text_decor(object, *styleSelector) | LV_TEXT_DECOR_UNDERLINE, *styleSelector);
        break;
    case TextDecorStrikethrough:
        lv_obj_set_style_text_decor(object, lv_obj_get_style_text_decor(object, *styleSelector) | LV_TEXT_DECOR_STRIKETHROUGH, *styleSelector);
        break;
    case AlignTransformPivot:
        lv_obj_update_layout(object);
        alignPivotForScaleForced(object, *styleSelector);
        break;
    case AlignTransformPivotAll:
        lv_obj_update_layout(object);
        alignPivotForScaleForced(object, 0);
        for (uint32_t selector = 1; selector <= 0x0FFFFF; selector <<= 1) {
            alignPivotForScaleForced(object, selector);
        }
        break;
    case AlignTransformPivotEvent:
        lv_obj_remove_event_cb(object, alignPivotForScaleForcedCB);
        lv_obj_add_event_cb(object, alignPivotForScaleForcedCB, LV_EVENT_SIZE_CHANGED, NULL);
        break;
    case AlignTransformPivotAllEvent:
        lv_obj_remove_event_cb(object, alignPivotForScaleForcedCB);
        break;
    case FontMontserrat14:
        lv_obj_set_style_text_font(object, &lv_font_montserrat_14, *styleSelector);
        break;
    case FontMontserrat48:
        lv_obj_set_style_text_font(object, &lv_font_montserrat_48, *styleSelector);
        break;
    case ColorFilterUnset:
        lv_obj_set_style_color_filter_dsc(object, NULL, *styleSelector);
        break;
    case ColorFilterShade:
        lv_obj_set_style_color_filter_dsc(object, &lv_color_filter_shade, *styleSelector);
        break;
    default: ;
    }
nonTypeSwitchEnd:
    return data + 1;
styleFormatError:
    ESP_LOGE(CONTROL_GRID_TAG, "Couldn't set style for cell with index %d (sub: %d; key: 0x%02X; valueLength: %ld)", index, subIndex, data <= end ? *data : -1, end - data);
    return end + 1;
}
