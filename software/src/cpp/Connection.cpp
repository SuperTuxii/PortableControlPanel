#include <QRegularExpression>
#include <QJSValueIterator>
#include <QDir>
#include "Connection.h"
#include "ControlGrid.h"

#define CHECK_CONNECTION(value)     if (!isConnected()) return value
#define WRITE_SERIAL(size)          serialPort.write(reinterpret_cast<const char*>(writeBuffer), size)

static uint8_t writeBuffer[260];

Connection::Connection(QQmlEngine* engine, QObject* parent) : QObject(parent) {
    this->engine = engine;
    connect(&serialPort, &QSerialPort::readyRead, this, &Connection::serialReadReady);
    connect(&serialPort, &QSerialPort::aboutToClose, this, &Connection::serialAboutToClose);
    connect(&serialPort, &QSerialPort::errorOccurred, this, &Connection::serialErrorOccurred);
}

Connection::~Connection() {
    serialPort.close();
}

Connection* Connection::create(QQmlEngine* qmlEngine, QJSEngine*) {
    return new Connection(qmlEngine);
}

bool Connection::isConnected() const {
    return serialConnected;
}

void Connection::tryConnect() {
    connectSerial();
}

void Connection::connectSerial() {
    if (isConnected())
        return;

    QDir deviceDirectory("/dev");
    for (const auto& port : deviceDirectory.entryList({"ttyACM*"}, QDir::System | QDir::Files)) {
        serialPort.setPortName("/dev/" + port);
        serialPort.setBaudRate(QSerialPort::Baud115200);
        serialPort.setDataBits(QSerialPort::Data8);
        serialPort.setParity(QSerialPort::NoParity);
        serialPort.setStopBits(QSerialPort::OneStop);
        serialPort.setFlowControl(QSerialPort::NoFlowControl);
        if (serialPort.open(QIODevice::ReadWrite)) {
            writeBuffer[0] = PrintProtocolInfoCMD;
            WRITE_SERIAL(1);

            uint8_t action = 0;
            if (serialPort.bytesAvailable() < 1 && !serialPort.waitForReadyRead(100)) goto initializeProtocolFailure;
            serialPort.read(reinterpret_cast<char*>(&action), 1);
            if (action != ProtocolInfoACT) goto initializeProtocolFailure;
            uint8_t textLength = 0;
            if (serialPort.bytesAvailable() < 1 && !serialPort.waitForReadyRead(100)) goto initializeProtocolFailure;
            serialPort.read(reinterpret_cast<char*>(&textLength), 1);
            while (serialPort.bytesAvailable() < textLength) if (!serialPort.waitForReadyRead(100)) break;
            if (serialPort.bytesAvailable() < textLength) goto initializeProtocolFailure;
            QString text = QString(serialPort.read(textLength));
            QStringList data = text.replace(QRegularExpression(R"(^\w* v([^ ]*) (\d*)x(\d*))"), R"(\1 \2 \3)").split(" ");
            if (data[0] != PROTOCOL_VERSION) {
                QString message = QString("Serial Protocol Version not matching: ") + PROTOCOL_VERSION + "/" + data[0];
                qWarning() << message.toStdString().c_str();
                emit connectionError(message);
                goto initializeProtocolFailure;
            }
            bool ok;
            int width = data[1].toInt(&ok);
            int height = 0;
            if (ok)
                height = data[2].toInt(&ok);
            if (!ok || width <= 0 || height <= 0) {
                qWarning() << "Failed to parse display size from Serial Protocol Information";
                emit connectionError("Failed to parse display size from Serial Protocol Information");
                goto initializeProtocolFailure;
            }
            emit updateDisplaySize(width, height);

            serialConnected = true;
            clear();
            emit connectedChanged();
            break;
        }
        qWarning() << "Serial Connection Error:" << serialPort.errorString();
        continue;
initializeProtocolFailure:
        serialPort.close();
    }
    if (!serialConnected)
        emit connectionError("Serial Connection Failed");
}

