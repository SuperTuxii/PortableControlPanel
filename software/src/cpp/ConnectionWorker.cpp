#include "ConnectionWorker.h"
#include <QRegularExpression>
#include <QSerialPortInfo>
#include <QNetworkReply>
#include <QSettings>

#include "Connection.h"

#define CHECK_CONNECTION(value)     if (!isConnected()) return value
#define CHECK_CONFIRMATION(value)   if (!checkConfirmation()) return value
#define WRITE_SERIAL(size)          serialPort.write(reinterpret_cast<const char*>(writeBuffer), size)
#define WRITE_WITH_OPERANDS_DATA(dataBuffer)                                        \
                                    writeBuffer[3] = dataBuffer - writeBuffer - 5;  \
                                    WRITE_SERIAL(dataBuffer - writeBuffer)
#define WRITE_WITH_DATA(dataBuffer) writeBuffer[1] = dataBuffer - writeBuffer - 3;  \
                                    WRITE_SERIAL(dataBuffer - writeBuffer)
#define BEGIN_NEW_STYLE(dataBuffer, index, subIndex, styleSelector)                 \
                                    writeBuffer[0] = Connection::SetStyleDataCMD;   \
                                    writeBuffer[1] = index;                         \
                                    writeBuffer[2] = subIndex;                      \
                                    dataBuffer = writeBuffer + 4;                   \
                                    dataBuffer[0] = Connection::SetStyleSelector;   \
                                    dataBuffer[1] = styleSelector >> 24;            \
                                    dataBuffer[2] = (styleSelector >> 16) & 0xFF;   \
                                    dataBuffer[3] = (styleSelector >> 8) & 0xFF;    \
                                    dataBuffer[4] = styleSelector & 0xFF;           \
                                    dataBuffer += 5
#define BEGIN_NEW_SCREEN_STYLE(dataBuffer, styleSelector)                           \
                                    writeBuffer[0] = Connection::SetScreenStyleCMD; \
                                    dataBuffer = writeBuffer + 2;                   \
                                    dataBuffer[0] = Connection::SetStyleSelector;   \
                                    dataBuffer[1] = styleSelector >> 24;            \
                                    dataBuffer[2] = (styleSelector >> 16) & 0xFF;   \
                                    dataBuffer[3] = (styleSelector >> 8) & 0xFF;    \
                                    dataBuffer[4] = styleSelector & 0xFF;           \
                                    dataBuffer += 5

static uint8_t writeBuffer[260];

ConnectionWorker::ConnectionWorker(QObject* parent) : QObject(parent), serialPort(this) {
    connect(&serialPort, &QSerialPort::readyRead, this, &ConnectionWorker::serialReadReady);
    connect(&serialPort, &QSerialPort::aboutToClose, this, &ConnectionWorker::serialAboutToClose);
    connect(&serialPort, &QSerialPort::errorOccurred, this, &ConnectionWorker::serialErrorOccurred);
}

ConnectionWorker::~ConnectionWorker() {
    serialPort.close();
}

bool ConnectionWorker::isConnected() const {
    return serialConnected;
}

bool ConnectionWorker::checkConfirmation() {
    int8_t action = 0;
    uint8_t dataLength = 0;
    QByteArray data;
    if (serialPort.bytesAvailable() < 1 && !serialPort.waitForReadyRead(100)) goto confirmationFailure;
    serialPort.read(reinterpret_cast<char*>(&action), 1);
    if (serialPort.bytesAvailable() < 1 && !serialPort.waitForReadyRead(100)) goto confirmationFailure;
    serialPort.read(reinterpret_cast<char*>(&dataLength), 1);
    while (serialPort.bytesAvailable() <= dataLength) if (!serialPort.waitForReadyRead(100)) break;
    if (serialPort.bytesAvailable() <= dataLength) goto confirmationFailure;
    data = serialPort.read(dataLength+1);
    if (data == QByteArray::fromRawData(reinterpret_cast<const char*>(writeBuffer), dataLength+1)) return true;
confirmationFailure:
    const QByteArray sentData = QByteArray::fromRawData(reinterpret_cast<const char*>(writeBuffer), dataLength+1);
    qCritical() << "Confirmation for Command failed:" << sentData.toHex(' ') << "!=" << data.toHex(' ');
    emit connectionError("Confirmation for Command failed");
    writeBuffer[0] = Connection::RestartCMD;
    WRITE_SERIAL(1);
    return false;
}

