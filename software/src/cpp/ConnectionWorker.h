#ifndef CONTROLPANELSOFTWARE_CONNECTIONWORKER_H
#define CONTROLPANELSOFTWARE_CONNECTIONWORKER_H
#include <QObject>
#include <QSerialPort>

class QJSValue;
class QQmlEngine;

class ConnectionWorker : public QObject {
    Q_OBJECT

    bool serialConnected = false;
    QSerialPort serialPort;
    QString loadedImages[256];
public:
    explicit ConnectionWorker(QObject *parent = nullptr);
    ~ConnectionWorker() override;

    [[nodiscard]] bool isConnected() const;
private:
    bool checkConfirmation();

    [[nodiscard]] int16_t findImage(const QString& key) const;
    int16_t loadImage(const QString& key, int16_t index = -1);
public slots:
    void tryConnect();
    void connectSerial();

    void setBacklightBrightness(int brightness);
    void setScreenStyle(const QVariant& data);
    void removeScreenStyle(uint32_t styleSelector);
    void removeScreenStyles();
    void setLayout(int rows, int columns);
    void setOuterPad(int32_t pad);
    void setRowPad(int32_t pad);
    void setColumnPad(int32_t pad);
    void testFill();
    void clear();
    void move(uint8_t fromIndex, uint8_t toIndex);
    void changeSize(uint8_t index, uint8_t index2);
    void remove(uint8_t index, uint8_t subIndex);
    void loadImages(const QSet<QString>& data);
    void removeUnusedImages(const QSet<QString>& data);
    void removeImage(uint8_t index);
    void removeImage(const QString& key);
    void clearImages();
    void addWidget(const QString& type, uint8_t index, uint8_t index2, const QVariant& data);
    void subWidget(const QString& type, uint8_t index, uint8_t subIndex, bool addNew, const QVariant& data);
    void setStyle(uint8_t index, uint8_t subIndex, const QVariant& data);
    void removeStyle(uint8_t index, uint8_t subIndex, uint32_t styleSelector);
    void removeStyles(uint8_t index, uint8_t subIndex);

    void serialReadReady();
    void serialErrorOccurred(QSerialPort::SerialPortError error);
    void serialAboutToClose();
signals:
    void connectedChanged(bool connected);
    void connectionError(QString error);

    void updateDisplaySize(int width, int height);
};

#endif //CONTROLPANELSOFTWARE_CONNECTIONWORKER_H