void Connection::setBacklightBrightness(int brightness) {
    CHECK_CONNECTION();
    writeBuffer[0] = SetBacklightBrightnessCMD;
    writeBuffer[1] = (brightness >> 8) & 0xFF;
    writeBuffer[2] = brightness & 0xFF;
    WRITE_SERIAL(3);
}

void Connection::setLayout(int rows, int columns) {
    CHECK_CONNECTION();
    if (rows * columns > 256 || rows * columns <= 0 || columns <= 0) return;
    writeBuffer[0] = SetLayoutCMD;
    writeBuffer[1] = (rows * columns) - 1;
    writeBuffer[2] = columns - 1;
    WRITE_SERIAL(3);
}

void Connection::setOuterPad(int32_t pad) {
    CHECK_CONNECTION();
    writeBuffer[0] = SetOuterPadCMD;
    writeBuffer[1] = 3;
    writeBuffer[2] = pad >> 24;
    writeBuffer[3] = (pad >> 16) & 0xFF;
    writeBuffer[4] = (pad >> 8) & 0xFF;
    writeBuffer[5] = pad & 0xFF;
    WRITE_SERIAL(6);
}

void Connection::setRowPad(int32_t pad) {
    CHECK_CONNECTION();
    writeBuffer[0] = SetRowPadCMD;
    writeBuffer[1] = 3;
    writeBuffer[2] = pad >> 24;
    writeBuffer[3] = (pad >> 16) & 0xFF;
    writeBuffer[4] = (pad >> 8) & 0xFF;
    writeBuffer[5] = pad & 0xFF;
    WRITE_SERIAL(6);
}

void Connection::setColumnPad(int32_t pad) {
    CHECK_CONNECTION();
    writeBuffer[0] = SetColumnPadCMD;
    writeBuffer[1] = 3;
    writeBuffer[2] = pad >> 24;
    writeBuffer[3] = (pad >> 16) & 0xFF;
    writeBuffer[4] = (pad >> 8) & 0xFF;
    writeBuffer[5] = pad & 0xFF;
    WRITE_SERIAL(6);
}

void Connection::testFill() {
    CHECK_CONNECTION();
    writeBuffer[0] = TestFillCMD;
    WRITE_SERIAL(1);
}

void Connection::clear() {
    CHECK_CONNECTION();
    writeBuffer[0] = ClearCMD;
    WRITE_SERIAL(1);
}

void Connection::move(uint8_t fromIndex, uint8_t toIndex) {
    CHECK_CONNECTION();
    writeBuffer[0] = MoveWidgetCMD;
    writeBuffer[1] = fromIndex;
    writeBuffer[2] = toIndex;
    WRITE_SERIAL(3);
}

void Connection::changeSize(uint8_t index, uint8_t index2) {
    CHECK_CONNECTION();
    writeBuffer[0] = ChangeWidgetSizeCMD;
    writeBuffer[1] = index;
    writeBuffer[2] = index2;
    WRITE_SERIAL(3);
}

void Connection::remove(uint8_t index, uint8_t subIndex) {
    CHECK_CONNECTION();
    writeBuffer[0] = RemoveWidgetCMD;
    writeBuffer[1] = index;
    writeBuffer[2] = subIndex;
    WRITE_SERIAL(3);
}

