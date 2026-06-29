#pragma once

#include <QString>

class ModelArtifactVerifier
{
public:
    static bool verify(const QString &path, const QString &expectedSha256,
                       qint64 expectedSize, QString *errorMessage = nullptr);
    static bool promote(const QString &partialPath, const QString &destinationPath,
                        QString *errorMessage = nullptr);
};
