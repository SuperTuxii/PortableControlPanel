#ifndef CONTROLPANELSOFTWARE_CONNECTION_H
#define CONTROLPANELSOFTWARE_CONNECTION_H
#include <qmetaobject.h>
#include <QObject>
#include <qqmlintegration.h>
#include <lvgl.h>
#include <QThread>

#include "ConnectionWorker.h"
#include "ControlGrid.h"
#include "protocol_macros.h"

#define WORKER_METHOD_INVOKER(methodName) \
void methodName() { \
    QMetaObject::invokeMethod(this->worker, &ConnectionWorker::methodName, Qt::QueuedConnection); \
}
#define WORKER_METHOD_1ARG_INVOKER(methodName, argType1, arg1) \
void methodName(argType1 arg1) { \
    QMetaObject::invokeMethod(this->worker, &ConnectionWorker::methodName, Qt::QueuedConnection, arg1); \
}
#define WORKER_METHOD_2ARG_INVOKER(methodName, argType1, arg1, argType2, arg2) \
void methodName(argType1 arg1, argType2 arg2) { \
    QMetaObject::invokeMethod(this->worker, &ConnectionWorker::methodName, Qt::QueuedConnection, arg1, arg2); \
}
#define WORKER_METHOD_3ARG_INVOKER(methodName, argType1, arg1, argType2, arg2, argType3, arg3) \
void methodName(argType1 arg1, argType2 arg2, argType3 arg3) { \
    QMetaObject::invokeMethod(this->worker, &ConnectionWorker::methodName, Qt::QueuedConnection, arg1, arg2, arg3); \
}

class ConnectionWorker;
class QNetworkReply;
class QJSEngine;
class QQmlEngine;

class Connection : public QObject {
    Q_OBJECT
    QML_SINGLETON
    QML_ELEMENT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

    bool connected = false;
    QQmlEngine *engine;
    QThread workerThread;
    ConnectionWorker *worker;
    QNetworkAccessManager *networkManager;
public:
    explicit Connection(QQmlEngine *engine, QObject *parent = nullptr);
    ~Connection() override;
    static Connection *create(QQmlEngine *qmlEngine, QJSEngine *);

    [[nodiscard]] bool isConnected() const;

    PROTOCOL_COMMANDS_ENUM
    PROTOCOL_ACTIONS_ENUM
    PROTOCOL_STYLE_KEYS_ENUM
    PROTOCOL_STYLE_STATES_ENUM
    PROTOCOL_STYLE_PARTS_ENUM
    PROTOCOL_COLOR_FORMATS_ENUM
    Q_ENUM(Commands)
    Q_ENUM(Actions)
    Q_ENUM(StyleKeys)
    Q_ENUM(StyleStates)
    Q_ENUM(StyleParts)
    Q_ENUM(ColorFormats)
    Q_INVOKABLE static QString styleStateString(const int value) {
        QString key = QMetaEnum::fromType<StyleStates>().valueToKey(value);
        return key.isNull() ? key : key.slice(5);
    }
    Q_INVOKABLE static int styleStateFromString(const QString& value) {
        return QMetaEnum::fromType<StyleStates>().keyToValue(("State" + value).toStdString().c_str());
    }
    Q_INVOKABLE static QString stylePartString(const int value) {
        QString key = QMetaEnum::fromType<StyleParts>().valueToKey(value);
        return key.isNull() ? key : key.slice(4);
    }
    Q_INVOKABLE static int stylePartFromString(const QString& value) {
        return QMetaEnum::fromType<StyleParts>().keyToValue(("Part" + value).toStdString().c_str());
    }
    Q_INVOKABLE static QString styleKeyString(const int value) {
        return QMetaEnum::fromType<StyleKeys>().valueToKey(value);
    }
    Q_INVOKABLE static QList<int> colorFormatValues() {
        QList<int> list;
        const QMetaEnum metaEnum = QMetaEnum::fromType<ColorFormats>();
        for (int i = 0; i < metaEnum.keyCount(); ++i) {
            list.append(metaEnum.value(i));
        }
        return list;
    }
    Q_INVOKABLE static QString colorFormatString(const int value) {
        QString key = QMetaEnum::fromType<ColorFormats>().valueToKey(value);
        return key.isNull() ? key : key.slice(11);
    }
    Q_INVOKABLE static QImage::Format colorFormatImageFormat(const int value) {
        return static_cast<QImage::Format>(QMetaEnum::fromType<QImage::Format>().keyToValue(
            ("Format_" + colorFormatString(value)).toStdString().c_str())
        );
    }
private:
    void handleConnectionChanged(bool isConnected);