void ConnectionWorker::tryConnect() {
    connectSerial();
}

void ConnectionWorker::connectSerial() {
    if (isConnected())
        return;

    for (const auto& port : QSerialPortInfo::availablePorts()) {
        serialPort.setPort(port);
        serialPort.setBaudRate(QSerialPort::Baud115200);
        serialPort.setDataBits(QSerialPort::Data8);
        serialPort.setParity(QSerialPort::NoParity);
        serialPort.setStopBits(QSerialPort::OneStop);
        serialPort.setFlowControl(QSerialPort::NoFlowControl);
        if (serialPort.open(QIODevice::ReadWrite)) {
            writeBuffer[0] = Connection::PrintProtocolInfoCMD;
            WRITE_SERIAL(1);

            uint8_t action = 0;
            if (serialPort.bytesAvailable() < 1 && !serialPort.waitForReadyRead(100)) goto initializeProtocolFailure;
            serialPort.read(reinterpret_cast<char*>(&action), 1);
            if (action != Connection::ProtocolInfoACT) goto initializeProtocolFailure;
            uint8_t textLength = 0;
            if (serialPort.bytesAvailable() < 1 && !serialPort.waitForReadyRead(100)) goto initializeProtocolFailure;
            serialPort.read(reinterpret_cast<char*>(&textLength), 1);
            while (serialPort.bytesAvailable() < textLength) if (!serialPort.waitForReadyRead(100)) break;
            if (serialPort.bytesAvailable() < textLength) goto initializeProtocolFailure;
            QString text(serialPort.read(textLength));
            QStringList data = text.replace(QRegularExpression(R"(^\w* v([^ ]*) (\d*)x(\d*))"), R"(\1 \2 \3)").split(" ");
            if (data[0] != PROTOCOL_VERSION) {
                QString message = QString("Serial Protocol Version not matching: ") + PROTOCOL_VERSION + "/" + data[0];
                qWarning() << message.toStdString().c_str();
                emit connectionError(message);
                goto initializeProtocolFailure;
            }
            bool ok;
            const int width = data[1].toInt(&ok);
            int height = 0;
            if (ok)
                height = data[2].toInt(&ok);
            if (!ok || width <= 0 || height <= 0) {
                qWarning() << "Failed to parse display size from Serial Protocol Information";
                emit connectionError("Failed to parse display size from Serial Protocol Information");
                goto initializeProtocolFailure;
            }
            emit updateDisplaySize(width, height);

            if (!checkConfirmation()) goto initializeProtocolFailure;

            serialConnected = true;
            clear();
            emit connectedChanged(true);
            break;
        }
        continue;
initializeProtocolFailure:
        serialPort.close();
    }
    if (!serialConnected)
        emit connectionError("Serial Connection Failed");
}

void ConnectionWorker::setBacklightBrightness(const int brightness) {
    CHECK_CONNECTION();
    writeBuffer[0] = Connection::SetBacklightBrightnessCMD;
    writeBuffer[1] = (brightness >> 8) & 0xFF;
    writeBuffer[2] = brightness & 0xFF;
    WRITE_SERIAL(3);
    checkConfirmation();
}

