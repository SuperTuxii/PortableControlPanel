#ifndef CONTROLPANELSOFTWARE_CONNECTION_H
#define CONTROLPANELSOFTWARE_CONNECTION_H
#include <qmetaobject.h>
#include <QObject>
#include <QJSValue>
#include <QSerialPort>
#include <qqmlintegration.h>
#include <lvgl.h>
#include "protocol_macros.h"

class QJSEngine;
class QQmlEngine;

class Connection : public QObject {
    Q_OBJECT
    QML_SINGLETON
    QML_ELEMENT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

    bool serialConnected = false;
    QSerialPort serialPort;
    QQmlEngine *engine;
public:
    explicit Connection(QQmlEngine *engine, QObject *parent = nullptr);
    ~Connection() override;
    static Connection *create(QQmlEngine *qmlEngine, QJSEngine *);

    [[nodiscard]] bool isConnected() const;

    PROTOCOL_COMMANDS_ENUM
    PROTOCOL_STYLE_KEYS_ENUM
    PROTOCOL_STYLE_STATES_ENUM
    PROTOCOL_STYLE_PARTS_ENUM
    Q_ENUM(Commands)
    Q_ENUM(StyleKeys)
    Q_ENUM(StyleStates)
    Q_ENUM(StyleParts)
    Q_INVOKABLE static QString styleStateString(const int value) {
        QString key = QMetaEnum::fromType<StyleStates>().valueToKey(value);
        return key.isNull() ? key : key.slice(5);
    }
    Q_INVOKABLE static QString stylePartString(const int value) {
        QString key = QMetaEnum::fromType<StyleParts>().valueToKey(value);
        return key.isNull() ? key : key.slice(4);
    }
    Q_INVOKABLE static QString styleKeyString(const int value) {
        return QMetaEnum::fromType<StyleKeys>().valueToKey(value);
    }
public slots:
    void tryConnect();
    void connectSerial();
    void setBacklightBrightness(int brightness);
    void setLayout(int rows, int columns);
    void setOuterPad(int32_t pad);
    void setRowPad(int32_t pad);
    void setColumnPad(int32_t pad);
    void testFill();
    void clear();
    void move(uint8_t fromIndex, uint8_t toIndex);
    void changeSize(uint8_t index, uint8_t index2);
    void remove(uint8_t index, uint8_t subIndex);
    void addWidget(const QString& type, uint8_t index, uint8_t index2, const QJSValue& data);
    void setStyle(uint8_t index, uint8_t subIndex, const QJSValue& data);

    void serialReadReady();
    void serialErrorOccurred(QSerialPort::SerialPortError error);
    void serialAboutToClose();
signals:
    void connectedChanged();
    void connectionError(QString error);
};

#endif //CONTROLPANELSOFTWARE_CONNECTION_H
