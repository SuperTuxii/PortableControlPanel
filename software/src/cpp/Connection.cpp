#include "Connection.h"

#include <QRegularExpression>
#include <QJSValueIterator>
#include <QDir>
#include <QNetworkReply>
#include <QMimeDatabase>
#include <QSettings>
#include "ControlGrid.h"
#include "ConnectionWorker.h"
#include "Connection.h"

Connection::Connection(QQmlEngine* engine, QObject* parent) : QObject(parent), networkManager(new QNetworkAccessManager(this)) {
    this->engine = engine;
    this->worker = new ConnectionWorker();
    this->worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &ConnectionWorker::connectedChanged, this, &Connection::handleConnectionChanged);
    connect(worker, &ConnectionWorker::connectionError, this, &Connection::connectionError);
    connect(worker, &ConnectionWorker::updateDisplaySize, this, &Connection::updateDisplaySize);
    workerThread.start();
}

Connection::~Connection() {
    workerThread.quit();
    workerThread.wait();
}

Connection* Connection::create(QQmlEngine* qmlEngine, QJSEngine*) {
    return new Connection(qmlEngine);
}

bool Connection::isConnected() const {
    return connected;
}

void Connection::handleConnectionChanged(const bool isConnected) {
    this->connected = isConnected;
    emit connectedChanged();
}

QSize Connection::imageSize(const QString& path) {
    const QImage image(path);
    return {image.width(), image.height()};
}
void Connection::cacheImage(const QString& urlString, const QString &replaceImagePath) {
    const QUrl url(urlString);
    if (!url.isValid()) {
        qWarning() << "URL for image caching is invalid:" << urlString;
        emit imageCachingFailed(urlString, "URL for image caching is invalid: \"" + urlString + "\"");
        return;
    }
    if (url.isLocalFile()) {
        const QString localPath = url.toLocalFile();
        const QFileInfo fileInfo(localPath);
        if (!fileInfo.exists() || !fileInfo.isFile()) {
            qWarning() << "Local file for image caching is invalid:" << localPath;
            emit imageCachingFailed(urlString, "Local file for image caching is invalid: \"" + localPath + "\"");
            return;
        }
        const QString mimeType = QMimeDatabase().mimeTypeForFile(localPath).name();
        if (!mimeType.startsWith("image/", Qt::CaseInsensitive)) {
            qWarning() << "Local file is not an image:" << localPath << "is" << mimeType;
            emit imageCachingFailed(urlString, "Local file is not an image: \"" + localPath + "\" is \"" + mimeType + "\"");
            return;
        }
    }

    QNetworkReply *reply = networkManager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, replaceImagePath] {
        this->onImageDownloadComplete(reply, replaceImagePath);
    });
}

void Connection::onImageDownloadComplete(QNetworkReply* reply, const QString &replaceImagePath) {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network error while downloading image:" << reply->errorString();
        emit imageCachingFailed(reply->url().toString(), "Network error while downloading image: \"" + reply->errorString() + "\"");
        return;
    }
    if (!reply->url().isLocalFile()) {
        const QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
        if (!contentType.startsWith("image/", Qt::CaseInsensitive)) {
            qWarning() << "URL doesn't point to an image:" << reply->url().toString() << "is" << contentType;
            emit imageCachingFailed(reply->url().toString(), "URL doesn't point to an image: \"" + reply->url().toString() + "\" is \"" + contentType + "\"");
            return;
        }
    }
    const QSettings settings;
    QDir settingsDir = QFileInfo(settings.fileName()).dir();
    if (!settingsDir.mkpath("images") || !settingsDir.cd("images")) {
        qWarning() << "Couldn't create directory for cached images";
        emit imageCachingFailed(reply->url().toString(), "Couldn't create directory for cached images");
        return;
    }
    QString cachedFilePath = replaceImagePath;
    if (replaceImagePath.isNull()) {
        cachedFilePath = settingsDir.absoluteFilePath(reply->url().fileName());
        if (QFileInfo::exists(cachedFilePath)) {
            const qsizetype pointIndex = reply->url().fileName().contains(".") ? cachedFilePath.lastIndexOf('.') : cachedFilePath.length();
            short suffixLength = 2;
            cachedFilePath.insert(pointIndex, "_1");
            for (int i = 2; QFileInfo::exists(cachedFilePath); ++i) {
                QString suffix = QString("_") + QString::number(i);
                cachedFilePath.replace(pointIndex, suffixLength, suffix);
                suffixLength = static_cast<short>(suffix.length());
            }
        }
    }
    QFile cachedFile(cachedFilePath);
    if (!cachedFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for caching images:" << cachedFilePath;
        emit imageCachingFailed(reply->url().toString(), "Failed to open file for caching images: \"" + cachedFilePath + "\"");
        return;
    }
    cachedFile.write(reply->readAll());
    cachedFile.close();
    emit imageCached(reply->url().toString(), QFileInfo(cachedFilePath).fileName(), cachedFilePath);
}

void Connection::deleteCachedImage(const QString& path) {
    QFile cachedFile(path);
    cachedFile.remove();
}