void ConnectionWorker::setScreenStyle(const QVariant& data) {
    CHECK_CONNECTION();
    writeBuffer[0] = Connection::SetScreenStyleCMD;
    uint8_t *dataBuffer = writeBuffer + 2;
    const uint8_t *dataEnd = dataBuffer + 255;
    QMapIterator selectorIterator(data.toMap());
    while (selectorIterator.hasNext()) {
        selectorIterator.next();
        bool ok;
        const uint32_t styleSelector = selectorIterator.key().toUInt(&ok);
        if (!ok) continue;
        if (dataEnd - dataBuffer < 4) {
            WRITE_WITH_DATA(dataBuffer);
            CHECK_CONFIRMATION();
            writeBuffer[0] = Connection::SetScreenStyleCMD;
            dataBuffer = writeBuffer + 2;
        }
        dataBuffer[0] = Connection::SetStyleSelector;
        dataBuffer[1] = styleSelector >> 24;
        dataBuffer[2] = (styleSelector >> 16) & 0xFF;
        dataBuffer[3] = (styleSelector >> 8) & 0xFF;
        dataBuffer[4] = styleSelector & 0xFF;
        dataBuffer += 5;
        for (auto styleElement : selectorIterator.value().toList()) {
            int part = 0;
            QVariantMap styleMap = styleElement.toMap();
            QVariant styleValue = styleMap.value("value");
            if (styleValue.typeId() == QMetaType::QVariantMap) {
                QVariantMap valueMap = styleValue.toMap();
                if (valueMap.contains("imageKey")) {
                    QString imageKey = valueMap.value("imageKey").toString();
                    int16_t imageIndex = findImage(imageKey);
                    if (imageIndex < 0) {
                        WRITE_WITH_DATA(dataBuffer);
                        CHECK_CONFIRMATION();
                        imageIndex = loadImage(imageKey);
                        BEGIN_NEW_SCREEN_STYLE(dataBuffer, styleSelector);
                    }
                    if (imageIndex >= 0 && imageIndex < 256) {
                        styleMap["value"] = QVariant(imageIndex);
                        styleElement = QVariant(styleMap);
                    }
                }
            }
            while (!ControlGrid::parseStyleElement(styleElement, dataBuffer, dataEnd, part)) {
                WRITE_WITH_DATA(dataBuffer);
                CHECK_CONFIRMATION();
                BEGIN_NEW_SCREEN_STYLE(dataBuffer, styleSelector);
            }
        }
    }
    if (dataBuffer == writeBuffer + 2) return;
    WRITE_WITH_DATA(dataBuffer);
    checkConfirmation();
}

void ConnectionWorker::removeScreenStyle(const uint32_t styleSelector) {
    CHECK_CONNECTION();
    writeBuffer[0] = Connection::ResetScreenStyleCMD;
    writeBuffer[1] = 3;
    writeBuffer[2] = styleSelector >> 24;
    writeBuffer[3] = (styleSelector >> 16) & 0xFF;
    writeBuffer[4] = (styleSelector >> 8) & 0xFF;
    writeBuffer[5] = styleSelector & 0xFF;
    WRITE_SERIAL(6);
    checkConfirmation();
}

void ConnectionWorker::removeScreenStyles() {
    CHECK_CONNECTION();
    writeBuffer[0] = Connection::ResetScreenStylesCMD;
    WRITE_SERIAL(1);
    checkConfirmation();
}

void ConnectionWorker::setLayout(const int rows, const int columns) {
    CHECK_CONNECTION();
    if (rows * columns > 256 || rows * columns <= 0 || columns <= 0) return;
    writeBuffer[0] = Connection::SetLayoutCMD;
    writeBuffer[1] = (rows * columns) - 1;
    writeBuffer[2] = columns - 1;
    WRITE_SERIAL(3);
    checkConfirmation();
}

void ConnectionWorker::setOuterPad(const int32_t pad) {
    CHECK_CONNECTION();
    writeBuffer[0] = Connection::SetOuterPadCMD;
    writeBuffer[1] = 3;
    writeBuffer[2] = pad >> 24;
    writeBuffer[3] = (pad >> 16) & 0xFF;
    writeBuffer[4] = (pad >> 8) & 0xFF;
    writeBuffer[5] = pad & 0xFF;
    WRITE_SERIAL(6);
    checkConfirmation();
}

void ConnectionWorker::setRowPad(const int32_t pad) {
    CHECK_CONNECTION();
    writeBuffer[0] = Connection::SetRowPadCMD;
    writeBuffer[1] = 3;
    writeBuffer[2] = pad >> 24;
    writeBuffer[3] = (pad >> 16) & 0xFF;
    writeBuffer[4] = (pad >> 8) & 0xFF;
    writeBuffer[5] = pad & 0xFF;
    WRITE_SERIAL(6);
    checkConfirmation();
}

void ConnectionWorker::setColumnPad(const int32_t pad) {
    CHECK_CONNECTION();
    writeBuffer[0] = Connection::SetColumnPadCMD;
    writeBuffer[1] = 3;
    writeBuffer[2] = pad >> 24;
    writeBuffer[3] = (pad >> 16) & 0xFF;
    writeBuffer[4] = (pad >> 8) & 0xFF;
    writeBuffer[5] = pad & 0xFF;
    WRITE_SERIAL(6);
    checkConfirmation();
}