void Connection::addWidget(const QString& type, uint8_t index, uint8_t index2, const QJSValue& data) {
    CHECK_CONNECTION();
    if (type == "Button")
        writeBuffer[0] = CreateButtonCMD;
    else
        return;
    writeBuffer[1] = index;
    writeBuffer[2] = index2;
    uint8_t *dataBuffer = writeBuffer + 4;
    uint8_t *dataEnd = dataBuffer + 255;
    QJSValueIterator selectorIterator(data);
    while (selectorIterator.next()) {
        bool ok;
        uint32_t styleSelector = selectorIterator.name().toUInt(&ok);
        if (!ok) continue;
        dataBuffer[0] = SetStyleSelector;
        dataBuffer[1] = styleSelector >> 24;
        dataBuffer[2] = (styleSelector >> 16) & 0xFF;
        dataBuffer[3] = (styleSelector >> 8) & 0xFF;
        dataBuffer[4] = styleSelector & 0xFF;
        dataBuffer += 5;
        QJSValueIterator elementIterator(selectorIterator.value());
        while (elementIterator.next()) {
            int part = 0;
            QJSValue styleElement = elementIterator.value();
            while (!ControlGrid::parseStyleElement(styleElement, dataBuffer, dataEnd, part)) {
                writeBuffer[3] = dataBuffer - writeBuffer - 5;
                WRITE_SERIAL(writeBuffer[3] + 5);
                writeBuffer[0] = SetStyleDataCMD;
                writeBuffer[1] = index;
                writeBuffer[2] = 0;
                dataBuffer = writeBuffer + 4;
                dataBuffer[0] = SetStyleSelector;
                dataBuffer[1] = styleSelector >> 24;
                dataBuffer[2] = (styleSelector >> 16) & 0xFF;
                dataBuffer[3] = (styleSelector >> 8) & 0xFF;
                dataBuffer[4] = styleSelector & 0xFF;
                dataBuffer += 5;
            }
        }
    }
    if (writeBuffer[0] == CreateButtonCMD && dataBuffer == writeBuffer + 4) {
        writeBuffer[3] = 4;
        writeBuffer[4] = SetStyleSelector;
        writeBuffer[5] = 0;
        writeBuffer[6] = 0;
        writeBuffer[7] = 0;
        writeBuffer[8] = 0;
    } else {
        if (dataBuffer == writeBuffer + 4) return;
        writeBuffer[3] = dataBuffer - writeBuffer - 5;
    }
    WRITE_SERIAL(writeBuffer[3] + 5);
}

void Connection::setStyle(uint8_t index, uint8_t subIndex, const QJSValue& data) {
    CHECK_CONNECTION();
    writeBuffer[0] = SetStyleDataCMD;
    writeBuffer[1] = index;
    writeBuffer[2] = subIndex;
    uint8_t *dataBuffer = writeBuffer + 4;
    uint8_t *dataEnd = dataBuffer + 255;
    QJSValueIterator selectorIterator(data);
    while (selectorIterator.next()) {
        bool ok;
        uint32_t styleSelector = selectorIterator.name().toUInt(&ok);
        if (!ok) continue;
        dataBuffer[0] = SetStyleSelector;
        dataBuffer[1] = styleSelector >> 24;
        dataBuffer[2] = (styleSelector >> 16) & 0xFF;
        dataBuffer[3] = (styleSelector >> 8) & 0xFF;
        dataBuffer[4] = styleSelector & 0xFF;
        dataBuffer += 5;
        QJSValueIterator elementIterator(selectorIterator.value());
        while (elementIterator.next()) {
            int part = 0;
            QJSValue styleElement = elementIterator.value();
            while (!ControlGrid::parseStyleElement(styleElement, dataBuffer, dataEnd, part)) {
                writeBuffer[3] = dataBuffer - writeBuffer - 5;
                WRITE_SERIAL(writeBuffer[3] + 5);
                writeBuffer[0] = SetStyleDataCMD;
                writeBuffer[1] = index;
                writeBuffer[2] = subIndex;
                dataBuffer = writeBuffer + 4;
                dataBuffer[0] = SetStyleSelector;
                dataBuffer[1] = styleSelector >> 24;
                dataBuffer[2] = (styleSelector >> 16) & 0xFF;
                dataBuffer[3] = (styleSelector >> 8) & 0xFF;
                dataBuffer[4] = styleSelector & 0xFF;
                dataBuffer += 5;
            }
        }
    }
    if (dataBuffer == writeBuffer + 4) return;
    writeBuffer[3] = dataBuffer - writeBuffer - 5;
    WRITE_SERIAL(writeBuffer[3] + 5);
}

void Connection::serialReadReady() {
    if (!serialConnected) return;
}

void Connection::serialErrorOccurred(QSerialPort::SerialPortError error) {
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

void Connection::serialAboutToClose() {
    if (!serialConnected) return;
    serialConnected = false;
    emit connectedChanged();
}
