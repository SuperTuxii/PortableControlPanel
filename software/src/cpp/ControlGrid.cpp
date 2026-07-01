#include "ControlGrid.h"
#include "src/LvglRenderer.h"
#include <QJSValueIterator>
#include "src/core/lv_obj_private.h"

#define CHECK_CONTROL_GRID(value)   if (!controlGrid) return value;

static uint8_t styleData[5];

ControlGrid::ControlGrid(QObject* parent) :
        QObject(parent),
        lvglRenderer(nullptr),
        controlGrid(nullptr) {

}

ControlGrid::~ControlGrid() {
    if (controlGrid && lvglRenderer) {
        lv_lock();
        lv_control_grid_delete(controlGrid);
        lv_unlock();
    }
}

int32_t ControlGrid::getCGWidth() const {
    CHECK_CONTROL_GRID(0)
    lv_lock();
    int32_t value = lv_obj_get_width(controlGrid->gridContainer) - (2 * controlGrid->outerPad);
    lv_unlock();
    return value;
}
int32_t ControlGrid::getCGHeight() const {
    CHECK_CONTROL_GRID(0)
    lv_lock();
    int32_t value = lv_obj_get_height(controlGrid->gridContainer) - (2 * controlGrid->outerPad);
    lv_unlock();
    return value;
}
uint8_t ControlGrid::getRowCount() const {
    CHECK_CONTROL_GRID(0)
    return controlGrid->rowCount;
}
uint8_t ControlGrid::getColumnCount() const {
    CHECK_CONTROL_GRID(0)
    return controlGrid->columnCount;
}
int32_t ControlGrid::getOuterPad() const {
    CHECK_CONTROL_GRID(0)
    return controlGrid->outerPad;
}
int32_t ControlGrid::getRowPad() const {
    CHECK_CONTROL_GRID(0)
    return controlGrid->rowPad;
}
int32_t ControlGrid::getColumnPad() const {
    CHECK_CONTROL_GRID(0)
    return controlGrid->columnPad;
}

