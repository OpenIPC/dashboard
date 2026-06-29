#include <QtTest>

#include "ModelArtifactVerifier.h"

#include <QCryptographicHash>
#include <QFile>
#include <QTemporaryDir>

class ModelArtifactVerifierTests : public QObject
{
    Q_OBJECT

private slots:
    void acceptsExpectedArtifactAndRejectsTampering();
    void promotesVerifiedDownloadOverPreviousVersion();
};

void ModelArtifactVerifierTests::acceptsExpectedArtifactAndRejectsTampering()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("model.onnx.part"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write("verified-model"), qint64{14});
    file.close();

    const QString hash = QString::fromLatin1(
        QCryptographicHash::hash(QByteArrayLiteral("verified-model"), QCryptographicHash::Sha256).toHex());
    QString error;
    QVERIFY2(ModelArtifactVerifier::verify(path, hash, 14, &error), qPrintable(error));

    QVERIFY(file.open(QIODevice::Append));
    QCOMPARE(file.write("!"), qint64{1});
    file.close();
    QVERIFY(!ModelArtifactVerifier::verify(path, hash, 14, &error));
}

void ModelArtifactVerifierTests::promotesVerifiedDownloadOverPreviousVersion()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString destination = directory.filePath(QStringLiteral("model.onnx"));
    const QString partial = destination + QStringLiteral(".part");

    QFile oldFile(destination);
    QVERIFY(oldFile.open(QIODevice::WriteOnly));
    oldFile.write("old");
    oldFile.close();
    QFile newFile(partial);
    QVERIFY(newFile.open(QIODevice::WriteOnly));
    newFile.write("new");
    newFile.close();

    QString error;
    QVERIFY2(ModelArtifactVerifier::promote(partial, destination, &error), qPrintable(error));
    QFile installed(destination);
    QVERIFY(installed.open(QIODevice::ReadOnly));
    QCOMPARE(installed.readAll(), QByteArrayLiteral("new"));
    QVERIFY(!QFile::exists(destination + QStringLiteral(".previous")));
}

QTEST_APPLESS_MAIN(ModelArtifactVerifierTests)

#include "ModelArtifactVerifierTests.moc"
