#pragma once

#include <QDir>
#include <QStandardPaths>
#include <QString>

namespace AppPaths {

inline QString runtimeRoot()
{
    const QString configured = qEnvironmentVariable("OPENIPC_DATA_ROOT").trimmed();
    if (configured.isEmpty()) {
        return {};
    }
    return QDir::cleanPath(QDir(configured).absolutePath());
}

inline QString dataDirectory()
{
    const QString root = runtimeRoot();
    return root.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        : QDir(root).filePath(QStringLiteral("data"));
}

inline QString configDirectory()
{
    const QString root = runtimeRoot();
    return root.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
        : QDir(root).filePath(QStringLiteral("config"));
}

inline QString evidenceDirectory(const QString &kind)
{
    const QString root = runtimeRoot();
    return root.isEmpty() ? QString() : QDir(root).filePath(QStringLiteral("evidence/") + kind);
}

} // namespace AppPaths