void ControlGrid::setLayout(const int rows, const int columns) {
    CHECK_CONTROL_GRID()
    lv_lock();
    lv_control_grid_set_layout(controlGrid, rows, columns);
    lv_obj_update_layout(controlGrid->gridContainer);
    lv_unlock();
    emit layoutChanged();
    emit sizeChanged();
}
void ControlGrid::setOuterPad(const int32_t pad) {
    CHECK_CONTROL_GRID()
    lv_lock();
    lv_control_grid_set_outer_pad(controlGrid, pad);
    lv_unlock();
    emit outerPadChanged();
    emit sizeChanged();
}
void ControlGrid::setRowPad(const int32_t pad) {
    CHECK_CONTROL_GRID()
    lv_lock();
    lv_control_grid_set_row_pad(controlGrid, pad);
    lv_unlock();
    emit rowPadChanged();
}
void ControlGrid::setColumnPad(const int32_t pad) {
    CHECK_CONTROL_GRID()
    lv_lock();
    lv_control_grid_set_column_pad(controlGrid, pad);
    lv_unlock();
    emit columnPadChanged();
}
void ControlGrid::testFill() const {
    CHECK_CONTROL_GRID()
    lv_lock();
    lv_control_grid_test_fill(controlGrid);
    lv_unlock();
}
void ControlGrid::clear() const {
    CHECK_CONTROL_GRID()
    lv_lock();
    lv_control_grid_clear(controlGrid);
    lv_unlock();
}
void ControlGrid::move(uint8_t fromIndex, uint8_t toIndex) const {
    CHECK_CONTROL_GRID()
    lv_lock();
    lv_control_grid_move(controlGrid, fromIndex, toIndex);
    lv_unlock();
}
void ControlGrid::changeSize(uint8_t index, uint8_t index2) const {
    CHECK_CONTROL_GRID()
    lv_lock();
    lv_control_grid_change_size(controlGrid, index, index2);
    lv_unlock();
}
void ControlGrid::remove(const uint8_t index, const uint8_t subIndex) const {
    CHECK_CONTROL_GRID()
    lv_lock();
    lv_control_grid_remove(controlGrid, index, subIndex);
    lv_unlock();
}
bool ControlGrid::addWidget(const QString& type, uint8_t index, uint8_t index2) const {
    if (type == "Button") {
        return addButton(index, index2);
    }
    return false;
}
bool ControlGrid::addButton(const uint8_t index, const uint8_t index2) const {
    CHECK_CONTROL_GRID(false)
    lv_lock();
    bool value = lv_control_grid_add_button(controlGrid, index, index2);
    lv_unlock();
    return value;
}
void ControlGrid::subText(const uint8_t index, const uint8_t subIndex, const QString& text) const {
    CHECK_CONTROL_GRID()
    uint8_t *data = reinterpret_cast<uint8_t*>(const_cast<char*>(text.toStdString().c_str()));
    lv_lock();
    lv_control_grid_text(controlGrid, index, subIndex, data, data + text.length());
    lv_unlock();
}
void ControlGrid::subImage(const uint8_t index, const uint8_t subIndex, uint8_t imageIndex) const {
    CHECK_CONTROL_GRID()
    lv_lock();
    lv_control_grid_image(controlGrid, index, subIndex, &imageIndex, &imageIndex);
    lv_unlock();
}
bool ControlGrid::parseStyleElement(const QJSValue &styleElement, uint8_t *&buffer, const uint8_t *bufferEnd, int &part) {
    if (!styleElement.property("attrKey").isNumber() || !styleElement.hasProperty("value"))
        return true;
    Connection::StyleKeys styleKey = static_cast<Connection::StyleKeys>(styleElement.property("attrKey").toInt());
    QJSValue styleValue = styleElement.property("value");
    if (!styleValue.isNumber() && !styleValue.isString() && styleValue.hasProperty("length")) {
        uint32_t length = styleValue.property("length").toUInt();
        if (styleKey == Connection::PadAll || styleKey == Connection::MarginAll) { // All + 4 Dimensions
            if (length == 1) {
                if (bufferEnd - buffer < 4)
                    return false;
                int32_t value = styleValue.property(0).toInt();
                buffer[0] = styleKey;
                buffer[1] = value >> 24;
                buffer[2] = (value >> 16) & 0xFF;
                buffer[3] = (value >> 8) & 0xFF;
                buffer[4] = value & 0xFF;
                buffer += 5;
            } else {
                int32_t values[4];
                if (length == 2) {
                    values[0] = styleValue.property(0).toInt();
                    values[1] = values[0];
                    values[2] = styleValue.property(1).toInt();
                    values[3] = values[2];
                } else {
                    values[0] = styleValue.property(0).toInt();
                    values[1] = styleValue.property(2).toInt();
                    values[2] = styleValue.property(1).toInt();
                    values[3] = styleValue.property(3).toInt();
                }
                for (; part < 4; ++part) {
                    if (bufferEnd - buffer < 4)
                        return false;
                    buffer[0] = styleKey + part + 1;
                    buffer[1] = values[part] >> 24;
                    buffer[2] = (values[part] >> 16) & 0xFF;
                    buffer[3] = (values[part] >> 8) & 0xFF;
                    buffer[4] = values[part] & 0xFF;
                    buffer += 5;
                }
            }
        } else if (styleKey == Connection::TranslateScale) { // Both + 2 Dimensions
            if (length == 1) {
                if (bufferEnd - buffer < 4)
                    return false;
                int32_t value = styleValue.property(0).toInt();
                buffer[0] = styleKey;
                buffer[1] = value >> 24;
                buffer[2] = (value >> 16) & 0xFF;
                buffer[3] = (value >> 8) & 0xFF;
                buffer[4] = value & 0xFF;
                buffer += 5;
            } else {
                int32_t values[] = { styleValue.property(0).toInt(), styleValue.property(1).toInt() };
                for (; part < 2; ++part) {
                    if (bufferEnd - buffer < 4)
                        return false;
                    buffer[0] = styleKey + part + 1;
                    buffer[1] = values[part] >> 24;
                    buffer[2] = (values[part] >> 16) & 0xFF;
                    buffer[3] = (values[part] >> 8) & 0xFF;
                    buffer[4] = values[part] & 0xFF;
                    buffer += 5;
                }
            }
        } else { // 2 Dimensions (Number 16/32)
            int32_t values[2];
            values[0] = styleValue.property(0).toInt();
            values[1] = styleValue.property(length == 1 ? 0 : 1).toInt();
            for (; part < 2; ++part) {
                if (styleKey <= Connection::NumberStyleKeyMax) { // Number (32)
                    if (bufferEnd - buffer < 4)
                        return false;
                    buffer[0] = styleKey + part;
                    buffer[1] = values[part] >> 24;
                    buffer[2] = (values[part] >> 16) & 0xFF;
                    buffer[3] = (values[part] >> 8) & 0xFF;
                    buffer[4] = values[part] & 0xFF;
                    buffer += 5;
                } else { // Number (16)
                    if (bufferEnd - buffer < 2)
                        return false;
                    buffer[0] = styleKey + part;
                    buffer[1] = (values[part] >> 8) & 0xFF;
                    buffer[2] = values[part] & 0xFF;
                    buffer += 3;
                }
            }
        }
    } else if (styleKey <= Connection::ColorOpacityStyleKeyMax) {
        if (bufferEnd - buffer < 4)
            return false;
        int32_t value;
        if (styleValue.isNumber())
            value = styleValue.toInt();
        else if (styleValue.isString() && styleKey >= Connection::ColorOpacityStyleKeyMin)
            value = static_cast<int32_t>(styleValue.toString().slice(1).toUInt(nullptr, 16));
        else
            return true;
        buffer[0] = styleKey;
        buffer[1] = value >> 24;
        buffer[2] = (value >> 16) & 0xFF;
        buffer[3] = (value >> 8) & 0xFF;
        buffer[4] = value & 0xFF;
        buffer += 5;
    } else if (styleKey <= Connection::Number16StyleKeyMax) {
        if (bufferEnd - buffer < 2)
            return false;
        if (!styleValue.isNumber())
            return true;
        const auto value = static_cast<int16_t>(styleValue.toInt());
        buffer[0] = styleKey;
        buffer[1] = (value >> 8) & 0xFF;
        buffer[2] = value & 0xFF;
        buffer += 3;
    } else if (styleKey <= Connection::ByteStyleKeyMax) {
        if (bufferEnd - buffer < 1)
            return false;
        if (!styleValue.isNumber())
            return true;
        buffer[0] = styleKey;
        buffer[1] = styleValue.toUInt() & 0xFF;
        buffer += 2;
    } else if (styleKey <= Connection::NonTypeStyleKeyMax) {
        if (bufferEnd - buffer < 0)
            return false;
        if (!styleValue.isNumber())
            return true;
        buffer[0] = styleValue.toUInt() & 0xFF;
        buffer += 1;
    }
    part = 0;
    return true;
}
void ControlGrid::setStyle(uint8_t index, uint8_t subIndex, lv_style_selector_t styleSelector, const QJSValue& data) const {
    CHECK_CONTROL_GRID()
    lv_lock();
    uint8_t *buffer = styleData;
    QJSValueIterator iterator(data);
    while (iterator.next()) {
        int part = 0;
        QJSValue styleElement = iterator.value();
        while (!parseStyleElement(styleElement, buffer, styleData + 4, part)) {
            if (buffer != styleData)
                lv_control_grid_set_style(controlGrid, index, subIndex, &styleSelector, styleData, buffer - 1);
            buffer = styleData;
        }
        if (buffer != styleData)
            lv_control_grid_set_style(controlGrid, index, subIndex, &styleSelector, styleData, buffer - 1);
        buffer = styleData;
    }
    lv_unlock();
}

