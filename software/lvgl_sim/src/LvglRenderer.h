#ifndef CONTROLPANELSOFTWARE_LVGLRENDERER_H
#define CONTROLPANELSOFTWARE_LVGLRENDERER_H

#include <QObject>
#include <qqmlintegration.h>
#include <QImage>
#include <QPixmap>
#include <lvgl.h>

#include "LvglImageProvider.h"

class QQmlEngine;

class LvglRenderer : public QObject, public QQmlParserStatus {
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT
    Q_PROPERTY(int width READ getWidth WRITE setWidth NOTIFY propertiesInitialized)
    Q_PROPERTY(int height READ getHeight WRITE setHeight NOTIFY propertiesInitialized)
    Q_PROPERTY(int tickPeriod READ getTickPeriodMs WRITE setTickPeriodMs NOTIFY propertiesInitialized)
    Q_PROPERTY(size_t displayBufferRatio READ getDisplayBufferRatio WRITE setDisplayBufferRatio NOTIFY propertiesInitialized)
    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY propertiesInitialized)
    Q_PROPERTY(LvglImageProvider* backend READ getLvglImageProvider NOTIFY propertiesInitialized)
public:
    static constexpr QImage::Format IMAGE_FORMAT = QImage::Format_ARGB32;
    static constexpr int BYTES_PER_PIXEL = 4;
private:
    int width;
    int height;
    int tickPeriodMs;
    size_t displayBufferRatio;

    uint8_t *displayFrame1;
    uint8_t *displayFrame2;
    lv_color32_t *currentFrame;
    lv_display_t *display;
    lv_obj_t *screen;

    QString name;
    QImage image;
    QQmlEngine *engine;
    LvglImageProvider *lvglImageProvider;
public:
    explicit LvglRenderer(QObject *parent = nullptr);
    ~LvglRenderer() override;

    void classBegin() override {}
    void componentComplete() override;

    void flush(const lv_display_t* display, const lv_area_t* area, const uint8_t* px_map) const;
    [[nodiscard]] int getWidth() const { return width; }
    [[nodiscard]] int getHeight() const { return height; }
    [[nodiscard]] int getTickPeriodMs() const { return tickPeriodMs; }
    [[nodiscard]] size_t getDisplayBufferRatio() const { return displayBufferRatio; }
    [[nodiscard]] lv_obj_t* getScreen() const { return screen; }
    [[nodiscard]] QString getName() const { return name; }
    [[nodiscard]] QPixmap getPixmap() const { return QPixmap::fromImage(image); }
    [[nodiscard]] LvglImageProvider* getLvglImageProvider() const { return lvglImageProvider; }
    void setWidth(const int width) { if (displayFrame1 == nullptr) this->width = width; }
    void setHeight(const int height) { if (displayFrame1 == nullptr) this->height = height; }
    void setTickPeriodMs(const int tickPeriodMs) { if (displayFrame1 == nullptr) this->tickPeriodMs = tickPeriodMs; }
    void setDisplayBufferRatio(const size_t displayBufferRatio) { if (displayFrame1 == nullptr) this->displayBufferRatio = displayBufferRatio; }
    void setName(const QString& name) { if (displayFrame1 == nullptr) this->name = name; }
    static uint32_t toAscii(Qt::Key key);
public slots:
    void setMouseArea(QObject* mouseArea) const { lvglImageProvider->setMouseArea(mouseArea); }
signals:
    void propertiesInitialized();
};

#endif //CONTROLPANELSOFTWARE_LVGLRENDERER_H