void ConnectionWorker::testFill() {
    CHECK_CONNECTION();
    writeBuffer[0] = Connection::TestFillCMD;
    WRITE_SERIAL(1);
    checkConfirmation();
}

void ConnectionWorker::clear() {
    CHECK_CONNECTION();
    writeBuffer[0] = Connection::ClearCMD;
    WRITE_SERIAL(1);
    checkConfirmation();
}

void ConnectionWorker::move(const uint8_t fromIndex, const uint8_t toIndex) {
    CHECK_CONNECTION();
    writeBuffer[0] = Connection::MoveWidgetCMD;
    writeBuffer[1] = fromIndex;
    writeBuffer[2] = toIndex;
    WRITE_SERIAL(3);
    checkConfirmation();
}

void ConnectionWorker::changeSize(const uint8_t index, const uint8_t index2) {
    CHECK_CONNECTION();
    writeBuffer[0] = Connection::ChangeWidgetSizeCMD;
    writeBuffer[1] = index;
    writeBuffer[2] = index2;
    WRITE_SERIAL(3);
    checkConfirmation();
}

void ConnectionWorker::remove(const uint8_t index, const uint8_t subIndex) {
    CHECK_CONNECTION();
    writeBuffer[0] = Connection::RemoveWidgetCMD;
    writeBuffer[1] = index;
    writeBuffer[2] = subIndex;
    WRITE_SERIAL(3);
    checkConfirmation();
}

int16_t ConnectionWorker::findImage(const QString& key) const {
    for (int16_t i = 0; i < 256; ++i) {
        if (loadedImages[i] == key) {
            return i;
        }
    }
    return -1;
}
int16_t ConnectionWorker::loadImage(const QString& key, int16_t index) {
    CHECK_CONNECTION(-1);
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
    if (!loadedImages[index].isEmpty())
        removeImage(index);
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
    const int bytesPerPixel = image.depth() >> 3;
    const int bytesPerLine = bytesPerPixel * image.width();
    writeBuffer[0] = Connection::AddImageCMD;
    writeBuffer[1] = index;
    writeBuffer[2] = colorFormat;
    writeBuffer[3] = 3;
    writeBuffer[4] = image.width() >> 8;
    writeBuffer[5] = image.width();
    writeBuffer[6] = image.height() >> 8;
    writeBuffer[7] = image.height();
    uint8_t dataSize = 3;
    for (int y = 0; y < image.height(); ++y) {
        const uint8_t *scanLine = image.scanLine(y);
        const uint8_t *currPos = scanLine;
        const uint8_t *lineEnd = currPos + bytesPerLine;
        while (lineEnd - currPos >= 255 - dataSize) {
            memcpy(writeBuffer + dataSize + 5, currPos, 255 - dataSize);
            currPos += ((255 - dataSize) / bytesPerPixel) * bytesPerPixel;
            writeBuffer[3] = 255;
            WRITE_SERIAL(260);
            CHECK_CONFIRMATION(-1);
            writeBuffer[0] = Connection::ModifyImageCMD;
            writeBuffer[1] = index;
            writeBuffer[3] = 3;
            dataSize = 3;
            const uint32_t pixelIndex = (y * image.width()) + ((currPos - scanLine) / bytesPerPixel);
            writeBuffer[4] = pixelIndex >> 24;
            writeBuffer[5] = pixelIndex >> 16;
            writeBuffer[6] = pixelIndex >> 8;
            writeBuffer[7] = pixelIndex;
        }
        const uint8_t remainingBytes = lineEnd - currPos;
        memcpy(writeBuffer + dataSize + 5, currPos, remainingBytes);
        writeBuffer[3] += remainingBytes;
        dataSize += remainingBytes;
    }
    if (dataSize > 3 && writeBuffer[0] != Connection::AddImageCMD) {
        WRITE_SERIAL(dataSize + 5);
        CHECK_CONFIRMATION(-1);
    }
    loadedImages[index] = key;
    return index;
}
void ConnectionWorker::loadImages(const QSet<QString>& data) {
    removeUnusedImages(data);
    for (const auto &imageData : data)
        loadImage(imageData);
}
void ConnectionWorker::removeUnusedImages(const QSet<QString>& data) {
    for (int i = 0; i < 256; ++i) {
        if (loadedImages[i].isEmpty() || data.contains(loadedImages[i])) continue;
        removeImage(i);
        loadedImages[i] = QString();
    }
}
void ConnectionWorker::removeImage(const uint8_t index) {
    CHECK_CONNECTION();
    writeBuffer[0] = Connection::RemoveImageCMD;
    writeBuffer[1] = index;
    WRITE_SERIAL(3);
    checkConfirmation();
}
void ConnectionWorker::removeImage(const QString& key) {
    for (int i = 0; i < 256; ++i) {
        if (loadedImages[i] != key) continue;
        removeImage(i);
        loadedImages[i] = QString();
        break;
    }
}
void ConnectionWorker::clearImages() {
    for (int i = 0; i < 256; ++i) {
        if (loadedImages[i].isEmpty()) continue;
        removeImage(i);
        loadedImages[i] = QString();
    }
}

