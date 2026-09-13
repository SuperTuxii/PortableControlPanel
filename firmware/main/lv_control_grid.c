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

#ifndef ESP_PLATFORM
#define ESP_LOGE(tag, format, ...)          fprintf(stderr, format, __VA_ARGS__);fprintf(stderr, "\n")
#define ESP_LOGW                            ESP_LOGE
#define ESP_ERR_NOT_ALLOWED                 false
#define ESP_OK                              true
#define heap_caps_malloc(size, caps)        malloc(size)
#define heap_caps_free                      free
#endif

#define BACKGROUND_COLOR                lv_color_hex(0x000000)

#ifdef ESP_PLATFORM
static const char *CONTROL_GRID_TAG = "lv_control_grid";
#endif

static lv_control_grid_t *active = nullptr;

static lv_obj_t *findCellChildByIndex(const lv_control_grid_t *cg, const uint8_t index) {
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
    return nullptr;
}

static void freeControlGridOnDelete(lv_event_t *event) {
    if (lv_event_get_code(event) != LV_EVENT_DELETE) return;
    lv_control_grid_t *control_grid = lv_event_get_user_data(event);
    free(control_grid->rowDsc);
    free(control_grid->columnDsc);
    free(control_grid);
}

static void set_grid_size(const lv_control_grid_t *cg) {
    const lv_obj_t *screen = lv_obj_get_parent(cg->gridContainer);
    const int cell_width = ((lv_obj_get_width(screen) - (2 * cg->outerPad) + cg->columnPad) / cg->columnCount) - cg->columnPad;
    const int cell_height = ((lv_obj_get_height(screen) - (2 * cg->outerPad) + cg->rowPad) / cg->rowCount) - cg->rowPad;
    const int cell_size = cell_height < cell_width ? cell_height : cell_width;
    lv_obj_set_size(cg->gridContainer, ((cell_size + cg->columnPad) * cg->columnCount) - cg->columnPad + (2 * cg->outerPad), ((cell_size + cg->rowPad) * cg->rowCount) - cg->rowPad + (2 * cg->outerPad));
}

lv_control_grid_t *lv_control_grid_create(lv_obj_t* parent) {
    active = malloc(sizeof(lv_control_grid_t));
    active->gridContainer = lv_obj_create(parent);
    active->rowDsc = nullptr;
    active->columnDsc = nullptr;
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

void lv_control_grid_delete(const lv_control_grid_t *control_grid) {
    if (control_grid->gridContainer) {
        lv_control_grid_clear(control_grid);
        lv_obj_delete(control_grid->gridContainer);
    }
}

lv_control_grid_t* lv_control_grid_active() {
    return active;
}

void lv_control_grid_set_layout(lv_control_grid_t *cg, const int rows, const int columns) {
    lv_control_grid_clear(cg);
    if (cg->rowDsc != nullptr)
        free(cg->rowDsc);
    if (cg->columnDsc != nullptr)
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
    lv_obj_set_style_bg_opa(cg->gridContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(cg->gridContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(cg->gridContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cg->gridContainer, cg->outerPad, LV_PART_MAIN);
    lv_obj_set_style_pad_row(cg->gridContainer, cg->rowPad, LV_PART_MAIN);
    lv_obj_set_style_pad_column(cg->gridContainer, cg->columnPad, LV_PART_MAIN);
}

void lv_control_grid_set_outer_pad(lv_control_grid_t *cg, const int32_t pad) {
    cg->outerPad = pad;
    set_grid_size(cg);
    lv_obj_set_style_pad_all(cg->gridContainer, cg->outerPad, LV_PART_MAIN);
}

void lv_control_grid_set_row_pad(lv_control_grid_t *cg, const int32_t pad) {
    cg->rowPad = pad;
    set_grid_size(cg);
    lv_obj_set_style_pad_row(cg->gridContainer, cg->rowPad, LV_PART_MAIN);
}

void lv_control_grid_set_column_pad(lv_control_grid_t *cg, const int32_t pad) {
    cg->columnPad = pad;
    set_grid_size(cg);
    lv_obj_set_style_pad_column(cg->gridContainer, cg->columnPad, LV_PART_MAIN);
}

void lv_control_grid_test_fill(const lv_control_grid_t *cg) {
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
                lv_obj_add_event_cb(label, alignPivotForScaleCB, LV_EVENT_SIZE_CHANGED, nullptr);
            } else {
                lv_obj_t* label = lv_label_create(obj);
                lv_label_set_text_fmt(label, "c%d, r%d", column, row);
                lv_obj_set_align(label, LV_ALIGN_BOTTOM_RIGHT);
                lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN);
            }
        }
    }
}

