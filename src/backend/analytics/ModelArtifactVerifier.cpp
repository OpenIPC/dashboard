#include "ModelArtifactVerifier.h"

#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>

namespace {

void setError(QString *destination, const QString &message)
{
    if (destination) *destination = message;
}

} // namespace

bool ModelArtifactVerifier::verify(const QString &path, const QString &expectedSha256,
                                   qint64 expectedSize, QString *errorMessage)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(errorMessage, QStringLiteral("Cannot open artifact: %1").arg(file.errorString()));
        return false;
    }

    if (expectedSize > 0 && file.size() != expectedSize) {
        setError(errorMessage, QStringLiteral("Size mismatch: expected %1 bytes, received %2")
                                   .arg(expectedSize)
                                   .arg(file.size()));
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        setError(errorMessage, QStringLiteral("Cannot calculate SHA-256"));
        return false;
    }

    const QString actualHash = QString::fromLatin1(hash.result().toHex());
    if (actualHash.compare(expectedSha256, Qt::CaseInsensitive) != 0) {
        setError(errorMessage, QStringLiteral("SHA-256 mismatch: expected %1, received %2")
                                   .arg(expectedSha256, actualHash));
        return false;
    }
    return true;
}

bool ModelArtifactVerifier::promote(const QString &partialPath, const QString &destinationPath,
                                    QString *errorMessage)
{
    const QString backupPath = destinationPath + QStringLiteral(".previous");
    QFile::remove(backupPath);

    const bool hadDestination = QFile::exists(destinationPath);
    if (hadDestination && !QFile::rename(destinationPath, backupPath)) {
        setError(errorMessage, QStringLiteral("Cannot back up the installed model"));
        return false;
    }

    if (!QFile::rename(partialPath, destinationPath)) {
        if (hadDestination) QFile::rename(backupPath, destinationPath);
        setError(errorMessage, QStringLiteral("Cannot atomically install the verified model"));
        return false;
    }

    QFile::remove(backupPath);
    return true;
}