static lv_obj_t *findCellChildByIndex(lv_control_grid_t *cg, const uint8_t index) {
    const uint8_t row = index / cg->columnCount;
    const uint8_t column = index % cg->columnCount;
    const uint32_t childCount = lv_obj_get_child_count(cg->gridContainer);
    for (int i = 0; i < childCount; ++i) {
        lv_obj_t *child = lv_obj_get_child(cg->gridContainer, i);
        const int32_t rowSpan = row - lv_obj_get_style_grid_cell_row_pos(child, LV_PART_MAIN);
        const int32_t columnSpan = column - lv_obj_get_style_grid_cell_column_pos(child, LV_PART_MAIN);
        if (rowSpan >= 0 && rowSpan < lv_obj_get_style_grid_cell_row_span(child, LV_PART_MAIN) &&
            columnSpan >= 0 && columnSpan < lv_obj_get_style_grid_cell_column_span(child, LV_PART_MAIN)) {
            return child;
            }
    }
    return nullptr;
}

void ControlGrid::setupDragTarget(QJSValue data) const {
    CHECK_CONTROL_GRID()
    lv_lock();
    lv_obj_t *object = findCellChildByIndex(controlGrid, data.property("index").toUInt());
    if (object == nullptr) return;
    int32_t baseX = object->coords.x1 - controlGrid->gridContainer->coords.x1 - controlGrid->outerPad - lv_obj_get_style_translate_x(object, LV_PART_MAIN);
    int32_t baseY = object->coords.y1 - controlGrid->gridContainer->coords.y1 - controlGrid->outerPad - lv_obj_get_style_translate_y(object, LV_PART_MAIN);
    int32_t baseWidth = lv_area_get_width(&object->coords);
    int32_t baseHeight = lv_area_get_height(&object->coords);
    lv_unlock();
    data.setProperty("baseX", baseX);
    data.setProperty("baseY", baseY);
    data.setProperty("x", baseX);
    data.setProperty("y", baseY);
    data.setProperty("baseWidth", baseWidth);
    data.setProperty("baseHeight", baseHeight);
    data.setProperty("width", baseWidth);
    data.setProperty("height", baseHeight);
}