void lv_control_grid_clear(const lv_control_grid_t *cg) {
    while (lv_obj_get_child_count(cg->gridContainer) > 0) {
        lv_obj_delete(lv_obj_get_child(cg->gridContainer, 0));
    }
}

void lv_control_grid_move(const lv_control_grid_t* cg, const uint8_t fromIndex, const uint8_t toIndex) {
    lv_obj_t *object = findCellChildByIndex(cg, fromIndex);
    if (object == nullptr) return;
    const int32_t row = toIndex / cg->columnCount;
    const int32_t column = toIndex % cg->columnCount;
    lv_obj_set_style_grid_cell_row_pos(object, row, LV_PART_MAIN);
    lv_obj_set_style_grid_cell_column_pos(object, column, LV_PART_MAIN);
}

void lv_control_grid_change_size(const lv_control_grid_t* cg, const uint8_t index, const uint8_t index2) {
    lv_obj_t *object = findCellChildByIndex(cg, index);
    if (object == nullptr) return;
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

void lv_control_grid_remove(const lv_control_grid_t *cg, const uint8_t index, const uint8_t subIndex) {
    lv_obj_t *object = findCellChildByIndex(cg, index);
    if (object == nullptr) return;
    if (subIndex != 0)
        object = lv_obj_get_child(object, subIndex - 1);
    if (object == nullptr) return;
    lv_obj_delete(object);
}

void lv_control_grid_add_image(lv_control_grid_t *cg, const uint8_t index, const uint8_t format, const uint8_t* data, const uint8_t* end) {
    if (cg->images[index].data != nullptr)
        lv_control_grid_remove_image(cg, index);
    const uint16_t width = convertDataToUInt16(data);
    const uint16_t height = convertDataToUInt16(data + 2);
    const uint8_t bytesPerPixel = LV_COLOR_FORMAT_GET_SIZE(format);
    uint8_t *imageData = heap_caps_malloc(bytesPerPixel * width * height, MALLOC_CAP_SPIRAM);
    data += 4;
    if (imageData == nullptr) {
        ESP_LOGE(CONTROL_GRID_TAG, "Couldn't allocate memory for image with index %d (format: 0x%02X; width: %d; height: %d)", index, format, width, height);
        return;
    }
    const lv_image_dsc_t imageDsc = {
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

void lv_control_grid_modify_image(const lv_control_grid_t *cg, const uint8_t index, const uint8_t* data, const uint8_t* end) {
    if (cg->images[index].data == nullptr) {
        ESP_LOGE(CONTROL_GRID_TAG, "Image with index %d can't be modified, because it is not valid", index);
        return;
    }
    const uint32_t pixelIndex = convertDataToInt32(data);
    data += 4;
    const uint8_t bytesPerPixel = LV_COLOR_FORMAT_GET_SIZE(cg->images[index].header.cf);
    if (cg->images[index].data_size <= ((end - data) + (pixelIndex * bytesPerPixel))) {
        ESP_LOGE(CONTROL_GRID_TAG, "Image with index %d can't be modified, because the data is out of bounds", index);
        return;
    }
    memcpy((void *) cg->images[index].data + (pixelIndex * bytesPerPixel), data, (end - data) + 1);
}

void lv_control_grid_remove_image(lv_control_grid_t *cg, const uint8_t index) {
    if (cg->images[index].data == nullptr) {
        ESP_LOGW(CONTROL_GRID_TAG, "The image with index %d has already been removed", index);
        return;
    }
    heap_caps_free((void *) cg->images[index].data);
    cg->images[index].data_size = 0;
    cg->images[index].data = nullptr;
}

esp_err_t lv_control_grid_add_button(const lv_control_grid_t *cg, const uint8_t index1, const uint8_t index2) {
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
        const lv_obj_t *child = lv_obj_get_child(cg->gridContainer, i);
        const int32_t childRow = lv_obj_get_style_grid_cell_row_pos(child, 0);
        const int32_t childColumn = lv_obj_get_style_grid_cell_column_pos(child, 0);
        const int32_t childRow2 = childRow + lv_obj_get_style_grid_cell_row_span(child, 0);
        const int32_t childColumn2 = childColumn + lv_obj_get_style_grid_cell_column_span(child, 0);
        if (childColumn < (column + columnSpan) && childColumn2 > column &&
            childRow < (row + rowSpan) && childRow2 > row) {
            ESP_LOGE(CONTROL_GRID_TAG, "Tried to create button from cell %d to cell %d, but is overlapping with %d", index1, index2, (childRow * cg->columnCount) + childColumn);
            return ESP_ERR_NOT_ALLOWED;
        }
    }

    lv_obj_t* obj = lv_button_create(cg->gridContainer);
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, column, columnSpan,
                         LV_GRID_ALIGN_STRETCH, row, rowSpan);
    lv_obj_update_layout(obj);
    return ESP_OK;
}

uint8_t lv_control_grid_text(const lv_control_grid_t *cg, const uint8_t index, const uint8_t subIndex, const uint8_t* data, const uint8_t* end) {
    if (data > end) goto textFormatError;
    lv_obj_t *object = findCellChildByIndex(cg, index);
    if (object == nullptr) goto textFormatError;

    if (subIndex == 0) {
        lv_obj_t *label = lv_label_create(object);
        lv_obj_center(label);
        lv_label_set_text(label, (const char *) data);
        return lv_obj_get_child_count(object);
    }
    lv_obj_t *label = lv_obj_get_child(object, subIndex - 1);
    if (label == nullptr) goto textFormatError;
    lv_label_set_text(label, (const char *) data);
    return subIndex;
textFormatError:
    ESP_LOGE(CONTROL_GRID_TAG, "Couldn't set text for cell with index %d (sub: %d; dataLength: %ld)", index, subIndex, end - data + 1);
    return 0;
}

uint8_t lv_control_grid_image(const lv_control_grid_t *cg, const uint8_t index, const uint8_t subIndex, const uint8_t* data, const uint8_t* end) {
    if (data > end) goto imageFormatError;
    lv_obj_t *object = findCellChildByIndex(cg, index);
    if (object == nullptr) goto imageFormatError;

    if (subIndex == 0) {
        lv_obj_t *image = lv_image_create(object);
        lv_image_set_src(image, cg->images + *data);
        lv_image_set_inner_align(image, LV_IMAGE_ALIGN_STRETCH);
        return lv_obj_get_child_count(object);
    }
    lv_obj_t *image = lv_obj_get_child(object, subIndex - 1);
    if (image == nullptr) goto imageFormatError;
    lv_image_set_src(image, cg->images + *data);
    return subIndex;
imageFormatError:
    ESP_LOGE(CONTROL_GRID_TAG, "Couldn't set image for cell with index %d (sub: %d; dataLength: %ld)", index, subIndex, end - data + 1);
    return 0;
}

uint8_t *lv_control_grid_set_style(const lv_control_grid_t *cg, const uint8_t index, const uint8_t subIndex, lv_style_t **style, lv_style_selector_t *styleSelector, uint8_t *data, uint8_t *end) {
    if (data > end) goto styleFormatError;
    lv_obj_t *object = findCellChildByIndex(cg, index);
    if (object == nullptr) goto styleFormatError;
    if (subIndex != 0) {
        object = lv_obj_get_child(object, subIndex - 1);
        if (object == nullptr) goto styleFormatError;
    }
    if (*style == nullptr) *style = getStyleData(object, *styleSelector, true);
    data = handle_style_data(cg, object, style, styleSelector, data, end);
    lv_obj_refresh_style(object, lv_obj_style_get_selector_part(*styleSelector), LV_STYLE_PROP_ANY);
    if (data == end + 2) goto styleFormatError;
    return data;
styleFormatError:
    ESP_LOGE(CONTROL_GRID_TAG, "Couldn't set style for cell with index %d (sub: %d; key: 0x%02X; valueLength: %ld)", index, subIndex, data <= end ? *data : -1, end - data);
    return end + 2;
}

void lv_control_grid_remove_style(const lv_control_grid_t *cg, const uint8_t index, const uint8_t subIndex, const lv_style_selector_t styleSelector) {
    lv_obj_t *object = findCellChildByIndex(cg, index);
    if (object == nullptr) return;
    if (subIndex != 0) {
        object = lv_obj_get_child(object, subIndex - 1);
        if (object == nullptr) return;
    }
    removeStyle(object, styleSelector);
}

void lv_control_grid_remove_styles(const lv_control_grid_t *cg, const uint8_t index, const uint8_t subIndex) {
    lv_obj_t *object = findCellChildByIndex(cg, index);
    if (object == nullptr) return;
    if (subIndex != 0) {
        object = lv_obj_get_child(object, subIndex - 1);
        if (object == nullptr) return;
    }
    removeStyles(object);
}