void ConnectionWorker::addWidget(const QString& type, const uint8_t index, const uint8_t index2, const QVariant& data) {
    CHECK_CONNECTION();
    if (type == "Button")
        writeBuffer[0] = Connection::CreateButtonCMD;
    else
        return;
    writeBuffer[1] = index;
    writeBuffer[2] = index2;
    uint8_t *dataBuffer = writeBuffer + 4;
    const uint8_t *dataEnd = dataBuffer + 255;
    QMapIterator selectorIterator(data.toMap());
    while (selectorIterator.hasNext()) {
        selectorIterator.next();
        bool ok;
        const uint32_t styleSelector = selectorIterator.key().toUInt(&ok);
        if (!ok) continue;
        if (dataEnd - dataBuffer < 4) {
            WRITE_WITH_OPERANDS_DATA(dataBuffer);
            CHECK_CONFIRMATION();
            writeBuffer[0] = Connection::SetStyleDataCMD;
            writeBuffer[1] = index;
            writeBuffer[2] = 0;
            dataBuffer = writeBuffer + 4;
        }
        dataBuffer[0] = Connection::SetStyleSelector;
        dataBuffer[1] = styleSelector >> 24;
        dataBuffer[2] = (styleSelector >> 16) & 0xFF;
        dataBuffer[3] = (styleSelector >> 8) & 0xFF;
        dataBuffer[4] = styleSelector & 0xFF;
        dataBuffer += 5;
        for (auto styleElement : selectorIterator.value().toList()) {
            int part = 0;
            QVariantMap styleMap = styleElement.toMap();
            QVariant styleValue = styleMap.value("value");
            if (styleValue.typeId() == QMetaType::QVariantMap) {
                QVariantMap valueMap = styleValue.toMap();
                if (valueMap.contains("imageKey")) {
                    QString imageKey = valueMap.value("imageKey").toString();
                    int16_t imageIndex = findImage(imageKey);
                    if (imageIndex < 0) {
                        WRITE_WITH_OPERANDS_DATA(dataBuffer);
                        CHECK_CONFIRMATION();
                        imageIndex = loadImage(imageKey);
                        BEGIN_NEW_STYLE(dataBuffer, index, 0, styleSelector);
                    }
                    if (imageIndex >= 0 && imageIndex < 256) {
                        styleMap["value"] = QVariant(imageIndex);
                        styleElement = QVariant(styleMap);
                    }
                }
            }
            while (!ControlGrid::parseStyleElement(styleElement, dataBuffer, dataEnd, part)) {
                WRITE_WITH_OPERANDS_DATA(dataBuffer);
                CHECK_CONFIRMATION();
                BEGIN_NEW_STYLE(dataBuffer, index, 0, styleSelector);
            }
        }
    }
    if (writeBuffer[0] == Connection::CreateButtonCMD && dataBuffer == writeBuffer + 4) {
        writeBuffer[3] = 4;
        writeBuffer[4] = Connection::SetStyleSelector;
        writeBuffer[5] = 0;
        writeBuffer[6] = 0;
        writeBuffer[7] = 0;
        writeBuffer[8] = 0;
    } else {
        if (dataBuffer == writeBuffer + 4) return;
        writeBuffer[3] = dataBuffer - writeBuffer - 5;
    }
    WRITE_SERIAL(writeBuffer[3] + 5);
    checkConfirmation();
}

