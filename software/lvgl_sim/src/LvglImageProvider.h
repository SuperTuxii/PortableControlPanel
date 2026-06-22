#ifndef CONTROLPANELSOFTWARE_LVGLIMAGEPROVIDER_H
#define CONTROLPANELSOFTWARE_LVGLIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <lvgl.h>

class LvglRenderer;

class LvglImageProvider : public QQuickImageProvider {
    Q_OBJECT

    LvglRenderer &renderer;
    QObject *mouseArea;
    Qt::Key key;
    lv_indev_t* mouseDevice;
    lv_indev_t* keyboardDevice;
public:
    explicit LvglImageProvider(LvglRenderer& renderer);
    ~LvglImageProvider() override;

    QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requestedSize) override;
    void setMouseArea(QObject* mouseAreaObj);
    QPointF getMousePosition() const;
    bool isMousePressed() const;
    Qt::Key getKey() const;
public slots:
    void onKeyEvent(Qt::Key keyVal, bool isPressed);
};

#endif //CONTROLPANELSOFTWARE_LVGLIMAGEPROVIDER_H