    void onImageDownloadComplete(QNetworkReply *reply, const QString &replaceImagePath);
public slots:
    WORKER_METHOD_INVOKER(tryConnect)
    WORKER_METHOD_INVOKER(connectSerial)

    WORKER_METHOD_1ARG_INVOKER(setBacklightBrightness, int, brightness)
    void setScreenStyle(const QJSValue& data) {
        QMetaObject::invokeMethod(this->worker, &ConnectionWorker::setScreenStyle, Qt::QueuedConnection, data.toVariant());
    }
    WORKER_METHOD_1ARG_INVOKER(removeScreenStyle, uint32_t, styleSelector)
    WORKER_METHOD_INVOKER(removeScreenStyles)
    WORKER_METHOD_2ARG_INVOKER(setLayout, int, rows, int, columns)
    WORKER_METHOD_1ARG_INVOKER(setOuterPad, int32_t, pad)
    WORKER_METHOD_1ARG_INVOKER(setRowPad, int32_t, pad)
    WORKER_METHOD_1ARG_INVOKER(setColumnPad, int32_t, pad)
    WORKER_METHOD_INVOKER(testFill)
    WORKER_METHOD_INVOKER(clear)
    WORKER_METHOD_2ARG_INVOKER(move, uint8_t, fromIndex, uint8_t, toIndex)
    WORKER_METHOD_2ARG_INVOKER(changeSize, uint8_t, index, uint8_t, index2)
    WORKER_METHOD_2ARG_INVOKER(remove, uint8_t, index, uint8_t, subIndex)
    void loadImages(const QStringList& data) {
        QMetaObject::invokeMethod(this->worker, &ConnectionWorker::loadImages, Qt::QueuedConnection, QSet(data.begin(), data.end()));
    }
    void removeUnusedImages(const QStringList& data) {
        QMetaObject::invokeMethod(this->worker, &ConnectionWorker::removeUnusedImages, Qt::QueuedConnection, QSet(data.begin(), data.end()));
    }
    void removeImage(uint8_t index) const {
        QMetaObject::invokeMethod(this->worker, "removeImage", Qt::QueuedConnection, index);
    }
    void removeImage(const QString& key) const {
        QMetaObject::invokeMethod(this->worker, "removeImage", Qt::QueuedConnection, key);
    }
    WORKER_METHOD_INVOKER(clearImages)
    void addWidget(const QString& type, uint8_t index, uint8_t index2, const QJSValue& data) {
        QMetaObject::invokeMethod(this->worker, &ConnectionWorker::addWidget, Qt::QueuedConnection, type, index, index2, data.toVariant());
    }
    void subWidget(const QString& type, uint8_t index, uint8_t subIndex, bool addNew, const QJSValue& data) {
        QMetaObject::invokeMethod(this->worker, &ConnectionWorker::subWidget, Qt::QueuedConnection, type, index, subIndex, addNew, data.toVariant());
    }
    void setStyle(uint8_t index, uint8_t subIndex, const QJSValue& data) {
        QMetaObject::invokeMethod(this->worker, &ConnectionWorker::setStyle, Qt::QueuedConnection, index, subIndex, data.toVariant());
    }
    WORKER_METHOD_3ARG_INVOKER(removeStyle, uint8_t, index, uint8_t, subIndex, uint32_t, styleSelector)
    WORKER_METHOD_2ARG_INVOKER(removeStyles, uint8_t, index, uint8_t, subIndex)

    static QSize imageSize(const QString &path);
    void cacheImage(const QString &urlString, const QString &replaceImagePath = QString());
    static void deleteCachedImage(const QString &path);

    static QString loadSymbolConfig();
signals:
    void connectedChanged();
    void connectionError(QString error);

    void updateDisplaySize(int width, int height);

    void imageCached(QString urlString, QString filename, QString path);
    void imageCachingFailed(QString urlString, QString error);
};

#endif //CONTROLPANELSOFTWARE_CONNECTION_H