void ConnectionWorker::subWidget(const QString& type, const uint8_t index, const uint8_t subIndex, const bool addNew, const QVariant& data) {
    CHECK_CONNECTION();
    uint8_t *dataBuffer = writeBuffer + 4;
    const uint8_t *dataEnd = dataBuffer + 255;
    const QVariantMap dataMap = data.toMap();
    if (type == "Text") {
        writeBuffer[0] = Connection::SubTextCMD;
        if (dataMap.contains("text")) {
            const std::string text = dataMap.value("text").toString().toStdString();
            for (int i = 0; i < text.length(); ++i) {
                dataBuffer[i] = text[i];
            }
            dataBuffer[text.length()] = 0;
            dataBuffer += text.length() + 1;
        } else {
            dataBuffer[0] = 0;
            dataBuffer++;
        }
    } else if (type == "Image") {
        const QVariantMap imageMap = dataMap.value("image").toMap();
        if (!imageMap.contains("imageKey")) return;
        const QString key = imageMap.value("imageKey").toString();
        if (key.length() <= 1) return;
        const int16_t loadIndex = loadImage(key);
        if (loadIndex < 0 || loadIndex >= 256) return;
        writeBuffer[0] = Connection::SubImageCMD;
        dataBuffer[0] = loadIndex;
        dataBuffer++;
    } else {
        return;
    }
    writeBuffer[1] = index;
    writeBuffer[2] = addNew ? 0 : subIndex;
    if (dataMap.contains("style")) {
        QMapIterator selectorIterator(dataMap.value("style").toMap());
        while (selectorIterator.hasNext()) {
            selectorIterator.next();
            bool ok;
            const uint32_t styleSelector = selectorIterator.key().toUInt(&ok);
            if (!ok) continue;
            if (dataEnd - dataBuffer < 4) {
                WRITE_WITH_OPERANDS_DATA(dataBuffer);
                CHECK_CONFIRMATION();
                writeBuffer[0] = Connection::SetStyleDataCMD;
                writeBuffer[1] = index;
                writeBuffer[2] = subIndex;
                dataBuffer = writeBuffer + 4;
            }
            dataBuffer[0] = Connection::SetStyleSelector;
            dataBuffer[1] = styleSelector >> 24;
            dataBuffer[2] = (styleSelector >> 16) & 0xFF;
            dataBuffer[3] = (styleSelector >> 8) & 0xFF;
            dataBuffer[4] = styleSelector & 0xFF;
            dataBuffer += 5;

            for (auto styleElement : selectorIterator.value().toList()) {
                int part = 0;
                QVariantMap styleMap = styleElement.toMap();
                QVariant styleValue = styleMap.value("value");
                if (styleValue.typeId() == QMetaType::QVariantMap) {
                    QVariantMap valueMap = styleValue.toMap();
                    if (valueMap.contains("imageKey")) {
                        QString imageKey = valueMap.value("imageKey").toString();
                        int16_t imageIndex = findImage(imageKey);
                        if (imageIndex < 0) {
                            WRITE_WITH_OPERANDS_DATA(dataBuffer);
                            CHECK_CONFIRMATION();
                            imageIndex = loadImage(imageKey);
                            BEGIN_NEW_STYLE(dataBuffer, index, 0, styleSelector);
                        }
                        if (imageIndex >= 0 && imageIndex < 256) {
                            styleMap["value"] = QVariant(imageIndex);
                            styleElement = QVariant(styleMap);
                        }
                    }
                }
                while (!ControlGrid::parseStyleElement(styleElement, dataBuffer, dataEnd, part)) {
                    WRITE_WITH_OPERANDS_DATA(dataBuffer);
                    CHECK_CONFIRMATION();
                    BEGIN_NEW_STYLE(dataBuffer, index, 0, styleSelector);
                }
            }
        }
    }
    if (writeBuffer[0] == Connection::SetStyleDataCMD && dataBuffer == writeBuffer + 4) return;
    WRITE_WITH_OPERANDS_DATA(dataBuffer);
    checkConfirmation();
}

