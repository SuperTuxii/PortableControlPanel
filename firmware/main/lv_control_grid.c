#include "lvgl.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "util.h"
#include "lv_control_grid.h"

#define BACKGROUND_COLOR    lv_color_hex(0x000000)

static const char *CONTROL_GRID_TAG = "lv_control_grid";

static lv_obj_t *gridContainer = NULL;
static lv_coord_t *rowDsc = NULL;
static lv_coord_t *columnDsc = NULL;
static uint8_t rowCount = 0;
static uint8_t columnCount = 0;
static int32_t outerPad = 5;
static int32_t rowPad = 5;
static int32_t columnPad = 5;
static lv_image_dsc_t images[256] = { 0 };

uint8_t lastIndex;
lv_obj_t *lastObject = NULL;
static lv_obj_t *findCellChildByIndex(const uint8_t index) {
    if (lastObject != NULL && lastIndex == index)
        return lastObject;
    const uint8_t row = index / columnCount;
    const uint8_t column = index % columnCount;
    const uint32_t childCount = lv_obj_get_child_count(gridContainer);
    for (int i = 0; i < childCount; ++i) {
        lv_obj_t *child = lv_obj_get_child(gridContainer, i);
        const int32_t rowSpan = row - lv_obj_get_style_grid_cell_row_pos(child, 0);
        const int32_t columnSpan = column - lv_obj_get_style_grid_cell_column_pos(child, 0);
        if (rowSpan >= 0 && rowSpan < lv_obj_get_style_grid_cell_row_span(child, 0) &&
            columnSpan >= 0 && columnSpan < lv_obj_get_style_grid_cell_column_span(child, 0)) {
            lastIndex = index;
            lastObject = child;
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

static void set_grid_size() {
    lv_obj_t *screen = lv_screen_active();
    const int cell_width = ((lv_obj_get_width(screen) - (2 * outerPad) + columnPad) / columnCount) - columnPad;
    const int cell_height = ((lv_obj_get_height(screen) - (2 * outerPad) + rowPad) / rowCount) - rowPad;
    const int cell_size = cell_height < cell_width ? cell_height : cell_width;
    lv_obj_set_size(gridContainer, ((cell_size + columnPad) * columnCount) - columnPad + (2 * outerPad), ((cell_size + rowPad) * rowCount) - rowPad + (2 * outerPad));
}

void lv_control_grid() {
    lv_obj_t *screen = lv_screen_active();

    gridContainer = lv_obj_create(screen);
    lv_control_grid_set_layout(3, 5);
    lv_control_grid_test_fill();
}

void lv_control_grid_set_layout(int rows, int columns) {
    lv_control_grid_clear();
    if (rowDsc != NULL)
        free(rowDsc);
    if (columnDsc != NULL)
        free(columnDsc);
    rowDsc = malloc((rows + 1) * sizeof(lv_coord_t));
    columnDsc = malloc((columns + 1) * sizeof(lv_coord_t));
    rowCount = rows;
    columnCount = columns;

    for (int i = 0; i < rows; ++i) {
        rowDsc[i] = LV_GRID_FR(1);
    }
    rowDsc[rows] = LV_GRID_TEMPLATE_LAST;
    for (int i = 0; i < columns; ++i) {
        columnDsc[i] = LV_GRID_FR(1);
    }
    columnDsc[columns] = LV_GRID_TEMPLATE_LAST;
    lv_obj_set_grid_dsc_array(gridContainer, columnDsc, rowDsc);

    set_grid_size();
    lv_obj_center(gridContainer);
    lv_obj_set_style_bg_color(gridContainer, BACKGROUND_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(gridContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(gridContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(gridContainer, outerPad, LV_PART_MAIN);
    lv_obj_set_style_pad_row(gridContainer, rowPad, LV_PART_MAIN);
    lv_obj_set_style_pad_column(gridContainer, columnPad, LV_PART_MAIN);
}

void lv_control_grid_set_outer_pad(int32_t pad) {
    outerPad = pad;
    set_grid_size();
    lv_obj_set_style_pad_all(gridContainer, outerPad, LV_PART_MAIN);
}

void lv_control_grid_set_row_pad(int32_t pad) {
    rowPad = pad;
    set_grid_size();
    lv_obj_set_style_pad_row(gridContainer, rowPad, LV_PART_MAIN);
}

void lv_control_grid_set_column_pad(int32_t pad) {
    columnPad = pad;
    set_grid_size();
    lv_obj_set_style_pad_column(gridContainer, columnPad, LV_PART_MAIN);
}

void lv_control_grid_test_fill() {
    for (int row = 0; row < rowCount; ++row) {
        for (int column = 0; column < columnCount; ++column) {
            lv_obj_t* obj = lv_button_create(gridContainer);
            lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, column, 1,
                                 LV_GRID_ALIGN_STRETCH, row, 1);

            if (((row * columnCount) + column) % 2 == 0) {
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

void lv_control_grid_clear() {
    while (lv_obj_get_child_count(gridContainer) > 0) {
        lv_obj_delete(lv_obj_get_child(gridContainer, 0));
    }
}

void lv_control_grid_remove(uint8_t index) {
    lv_obj_t *object = findCellChildByIndex(index);
    if (object == NULL)
        return;
    lv_obj_delete(object);
}

void lv_control_grid_add_image(uint8_t index, uint8_t format, uint8_t* data, uint8_t* end) {
    if (images[index].data != NULL)
        lv_control_grid_remove_image(index);
    uint16_t width = convertUInt16ToLittleEndian(data);
    uint16_t height = convertUInt16ToLittleEndian(data + 2);
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
    images[index] = imageDsc;
    memcpy(imageData, data, (end - data) + 1);
}

void lv_control_grid_modify_image(uint8_t index, uint8_t* data, uint8_t* end) {
    if (images[index].data == NULL) {
        ESP_LOGE(CONTROL_GRID_TAG, "Image with index %d can't be modified, because it is not valid", index);
        return;
    }
    uint32_t pixelIndex = convertInt32ToLittleEndian(data);
    data += 4;
    uint8_t bytesPerPixel = LV_COLOR_FORMAT_GET_SIZE(images[index].header.cf);
    memcpy((void *) images[index].data + (pixelIndex * bytesPerPixel), data, (end - data) + 1);
}

void lv_control_grid_remove_image(uint8_t index) {
    if (images[index].data == NULL) {
        ESP_LOGW(CONTROL_GRID_TAG, "The image with index %d has already been removed", index);
        return;
    }
    heap_caps_free((void *) images[index].data);
    images[index].data_size = 0;
    images[index].data = NULL;
}

esp_err_t lv_control_grid_add_button(uint8_t index1, uint8_t index2) {
    int32_t row = index1 / columnCount;
    int32_t column = index1 % columnCount;
    int32_t rowSpan = (index2 / columnCount) - row;
    int32_t columnSpan = (index2 % columnCount) - column;
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

    const uint32_t childCount = lv_obj_get_child_count(gridContainer);
    for (int i = 0; i < childCount; ++i) {
        lv_obj_t *child = lv_obj_get_child(gridContainer, i);
        int32_t childRow = lv_obj_get_style_grid_cell_row_pos(child, 0);
        int32_t childColumn = lv_obj_get_style_grid_cell_column_pos(child, 0);
        int32_t childRow2 = childRow + lv_obj_get_style_grid_cell_row_span(child, 0);
        int32_t childColumn2 = childColumn + lv_obj_get_style_grid_cell_column_span(child, 0);
        if (childColumn < (column + columnSpan) && childColumn2 > column &&
            childRow < (row + rowSpan) && childRow2 > row) {
            ESP_LOGE(CONTROL_GRID_TAG, "Tried to create button from cell %d to cell %d, but is overlapping with %d", index1, index2, (childRow * columnCount) + childColumn);
            return ESP_ERR_NOT_ALLOWED;
        }
    }

    lv_obj_t* obj = lv_button_create(gridContainer);
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, column, columnSpan,
                         LV_GRID_ALIGN_STRETCH, row, rowSpan);
    return ESP_OK;
}

uint8_t lv_control_grid_text(const uint8_t index, const uint8_t subIndex, uint8_t* data, uint8_t* end) {
    if (data > end)
        goto textFormatError;
    lv_obj_t *object = findCellChildByIndex(index);
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
    ESP_LOGE(CONTROL_GRID_TAG, "Couldn't set text for cell with index %d (sub: %d; dataLength: %d)", index, subIndex, end - data + 1);
    return 0;
}

uint8_t lv_control_grid_image(uint8_t index, uint8_t subIndex, uint8_t* data, uint8_t* end) {
    if (data > end)
        goto imageFormatError;
    lv_obj_t *object = findCellChildByIndex(index);
    if (object == NULL)
        goto imageFormatError;

    if (subIndex == 0) {
        lv_obj_t *image = lv_image_create(object);
        lv_image_set_src(image, images + *data);
        lv_obj_set_size(image, lv_obj_get_width(object), lv_obj_get_height(object));
        lv_image_set_inner_align(image, LV_IMAGE_ALIGN_STRETCH);
        return lv_obj_get_child_count(object);
    } else {
        lv_obj_t *image = lv_obj_get_child(object, subIndex - 1);
        if (image == NULL)
            goto imageFormatError;
        lv_image_set_src(image, images + *data);
        return subIndex;
    }
imageFormatError:
    ESP_LOGE(CONTROL_GRID_TAG, "Couldn't set image for cell with index %d (sub: %d; dataLength: %d)", index, subIndex, end - data + 1);
    return 0;
}

uint8_t *lv_control_grid_set_style(const uint8_t index, const uint8_t subIndex, lv_style_selector_t *styleSelector, uint8_t *data, uint8_t *end) {
    if (data > end)
        goto styleFormatError;
    lv_obj_t *object = findCellChildByIndex(index);
    if (object == NULL)
        goto styleFormatError;
    if (subIndex != 0) {
        object = lv_obj_get_child(object, subIndex - 1);
        if (object == NULL)
            goto styleFormatError;
    }

    bool considerScale = (*data == 0x01);
    if (considerScale)
        data++;
    int styleDataSize = end - data;
    if (*data <= 0x2F) {
        if (styleDataSize < 4)
            goto styleFormatError;
        int32_t styleData = convertInt32ToLittleEndian(data + 1);
        if (considerScale) {
            if (*data == 0x03 || *data == 0x04 || *data == 0x08 || *data == 0x09 ||
                *data == 0x11 || *data == 0x14 || *data == 0x19 || (*data >= 0x1D && *data <= 0x1F)) {
                styleData = (styleData << 8) / lv_obj_get_style_transform_scale_y(object, *styleSelector);
            } else {
                styleData = (styleData << 8) / lv_obj_get_style_transform_scale_x(object, *styleSelector);
            }
        }

        switch (*data) {
        case 0x00:
            *styleSelector = styleData;
            break;
        case 0x02:
            lv_obj_set_style_pad_all(object, styleData, *styleSelector);
            break;
        case 0x03:
            lv_obj_set_style_pad_top(object, styleData, *styleSelector);
            break;
        case 0x04:
            lv_obj_set_style_pad_bottom(object, styleData, *styleSelector);
            break;
        case 0x05:
            lv_obj_set_style_pad_left(object, styleData, *styleSelector);
            break;
        case 0x06:
            lv_obj_set_style_pad_right(object, styleData, *styleSelector);
            break;
        case 0x07:
            lv_obj_set_style_margin_all(object, styleData, *styleSelector);
            break;
        case 0x08:
            lv_obj_set_style_margin_top(object, styleData, *styleSelector);
            break;
        case 0x09:
            lv_obj_set_style_margin_bottom(object, styleData, *styleSelector);
            break;
        case 0x0A:
            lv_obj_set_style_margin_left(object, styleData, *styleSelector);
            break;
        case 0x0B:
            lv_obj_set_style_margin_right(object, styleData, *styleSelector);
            break;
        case 0x0C:
            lv_obj_set_style_border_width(object, styleData, *styleSelector);
            break;
        case 0x0D:
            lv_obj_set_style_outline_width(object, styleData, *styleSelector);
            break;
        case 0x0E:
            lv_obj_set_style_outline_pad(object, styleData, *styleSelector);
            break;
        case 0x0F:
            lv_obj_set_style_shadow_width(object, styleData, *styleSelector);
            break;
        case 0x10:
            lv_obj_set_style_shadow_offset_x(object, styleData, *styleSelector);
            break;
        case 0x11:
            lv_obj_set_style_shadow_offset_y(object, styleData, *styleSelector);
            break;
        case 0x12:
            lv_obj_set_style_shadow_spread(object, styleData, *styleSelector);
            break;
        case 0x13:
            lv_obj_set_style_text_letter_space(object, styleData, *styleSelector);
            break;
        case 0x14:
            lv_obj_set_style_text_line_space(object, styleData, *styleSelector);
            break;
        case 0x15:
            lv_obj_set_style_text_outline_stroke_width(object, styleData, *styleSelector);
            break;
        case 0x16:
            lv_obj_set_style_blur_radius(object, styleData, *styleSelector);
            break;
        case 0x17:
            lv_obj_set_style_drop_shadow_radius(object, styleData, *styleSelector);
            break;
        case 0x18:
            lv_obj_set_style_drop_shadow_offset_x(object, styleData, *styleSelector);
            break;
        case 0x19:
            lv_obj_set_style_drop_shadow_offset_y(object, styleData, *styleSelector);
            break;
        case 0x1A:
            lv_obj_set_style_width(object, styleData, *styleSelector);
            break;
        case 0x1B:
            lv_obj_set_style_min_width(object, styleData, *styleSelector);
            break;
        case 0x1C:
            lv_obj_set_style_max_width(object, styleData, *styleSelector);
            break;
        case 0x1D:
            lv_obj_set_style_height(object, styleData, *styleSelector);
            break;
        case 0x1E:
            lv_obj_set_style_min_height(object, styleData, *styleSelector);
            break;
        case 0x1F:
            lv_obj_set_style_max_height(object, styleData, *styleSelector);
            break;
        case 0x20:
            lv_obj_set_style_length(object, styleData, *styleSelector);
            break;
        case 0x21:
            lv_obj_set_style_x(object, styleData, *styleSelector);
            break;
        case 0x22:
            lv_obj_set_style_y(object, styleData, *styleSelector);
            break;
        case 0x23:
            lv_obj_set_style_transform_width(object, styleData, *styleSelector);
            break;
        case 0x24:
            lv_obj_set_style_transform_height(object, styleData, *styleSelector);
            break;
        case 0x25:
            lv_obj_set_style_translate_x(object, styleData, *styleSelector);
            break;
        case 0x26:
            lv_obj_set_style_translate_y(object, styleData, *styleSelector);
            break;
        case 0x27:
            lv_obj_set_style_translate_radial(object, styleData, *styleSelector);
            break;
        case 0x28:
            lv_obj_set_style_transform_scale(object, styleData, *styleSelector);
            lv_obj_remove_event_cb(object, alignPivotForScaleCB);
            lv_obj_add_event_cb(object, alignPivotForScaleCB, LV_EVENT_SIZE_CHANGED, NULL);
            alignPivotForScale(object, *styleSelector);
            break;
        case 0x29:
            lv_obj_set_style_transform_scale_x(object, styleData, *styleSelector);
            lv_obj_remove_event_cb(object, alignPivotForScaleCB);
            lv_obj_add_event_cb(object, alignPivotForScaleCB, LV_EVENT_SIZE_CHANGED, NULL);
            alignPivotForScale(object, *styleSelector);
            break;
        case 0x2A:
            lv_obj_set_style_transform_scale_y(object, styleData, *styleSelector);
            lv_obj_remove_event_cb(object, alignPivotForScaleCB);
            lv_obj_add_event_cb(object, alignPivotForScaleCB, LV_EVENT_SIZE_CHANGED, NULL);
            alignPivotForScale(object, *styleSelector);
            break;
        case 0x2B:
            lv_obj_set_style_transform_rotation(object, styleData, *styleSelector);
            break;
        case 0x2C:
            lv_obj_set_style_transform_pivot_x(object, styleData, *styleSelector);
            break;
        case 0x2D:
            lv_obj_set_style_transform_pivot_y(object, styleData, *styleSelector);
            break;
        case 0x2E:
            lv_obj_set_style_transform_skew_x(object, styleData, *styleSelector);
            break;
        case 0x2F:
            lv_obj_set_style_transform_skew_y(object, styleData, *styleSelector);
            break;
        default: ;
        }
        return data + 5;
    }
    if (*data <= 0x38) {
        if (styleDataSize < 4)
            goto styleFormatError;
        uint32_t rawData = (uint32_t) convertInt32ToLittleEndian(data + 1);
        lv_color_t color = lv_color_hex(rawData >> 8);
        lv_opa_t opa = rawData & 0xFF;
        switch (*data) {
        case 0x30:
            lv_obj_set_style_bg_color(object, color, *styleSelector);
            lv_obj_set_style_bg_opa(object, opa, *styleSelector);
            break;
        case 0x31:
            lv_obj_set_style_bg_grad_color(object, color, *styleSelector);
            lv_obj_set_style_bg_main_opa(object, opa, *styleSelector);
            break;
        case 0x32:
            lv_obj_set_style_bg_image_recolor(object, color, *styleSelector);
            lv_obj_set_style_bg_image_recolor_opa(object, opa, *styleSelector);
            break;
        case 0x33:
            lv_obj_set_style_border_color(object, color, *styleSelector);
            lv_obj_set_style_border_opa(object, opa, *styleSelector);
            break;
        case 0x34:
            lv_obj_set_style_outline_color(object, color, *styleSelector);
            lv_obj_set_style_outline_opa(object, opa, *styleSelector);
            break;
        case 0x35:
            lv_obj_set_style_shadow_color(object, color, *styleSelector);
            lv_obj_set_style_shadow_opa(object, opa, *styleSelector);
            break;
        case 0x36:
            lv_obj_set_style_text_color(object, color, *styleSelector);
            lv_obj_set_style_text_opa(object, opa, *styleSelector);
            break;
        case 0x37:
            lv_obj_set_style_text_outline_stroke_color(object, color, *styleSelector);
            lv_obj_set_style_text_outline_stroke_opa(object, opa, *styleSelector);
            break;
        case 0x38:
            lv_obj_set_style_drop_shadow_color(object, color, *styleSelector);
            lv_obj_set_style_drop_shadow_opa(object, opa, *styleSelector);
            break;
        default: ;
        }
        return data + 5;
    }
    if (*data <= 0x3E) {
        if (styleDataSize < 1)
            goto styleFormatError;
        uint8_t value = *(data + 1);
        switch (*data) {
        case 0x39:
            lv_obj_set_style_bg_image_src(object, images + value, *styleSelector);
            break;
        case 0x3A:
            lv_obj_set_style_bg_grad_opa(object, value, *styleSelector);
            break;
        case 0x3B:
            lv_obj_set_style_bg_main_stop(object, value, *styleSelector);
            break;
        case 0x3C:
            lv_obj_set_style_bg_grad_stop(object, value, *styleSelector);
            break;
        case 0x3D:
            lv_obj_set_style_bg_image_opa(object, value, *styleSelector);
            break;
        case 0x3E:
            lv_obj_set_style_text_font(object, &lv_font_montserrat_48, *styleSelector);
            lv_obj_set_style_transform_scale(object, (value << 8) / 48, *styleSelector | LV_PART_ANY);
            lv_obj_remove_event_cb(object, alignPivotForScaleCB);
            lv_obj_add_event_cb(object, alignPivotForScaleCB, LV_EVENT_SIZE_CHANGED, NULL);
            alignPivotForScale(object, *styleSelector);
            break;
        default: ;
        }
        return data + 2;
    }
    switch (*data) {
    case 0x3F:
        lv_obj_set_style_bg_grad_dir(object, LV_GRAD_DIR_NONE, *styleSelector);
        break;
    case 0x40:
        lv_obj_set_style_bg_grad_dir(object, LV_GRAD_DIR_VER, *styleSelector);
        break;
    case 0x41:
        lv_obj_set_style_bg_grad_dir(object, LV_GRAD_DIR_HOR, *styleSelector);
        break;
    case 0x42:
        lv_obj_set_style_bg_grad_dir(object, LV_GRAD_DIR_LINEAR, *styleSelector);
        break;
    case 0x43:
        lv_obj_set_style_bg_grad_dir(object, LV_GRAD_DIR_RADIAL, *styleSelector);
        break;
    case 0x44:
        lv_obj_set_style_bg_grad_dir(object, LV_GRAD_DIR_CONICAL, *styleSelector);
        break;
    case 0x45:
        lv_obj_set_style_border_side(object, LV_BORDER_SIDE_NONE, *styleSelector);
        break;
    case 0x46:
        lv_obj_set_style_border_side(object, lv_obj_get_style_border_side(object, *styleSelector) | LV_BORDER_SIDE_BOTTOM, *styleSelector);
        break;
    case 0x47:
        lv_obj_set_style_border_side(object, lv_obj_get_style_border_side(object, *styleSelector) | LV_BORDER_SIDE_TOP, *styleSelector);
        break;
    case 0x48:
        lv_obj_set_style_border_side(object, lv_obj_get_style_border_side(object, *styleSelector) | LV_BORDER_SIDE_LEFT, *styleSelector);
        break;
    case 0x49:
        lv_obj_set_style_border_side(object, lv_obj_get_style_border_side(object, *styleSelector) | LV_BORDER_SIDE_RIGHT, *styleSelector);
        break;
    case 0x4A:
        lv_obj_set_style_border_side(object, LV_BORDER_SIDE_FULL, *styleSelector);
        break;
    case 0x4B:
        lv_obj_set_style_text_decor(object, LV_TEXT_DECOR_NONE, *styleSelector);
        break;
    case 0x4C:
        lv_obj_set_style_text_decor(object, lv_obj_get_style_text_decor(object, *styleSelector) | LV_TEXT_DECOR_UNDERLINE, *styleSelector);
        break;
    case 0x4D:
        lv_obj_set_style_text_decor(object, lv_obj_get_style_text_decor(object, *styleSelector) | LV_TEXT_DECOR_STRIKETHROUGH, *styleSelector);
        break;
    case 0x4E:
        lv_obj_set_style_text_align(object, LV_TEXT_ALIGN_AUTO, *styleSelector);
        break;
    case 0x4F:
        lv_obj_set_style_text_align(object, LV_TEXT_ALIGN_LEFT, *styleSelector);
        break;
    case 0x50:
        lv_obj_set_style_text_align(object, LV_TEXT_ALIGN_CENTER, *styleSelector);
        break;
    case 0x51:
        lv_obj_set_style_text_align(object, LV_TEXT_ALIGN_RIGHT, *styleSelector);
        break;
    case 0x52:
        lv_obj_set_style_blur_backdrop(object, false, *styleSelector);
        break;
    case 0x53:
        lv_obj_set_style_blur_backdrop(object, true, *styleSelector);
        break;
    case 0x54:
        lv_obj_set_style_blur_quality(object, LV_BLUR_QUALITY_AUTO, *styleSelector);
        break;
    case 0x55:
        lv_obj_set_style_blur_quality(object, LV_BLUR_QUALITY_SPEED, *styleSelector);
        break;
    case 0x56:
        lv_obj_set_style_blur_quality(object, LV_BLUR_QUALITY_PRECISION, *styleSelector);
        break;
    case 0x57:
        lv_obj_set_style_drop_shadow_quality(object, LV_BLUR_QUALITY_AUTO, *styleSelector);
        break;
    case 0x58:
        lv_obj_set_style_drop_shadow_quality(object, LV_BLUR_QUALITY_SPEED, *styleSelector);
        break;
    case 0x59:
        lv_obj_set_style_drop_shadow_quality(object, LV_BLUR_QUALITY_PRECISION, *styleSelector);
        break;
    case 0x5A:
        lv_obj_set_style_align(object, LV_ALIGN_TOP_LEFT, *styleSelector);
        alignPivotForScale(object, *styleSelector);
        break;
    case 0x5B:
        lv_obj_set_style_align(object, LV_ALIGN_TOP_MID, *styleSelector);
        alignPivotForScale(object, *styleSelector);
        break;
    case 0x5C:
        lv_obj_set_style_align(object, LV_ALIGN_TOP_RIGHT, *styleSelector);
        alignPivotForScale(object, *styleSelector);
        break;
    case 0x5D:
        lv_obj_set_style_align(object, LV_ALIGN_LEFT_MID, *styleSelector);
        alignPivotForScale(object, *styleSelector);
        break;
    case 0x5E:
        lv_obj_set_style_align(object, LV_ALIGN_CENTER, *styleSelector);
        alignPivotForScale(object, *styleSelector);
        break;
    case 0x5F:
        lv_obj_set_style_align(object, LV_ALIGN_RIGHT_MID, *styleSelector);
        alignPivotForScale(object, *styleSelector);
        break;
    case 0x60:
        lv_obj_set_style_align(object, LV_ALIGN_BOTTOM_LEFT, *styleSelector);
        alignPivotForScale(object, *styleSelector);
        break;
    case 0x61:
        lv_obj_set_style_align(object, LV_ALIGN_BOTTOM_MID, *styleSelector);
        alignPivotForScale(object, *styleSelector);
        break;
    case 0x62:
        lv_obj_set_style_align(object, LV_ALIGN_BOTTOM_RIGHT, *styleSelector);
        alignPivotForScale(object, *styleSelector);
        break;
    case 0x63:
        lv_obj_update_layout(object);
        alignPivotForScaleForced(object, *styleSelector);
        break;
    case 0x64:
        lv_obj_update_layout(object);
        alignPivotForScaleForced(object, 0);
        for (uint32_t selector = 1; selector <= 0x0FFFFF; selector <<= 1) {
            alignPivotForScaleForced(object, selector);
        }
        break;
    case 0x65:
        lv_obj_remove_event_cb(object, alignPivotForScaleForcedCB);
        lv_obj_add_event_cb(object, alignPivotForScaleForcedCB, LV_EVENT_SIZE_CHANGED, NULL);
        break;
    case 0x66:
        lv_obj_remove_event_cb(object, alignPivotForScaleForcedCB);
        break;
    case 0x67:
        lv_obj_set_style_text_font(object, &lv_font_montserrat_14, *styleSelector);
        break;
    case 0x68:
        lv_obj_set_style_text_font(object, &lv_font_montserrat_48, *styleSelector);
        break;
    case 0x69:
        lv_obj_set_style_bg_image_tiled(object, false, *styleSelector);
        break;
    case 0x6A:
        lv_obj_set_style_bg_image_tiled(object, true, *styleSelector);
        break;
    default: ;
    }
    return data + 1;
styleFormatError:
    ESP_LOGE(CONTROL_GRID_TAG, "Couldn't set style for cell with index %d (sub: %d; key: 0x%02X; valueLength: %d)", index, subIndex, data <= end ? *data : -1, end - data);
    return end + 1;
}
