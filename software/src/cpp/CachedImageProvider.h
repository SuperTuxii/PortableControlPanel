#ifndef CONTROLPANELSOFTWARE_CACHEDIMAGEPROVIDER_H
#define CONTROLPANELSOFTWARE_CACHEDIMAGEPROVIDER_H
#include <QQuickImageProvider>

class CachedImageProvider : public QQuickImageProvider {
public:
    CachedImageProvider() : QQuickImageProvider(Image) {}

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;
};

#endif //CONTROLPANELSOFTWARE_CACHEDIMAGEPROVIDER_H
