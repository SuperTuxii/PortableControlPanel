#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <QtLogging>
#include <QJSValueIterator>
#include "Connection.h"
#include "ControlGrid.h"

#define CHECK_CONNECTION(value)    if (!isConnected()) return value;

static uint8_t writeBuffer[260];

Connection::Connection(QQmlEngine* engine, QObject* parent) : QObject(parent) {
    this->engine = engine;
}

Connection::~Connection() {
    if (serialFileDescriptor >= 0)
        close(serialFileDescriptor);
}

Connection* Connection::create(QQmlEngine* qmlEngine, QJSEngine*) {
    return new Connection(qmlEngine);
}

void Connection::tryConnect() {
    connectSerial();
}

void Connection::connectSerial() {
    if (isConnected())
        return;

    serialFileDescriptor = open(serialPort.toStdString().c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (serialFileDescriptor < 0) {
        qWarning("Failed to connect via Serial Port %s (open): %s", serialPort.toStdString().c_str(), strerror(errno));
        return;
    }
    termios tty = {};
    if (tcgetattr(serialFileDescriptor, &tty) != 0) {
        qWarning("Failed to connect via Serial Port %s (tcgetattr): %s", serialPort.toStdString().c_str(), strerror(errno));
        close(serialFileDescriptor);
        serialFileDescriptor = -1;
        return;
    }
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(serialFileDescriptor, TCSANOW, &tty) != 0) {
        qWarning("Failed to connect via Serial Port %s (tcsetattr): %s", serialPort.toStdString().c_str(), strerror(errno));
        close(serialFileDescriptor);
        serialFileDescriptor = -1;
        return;
    }

    clear();
    emit connectedChanged();
}

void Connection::setLayout(int rows, int columns) {
    CHECK_CONNECTION()
    if (rows * columns > 256 || rows * columns <= 0 || columns <= 0) return;
    writeBuffer[0] = SetLayoutCMD;
    writeBuffer[1] = (rows * columns) - 1;
    writeBuffer[2] = columns - 1;
    write(serialFileDescriptor, writeBuffer, 3);
}

void Connection::setOuterPad(int32_t pad) {
    CHECK_CONNECTION()
    writeBuffer[0] = SetOuterPadCMD;
    writeBuffer[1] = 3;
    writeBuffer[2] = pad >> 24;
    writeBuffer[3] = (pad >> 16) & 0xFF;
    writeBuffer[4] = (pad >> 8) & 0xFF;
    writeBuffer[5] = pad & 0xFF;
    write(serialFileDescriptor, writeBuffer, 6);
}

void Connection::setRowPad(int32_t pad) {
    CHECK_CONNECTION()
    writeBuffer[0] = SetRowPadCMD;
    writeBuffer[1] = 3;
    writeBuffer[2] = pad >> 24;
    writeBuffer[3] = (pad >> 16) & 0xFF;
    writeBuffer[4] = (pad >> 8) & 0xFF;
    writeBuffer[5] = pad & 0xFF;
    write(serialFileDescriptor, writeBuffer, 6);
}

void Connection::setColumnPad(int32_t pad) {
    CHECK_CONNECTION()
    writeBuffer[0] = SetColumnPadCMD;
    writeBuffer[1] = 3;
    writeBuffer[2] = pad >> 24;
    writeBuffer[3] = (pad >> 16) & 0xFF;
    writeBuffer[4] = (pad >> 8) & 0xFF;
    writeBuffer[5] = pad & 0xFF;
    write(serialFileDescriptor, writeBuffer, 6);
}

void Connection::testFill() const {
    CHECK_CONNECTION()
    writeBuffer[0] = TestFillCMD;
    write(serialFileDescriptor, writeBuffer, 1);
}

void Connection::clear() const {
    CHECK_CONNECTION()
    writeBuffer[0] = ClearCMD;
    write(serialFileDescriptor, writeBuffer, 1);
}

void Connection::move(uint8_t fromIndex, uint8_t toIndex) const {
    CHECK_CONNECTION()
    writeBuffer[0] = MoveWidgetCMD;
    writeBuffer[1] = fromIndex;
    writeBuffer[2] = toIndex;
    write(serialFileDescriptor, writeBuffer, 3);
}

void Connection::changeSize(uint8_t index, uint8_t index2) const {
    CHECK_CONNECTION()
    writeBuffer[0] = ChangeWidgetSizeCMD;
    writeBuffer[1] = index;
    writeBuffer[2] = index2;
    write(serialFileDescriptor, writeBuffer, 3);
}

void Connection::remove(uint8_t index, uint8_t subIndex) const {
    CHECK_CONNECTION()
    writeBuffer[0] = RemoveWidgetCMD;
    writeBuffer[1] = index;
    writeBuffer[2] = subIndex;
    write(serialFileDescriptor, writeBuffer, 3);
}

void Connection::addWidget(const QString& type, uint8_t index, uint8_t index2, const QJSValue& data) const {
    CHECK_CONNECTION()
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
                write(serialFileDescriptor, writeBuffer, writeBuffer[3] + 5);
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
    write(serialFileDescriptor, writeBuffer, writeBuffer[3] + 5);
}

void Connection::setStyle(uint8_t index, uint8_t subIndex, const QJSValue& data) const {
    CHECK_CONNECTION()
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
                write(serialFileDescriptor, writeBuffer, writeBuffer[3] + 5);
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
    write(serialFileDescriptor, writeBuffer, writeBuffer[3] + 5);
}
