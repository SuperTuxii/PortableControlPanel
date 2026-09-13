#include "CachedImageProvider.h"
#include <QSettings>
#include "Connection.h"

QImage CachedImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize) {
    const QSettings settings;
    QDir settingsDir = QFileInfo(settings.fileName()).dir();
    if (!settingsDir.cd("images"))
        return QQuickImageProvider::requestImage(id, size, requestedSize);
    const qsizetype attrDelimiter = id.lastIndexOf("?");
    const QString path = settingsDir.absoluteFilePath(id.sliced(0, attrDelimiter));
    QStringList attrs = id.sliced(attrDelimiter + 1).split(",", Qt::SkipEmptyParts);
    QRect cropRect(0, 0, requestedSize.width(), requestedSize.height());
    QImage::Format colorFormat = QImage::Format_Invalid;
    for (const QString& attr : attrs) {
        if (!attr.contains("=")) continue;
        QString key = attr.section("=", 0, 0);
        QString value = attr.section("=", 1);
        if (key == "cropPos") {
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
        } else if (key == "cropSize") {
            if (value.contains(";")) {
                const qsizetype delimiter = value.indexOf(";");
                cropRect.setWidth(value.sliced(0, delimiter).toInt());
                cropRect.setHeight(value.sliced(delimiter + 1).toInt());
            } else {
                const int val = value.toInt();
                cropRect.setWidth(val);
                cropRect.setHeight(val);
            }
        } else if (key == "colorFormat") {
            colorFormat = Connection::colorFormatImageFormat(value.toInt());
        }
    }

    QImage image = QImage(path).copy(cropRect).scaled(requestedSize);
    if (colorFormat > QImage::Format_Invalid)
        image.convertTo(colorFormat);
    *size = requestedSize;
    return image;
}
