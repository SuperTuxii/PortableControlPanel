#include "ControlGrid.h"
#include "Connection.h"
#include "src/LvglRenderer.h"
#include <QJSValueIterator>
#include <QSettings>
#include <QDir>
#include "src/core/lv_obj_private.h"
#include "util.h"

#define CHECK_CONTROL_GRID(value)   if (!controlGrid) return value;

static uint8_t styleData[5];

void ControlGrid::reinitControlGrid() {
    lv_lock();
    controlGrid = lv_control_grid_create(lvglRenderer->getScreen());
    lv_unlock();
}

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
    const int32_t value = lv_obj_get_width(controlGrid->gridContainer) - (2 * controlGrid->outerPad);
    lv_unlock();
    return value;
}
int32_t ControlGrid::getCGHeight() const {
    CHECK_CONTROL_GRID(0)
    lv_lock();
    const int32_t value = lv_obj_get_height(controlGrid->gridContainer) - (2 * controlGrid->outerPad);
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

void ControlGrid::setScreenStyle(lv_style_selector_t styleSelector, const QJSValue& data) {
    CHECK_CONTROL_GRID()
    lv_lock();
    lv_obj_t *screen = lvglRenderer->getScreen();
    lv_style_t *style = getStyleData(screen, styleSelector, true);
    uint8_t *buffer = styleData;
    QJSValueIterator iterator(data);
    while (iterator.next()) {
        int part = 0;
        QVariant styleElement = iterator.value().toVariant();
        QVariantMap styleMap = styleElement.toMap();
        QVariant styleValue = styleMap.value("value");
        if (styleValue.typeId() == QMetaType::QVariantMap) {
            QVariantMap valueMap = styleValue.toMap();
            if (valueMap.contains("imageKey")) {
                const int16_t loadIndex = loadImage(valueMap.value("imageKey").toString());
                if (loadIndex >= 0 && loadIndex < 256) {
                    styleMap["value"] = QVariant(loadIndex);
                    styleElement = QVariant(styleMap);
                }
            }
        }
        while (!parseStyleElement(styleElement, buffer, styleData + 4, part)) {
            if (buffer != styleData && buffer <= styleData + 5)
                handle_style_data(controlGrid, screen, &style, &styleSelector, styleData, buffer - 1);
            buffer = styleData;
        }
        if (buffer != styleData && buffer <= styleData + 5)
            handle_style_data(controlGrid, screen, &style, &styleSelector, styleData, buffer - 1);
        buffer = styleData;
    }
    lv_obj_refresh_style(screen, lv_obj_style_get_selector_part(styleSelector), LV_STYLE_PROP_ANY);
    lv_unlock();
}
void ControlGrid::removeScreenStyle(const lv_style_selector_t styleSelector) const {
    lv_lock();
    ::removeStyle(lvglRenderer->getScreen(), styleSelector);
    lv_unlock();
}
void ControlGrid::removeScreenStyles() const {
    lv_lock();
    ::removeStyles(lvglRenderer->getScreen());
    lv_unlock();
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
void ControlGrid::move(const uint8_t fromIndex, const uint8_t toIndex) const {
    CHECK_CONTROL_GRID()
    lv_lock();
    lv_control_grid_move(controlGrid, fromIndex, toIndex);
    lv_unlock();
}
void ControlGrid::changeSize(const uint8_t index, const uint8_t index2) const {
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
int16_t ControlGrid::findImage(const QString& key) const {
    for (int16_t i = 0; i < 256; ++i) {
        if (loadedImages[i] == key) {
            return i;
        }
    }
    return -1;
}
int16_t ControlGrid::loadImage(const QString& key, int16_t index) {
    CHECK_CONTROL_GRID(-1)
    const QSettings settings;
    QDir settingsDir = QFileInfo(settings.fileName()).dir();
    if (!settingsDir.cd("images")) return -1;
    if (index < 0 || index >= 256) {
        const int16_t findIndex = findImage(key);
        if (findIndex != -1) return findIndex;
        for (int16_t i = 0; i < 256; ++i) {
            if (loadedImages[i].isEmpty()) {
                index = i;
                break;
            }
        }
        if (index < 0 || index >= 256) return -1;
    } else if (loadedImages[index] == key) {
        return index;
    }
    lv_lock();
    if (!loadedImages[index].isEmpty())
        lv_control_grid_remove_image(controlGrid, index);
    const qsizetype attrDelimiter = key.lastIndexOf("?");
    const QString imagePath = settingsDir.absoluteFilePath(key.sliced(0, attrDelimiter));
    QImage image(imagePath);
    QStringList attrs = key.sliced(attrDelimiter + 1).split(",", Qt::SkipEmptyParts);
    int colorFormat = Connection::ColorFormatARGB32_Premultiplied;
    QRect cropRect(0, 0, image.width(), image.height());
    QSize resizeSize(image.width(), image.height());
    for (const QString& attr : attrs) {
        if (!attr.contains("=")) continue;
        QString attrKey = attr.section("=", 0, 0);
        QString value = attr.section("=", 1);
        if (attrKey == "cropPos") {
            if (value.contains(";")) {
                const qsizetype delimiter = value.indexOf(";");
                cropRect.setTopLeft(QPoint(
                    value.sliced(0, delimiter).toInt(),
                    value.sliced(delimiter + 1).toInt()
                ));
            } else {
                const int val = value.toInt();
                cropRect.setTopLeft(QPoint(val, val));
            }
        } else if (attrKey == "cropSize") {
            if (value.contains(";")) {
                const qsizetype delimiter = value.indexOf(";");
                cropRect.setWidth(value.sliced(0, delimiter).toInt());
                cropRect.setHeight(value.sliced(delimiter + 1).toInt());
            } else {
                const int val = value.toInt();
                cropRect.setWidth(val);
                cropRect.setHeight(val);
            }
        } else if (attrKey == "resize") {
            if (value.contains(";")) {
                const qsizetype delimiter = value.indexOf(";");
                resizeSize.setWidth(value.sliced(0, delimiter).toInt());
                resizeSize.setHeight(value.sliced(delimiter + 1).toInt());
            } else {
                const int val = value.toInt();
                resizeSize.setWidth(val);
                resizeSize.setHeight(val);
            }
        } else if (attrKey == "colorFormat") {
            colorFormat = value.toInt();
        }
    }
    image = image.copy(cropRect).scaled(resizeSize);
    image.convertTo(Connection::colorFormatImageFormat(colorFormat));
    const int bytesPerLine = (image.depth() * image.width()) >> 3;
    QByteArray byteData((bytesPerLine * image.height()) + 4, Qt::Uninitialized);
    byteData[0] = static_cast<char>(image.width() >> 8);
    byteData[1] = static_cast<char>(image.width());
    byteData[2] = static_cast<char>(image.height() >> 8);
    byteData[3] = static_cast<char>(image.height());
    for (int y = 0; y < image.height(); ++y) {
        byteData.replace(4 + (y * bytesPerLine), bytesPerLine, reinterpret_cast<const char*>(image.scanLine(y)), bytesPerLine);
    }
    auto *dataStart = reinterpret_cast<uint8_t*>(byteData.data());
    const uint8_t *dataEnd = dataStart + byteData.length() - 1;
    lv_control_grid_add_image(controlGrid, index, colorFormat, dataStart, dataEnd);
    loadedImages[index] = key;
    lv_unlock();
    return index;
}
void ControlGrid::loadImages(const QSet<QString>& data) {
    removeUnusedImages(data);
    for (const auto &imageData : data)
        loadImage(imageData);
}
void ControlGrid::removeUnusedImages(const QSet<QString>& data) {
    CHECK_CONTROL_GRID()
    lv_lock();
    for (int i = 0; i < 256; ++i) {
        if (loadedImages[i].isEmpty() || data.contains(loadedImages[i])) continue;
        lv_control_grid_remove_image(controlGrid, i);
        loadedImages[i] = QString();
    }
    lv_unlock();
}
void ControlGrid::removeImage(const QString& key) {
    CHECK_CONTROL_GRID()
    lv_lock();
    for (int i = 0; i < 256; ++i) {
        if (loadedImages[i] != key) continue;
        lv_control_grid_remove_image(controlGrid, i);
        loadedImages[i] = QString();
        break;
    }
    lv_unlock();
}
void ControlGrid::clearImages() {
    CHECK_CONTROL_GRID()
    lv_lock();
    for (int i = 0; i < 256; ++i) {
        if (loadedImages[i].isEmpty()) continue;
        lv_control_grid_remove_image(controlGrid, i);
        loadedImages[i] = QString();
    }
    lv_unlock();
}
bool ControlGrid::addWidget(const QString& type, const uint8_t index, const uint8_t index2) const {
    if (type == "Button") {
        return addButton(index, index2);
    }
    return false;
}
bool ControlGrid::addButton(const uint8_t index, const uint8_t index2) const {
    CHECK_CONTROL_GRID(false)
    lv_lock();
    const bool value = lv_control_grid_add_button(controlGrid, index, index2);
    lv_unlock();
    return value;
}
uint8_t ControlGrid::subWidget(const QString& type, const uint8_t index, const uint8_t subIndex, const QJSValue& data) {
    CHECK_CONTROL_GRID(0)
    if (type == "Text") {
        return subText(index, subIndex, data.hasProperty("text") ? data.property("text").toString() : "");
    }
    if (type == "Image") {
        if (!data.property("image").hasProperty("imageKey")) return 0;
        return subImage(index, subIndex, data.property("image").property("imageKey").toString());
    }
    return 0;
}
uint8_t ControlGrid::subText(const uint8_t index, const uint8_t subIndex, const QString& text) const {
    CHECK_CONTROL_GRID(0)
    const std::string stdText = text.toStdString();
    const auto *data = reinterpret_cast<uint8_t*>(const_cast<char*>(stdText.c_str()));
    lv_lock();
    const uint8_t value = lv_control_grid_text(controlGrid, index, subIndex, data, data + stdText.length());
    lv_unlock();
    return value;
}
uint8_t ControlGrid::subImage(const uint8_t index, const uint8_t subIndex, const QString& key) {
    CHECK_CONTROL_GRID(0)
    if (key.length() <= 1) return 0;
    lv_lock();
    const int16_t loadIndex = loadImage(key);
    if (loadIndex < 0 || loadIndex >= 256) {
        lv_unlock();
        return 0;
    }
    const uint8_t imageIndex = loadIndex;
    const uint8_t value = lv_control_grid_image(controlGrid, index, subIndex, &imageIndex, &imageIndex);
    lv_unlock();
    return value;
}

bool ControlGrid::parseStyleElement(const QVariant& styleElement, uint8_t*& buffer, const uint8_t* bufferEnd, int& part) {
    const QVariantMap styleMap = styleElement.toMap();
    bool ok;
    const auto styleKey = static_cast<Connection::StyleKeys>(styleMap.value("attrKey").toInt(&ok));
    if (!ok || !styleMap.contains("value")) return true;
    const QVariant styleValue = styleMap.value("value");
    if (styleValue.typeId() == QMetaType::QVariantList) {
        QVariantList valueList = styleValue.toList();
        const uint32_t length = valueList.length();
        if (styleKey == Connection::PadAll || styleKey == Connection::MarginAll) { // All + 4 Dimensions
            if (length == 1) {
                if (bufferEnd - buffer < 4)
                    return false;
                const int32_t value = valueList[0].toInt();
                buffer[0] = styleKey;
                buffer[1] = value >> 24;
                buffer[2] = (value >> 16) & 0xFF;
                buffer[3] = (value >> 8) & 0xFF;
                buffer[4] = value & 0xFF;
                buffer += 5;
            } else {
                int32_t values[4];
                if (length == 2) {
                    values[0] = valueList[0].toInt();
                    values[1] = values[0];
                    values[2] = valueList[1].toInt();
                    values[3] = values[2];
                } else {
                    values[0] = valueList[0].toInt();
                    values[1] = valueList[2].toInt();
                    values[2] = valueList[1].toInt();
                    values[3] = valueList[3].toInt();
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
                const int32_t value = valueList[0].toInt();
                buffer[0] = styleKey;
                buffer[1] = value >> 24;
                buffer[2] = (value >> 16) & 0xFF;
                buffer[3] = (value >> 8) & 0xFF;
                buffer[4] = value & 0xFF;
                buffer += 5;
            } else {
                const int32_t values[] = { valueList[0].toInt(), valueList[1].toInt() };
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
            values[0] = valueList[0].toInt();
            values[1] = valueList[length == 1 ? 0 : 1].toInt();
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
        int32_t value = styleValue.toInt(&ok);
        if (!ok && styleValue.typeId() == QMetaType::QString && styleKey >= Connection::ColorOpacityStyleKeyMin)
            value = static_cast<int32_t>(styleValue.toString().slice(1).toUInt(nullptr, 16));
        else if (!ok)
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
        const auto value = static_cast<int16_t>(styleValue.toInt(&ok));
        if (!ok) return true;
        buffer[0] = styleKey;
        buffer[1] = (value >> 8) & 0xFF;
        buffer[2] = value & 0xFF;
        buffer += 3;
    } else if (styleKey <= Connection::ByteStyleKeyMax) {
        if (bufferEnd - buffer < 1)
            return false;
        buffer[0] = styleKey;
        buffer[1] = styleValue.toUInt(&ok) & 0xFF;
        if (!ok) return true;
        buffer += 2;
    } else if (styleKey <= Connection::NonTypeStyleKeyMax) {
        if (bufferEnd - buffer < 0)
            return false;
        buffer[0] = styleValue.toUInt(&ok) & 0xFF;
        if (!ok) return true;
        buffer += 1;
    }
    part = 0;
    return true;
}
void ControlGrid::setStyle(const uint8_t index, const uint8_t subIndex, lv_style_selector_t styleSelector, const QJSValue& data) {
    CHECK_CONTROL_GRID()
    lv_lock();
    lv_style_t *style = nullptr;
    uint8_t *buffer = styleData;
    QJSValueIterator iterator(data);
    while (iterator.next()) {
        int part = 0;
        QVariant styleElement = iterator.value().toVariant();
        QVariantMap styleMap = styleElement.toMap();
        QVariant styleValue = styleMap.value("value");
        if (styleValue.typeId() == QMetaType::QVariantMap) {
            QVariantMap valueMap = styleValue.toMap();
            if (valueMap.contains("imageKey")) {
                const int16_t loadIndex = loadImage(valueMap.value("imageKey").toString());
                if (loadIndex >= 0 && loadIndex < 256) {
                    styleMap["value"] = QVariant(loadIndex);
                    styleElement = QVariant(styleMap);
                }
            }
        }
        while (!parseStyleElement(styleElement, buffer, styleData + 4, part)) {
            if (buffer != styleData)
                lv_control_grid_set_style(controlGrid, index, subIndex, &style, &styleSelector, styleData, buffer - 1);
            buffer = styleData;
        }
        if (buffer != styleData)
            lv_control_grid_set_style(controlGrid, index, subIndex, &style, &styleSelector, styleData, buffer - 1);
        buffer = styleData;
    }
    lv_unlock();
}

void ControlGrid::removeStyle(const uint8_t index, const uint8_t subIndex, const lv_style_selector_t styleSelector) const {
    CHECK_CONTROL_GRID()
    lv_lock();
    lv_control_grid_remove_style(controlGrid, index, subIndex, styleSelector);
    lv_unlock();
}

void ControlGrid::removeStyles(const uint8_t index, const uint8_t subIndex) const {
    CHECK_CONTROL_GRID()
    lv_lock();
    lv_control_grid_remove_styles(controlGrid, index, subIndex);
    lv_unlock();
}

static lv_obj_t *findCellChildByIndex(const lv_control_grid_t *cg, const uint8_t index) {
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

void ControlGrid::insertCoordsData(QJSValue data) const {
    CHECK_CONTROL_GRID()
    lv_lock();
    const lv_obj_t *object = findCellChildByIndex(controlGrid, data.property("index").toUInt());
    if (object == nullptr) return;
    if (data.hasProperty("subIndex") && data.property("subIndex").toInt() > 0) {
        object = lv_obj_get_child(object, data.property("subIndex").toInt() - 1);
        if (object == nullptr) return;
    }
    lv_obj_update_layout(object);
    const int32_t baseX = object->coords.x1 - controlGrid->gridContainer->coords.x1 - controlGrid->outerPad - lv_obj_get_style_translate_x(object, LV_PART_MAIN);
    const int32_t baseY = object->coords.y1 - controlGrid->gridContainer->coords.y1 - controlGrid->outerPad - lv_obj_get_style_translate_y(object, LV_PART_MAIN);
    const int32_t baseWidth = lv_area_get_width(&object->coords);
    const int32_t baseHeight = lv_area_get_height(&object->coords);
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