void ConnectionWorker::setStyle(const uint8_t index, const uint8_t subIndex, const QVariant& data) {
    CHECK_CONNECTION();
    writeBuffer[0] = Connection::SetStyleDataCMD;
    writeBuffer[1] = index;
    writeBuffer[2] = subIndex;
    uint8_t *dataBuffer = writeBuffer + 4;
    const uint8_t *dataEnd = dataBuffer + 255;
    QMapIterator selectorIterator(data.toMap());
    while (selectorIterator.hasNext()) {
        selectorIterator.next();
        bool ok;
        const uint32_t styleSelector = selectorIterator.key().toUInt(&ok);
        if (!ok) continue;
        if (dataEnd - dataBuffer < 4) {
            WRITE_WITH_OPERANDS_DATA(dataBuffer);
            CHECK_CONFIRMATION();
            writeBuffer[0] = Connection::SetStyleDataCMD;
            writeBuffer[1] = index;
            writeBuffer[2] = 0;
            dataBuffer = writeBuffer + 4;
        }
        dataBuffer[0] = Connection::SetStyleSelector;
        dataBuffer[1] = styleSelector >> 24;
        dataBuffer[2] = (styleSelector >> 16) & 0xFF;
        dataBuffer[3] = (styleSelector >> 8) & 0xFF;
        dataBuffer[4] = styleSelector & 0xFF;
        dataBuffer += 5;
        for (auto styleElement : selectorIterator.value().toList()) {
            int part = 0;
            QVariantMap styleMap = styleElement.toMap();
            QVariant styleValue = styleMap.value("value");
            if (styleValue.typeId() == QMetaType::QVariantMap) {
                QVariantMap valueMap = styleValue.toMap();
                if (valueMap.contains("imageKey")) {
                    QString imageKey = valueMap.value("imageKey").toString();
                    int16_t imageIndex = findImage(imageKey);
                    if (imageIndex < 0) {
                        WRITE_WITH_OPERANDS_DATA(dataBuffer);
                        CHECK_CONFIRMATION();
                        imageIndex = loadImage(imageKey);
                        BEGIN_NEW_STYLE(dataBuffer, index, 0, styleSelector);
                    }
                    if (imageIndex >= 0 && imageIndex < 256) {
                        styleMap["value"] = QVariant(imageIndex);
                        styleElement = QVariant(styleMap);
                    }
                }
            }
            while (!ControlGrid::parseStyleElement(styleElement, dataBuffer, dataEnd, part)) {
                WRITE_WITH_OPERANDS_DATA(dataBuffer);
                CHECK_CONFIRMATION();
                BEGIN_NEW_STYLE(dataBuffer, index, 0, styleSelector);
            }
        }
    }
    if (dataBuffer == writeBuffer + 4) return;
    WRITE_WITH_OPERANDS_DATA(dataBuffer);
    checkConfirmation();
}

void ConnectionWorker::removeStyle(const uint8_t index, const uint8_t subIndex, const uint32_t styleSelector) {
    CHECK_CONNECTION();
    writeBuffer[0] = Connection::ResetStyleCMD;
    writeBuffer[1] = index;
    writeBuffer[2] = subIndex;
    writeBuffer[3] = 3;
    writeBuffer[4] = styleSelector >> 24;
    writeBuffer[5] = (styleSelector >> 16) & 0xFF;
    writeBuffer[6] = (styleSelector >> 8) & 0xFF;
    writeBuffer[7] = styleSelector & 0xFF;
    WRITE_SERIAL(8);
    checkConfirmation();
}

void ConnectionWorker::removeStyles(const uint8_t index, const uint8_t subIndex) {
    CHECK_CONNECTION();
    writeBuffer[0] = Connection::ResetStylesCMD;
    writeBuffer[1] = index;
    writeBuffer[2] = subIndex;
    WRITE_SERIAL(3);
    checkConfirmation();
}

void ConnectionWorker::serialReadReady() {
    if (!serialConnected) return;
    // while (serialPort.canReadLine())
    //     qInfo() << "Read:" << QString(serialPort.readLine());
}

void ConnectionWorker::serialErrorOccurred(const QSerialPort::SerialPortError error) {
    if (error == QSerialPort::ResourceError) {
        serialPort.close();
    } else if (error != QSerialPort::NoError
        && error != QSerialPort::NotOpenError
        && error != QSerialPort::DeviceNotFoundError
        && error != QSerialPort::TimeoutError) {
        qWarning() << "Serial Error Occurred:" << error;
        emit connectionError(QString("Serial Error Occurred: ") + QMetaEnum::fromType<QSerialPort::SerialPortError>().valueToKey(error));
        }
}

void ConnectionWorker::serialAboutToClose() {
    if (!serialConnected) return;
    serialConnected = false;
    emit connectedChanged(false);
}