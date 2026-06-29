#include "PathUtils.h"

#include <QDir>
#include <QUrl>

namespace {

#ifndef Q_OS_WIN
bool isWindowsDrivePath(const QString &path)
{
    return path.size() >= 2 && path.at(1) == QLatin1Char(':') && path.at(0).isLetter();
}
#endif

#ifdef Q_OS_LINUX
bool isLikelyLinuxAbsoluteWithoutSlash(const QString &path)
{
    return path.startsWith(QStringLiteral("mnt/"))
        || path.startsWith(QStringLiteral("media/"))
        || path.startsWith(QStringLiteral("run/media/"));
}
#endif

QString decodePercentEncoding(const QString &path)
{
    return QUrl::fromPercentEncoding(path.toUtf8());
}

} // namespace

QString PathUtils::localPathFromUserInput(const QString &pathOrUrl)
{
    QString raw = pathOrUrl.trimmed();
    if (raw.isEmpty()) {
        return QString();
    }

    QString localPath;
    const QUrl url(raw);
    if (url.isLocalFile()) {
        localPath = url.toLocalFile();
    } else if (raw.startsWith(QStringLiteral("file:"), Qt::CaseInsensitive)) {
        if (raw.startsWith(QStringLiteral("file:///"), Qt::CaseInsensitive)) {
            localPath = raw.mid(8);
#ifndef Q_OS_WIN
            if (!isWindowsDrivePath(localPath) && !localPath.startsWith(QLatin1Char('/'))) {
                localPath.prepend(QLatin1Char('/'));
            }
#endif
        } else if (raw.startsWith(QStringLiteral("file://"), Qt::CaseInsensitive)) {
            localPath = raw.mid(7);
#ifndef Q_OS_WIN
            if (!isWindowsDrivePath(localPath) && !localPath.startsWith(QLatin1Char('/'))) {
                localPath.prepend(QLatin1Char('/'));
            }
#endif
        } else {
            localPath = raw.mid(5);
        }
    } else {
        localPath = raw;
    }

    localPath = decodePercentEncoding(localPath);
    localPath = QDir::fromNativeSeparators(localPath);

#ifdef Q_OS_WIN
    if (localPath.size() >= 3
        && localPath.at(0) == QLatin1Char('/')
        && localPath.at(2) == QLatin1Char(':')
        && localPath.at(1).isLetter()) {
        localPath = localPath.mid(1);
    }
#endif

    if (localPath == QStringLiteral("~")) {
        localPath = QDir::homePath();
    } else if (localPath.startsWith(QStringLiteral("~/")) || localPath.startsWith(QStringLiteral("~\\"))) {
        localPath = QDir::home().filePath(localPath.mid(2));
    }

#ifdef Q_OS_LINUX
    if (!QDir::isAbsolutePath(localPath) && isLikelyLinuxAbsoluteWithoutSlash(localPath)) {
        localPath.prepend(QLatin1Char('/'));
    }
#endif

    return QDir::cleanPath(localPath);
}
