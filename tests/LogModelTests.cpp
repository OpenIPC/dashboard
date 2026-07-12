#include <QtTest>

#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>

#include "LogModel.h"

class LogModelTests : public QObject
{
    Q_OBJECT

private slots:
    void loadFromFileKeepsTailAndParsesLevels();
    void addLogIsCappedAndExportable();
};

void LogModelTests::loadFromFileKeepsTailAndParsesLevels()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.filePath(QStringLiteral("app.log"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&file);
    for (int i = 0; i < 2005; ++i) {
        out << "2026-07-12T10:00:00.000 [WRN] warning " << i << '\n';
    }
    file.close();

    LogModel model;
    QVERIFY(model.loadFromFile(path));
    QCOMPARE(model.sourcePath(), path);
    QCOMPARE(model.count(), 2000);
    QCOMPARE(model.rowCount(), 2000);

    const QModelIndex first = model.index(0, 0);
    QCOMPARE(model.data(first, LogModel::MessageRole).toString(), QStringLiteral("warning 5"));
    QCOMPARE(model.data(first, LogModel::LevelStringRole).toString(), QStringLiteral("WRN"));
    QCOMPARE(model.data(first, LogModel::TypeRole).toInt(), static_cast<int>(QtWarningMsg));
}

void LogModelTests::addLogIsCappedAndExportable()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    LogModel model;
    for (int i = 0; i < 2001; ++i) {
        model.addLog(QtInfoMsg, QStringLiteral("live %1").arg(i));
    }

    QCOMPARE(model.count(), 2000);
    QCOMPARE(model.data(model.index(0, 0), LogModel::MessageRole).toString(), QStringLiteral("live 1"));

    const QString exportPath = dir.filePath(QStringLiteral("export.txt"));
    model.saveLog(exportPath);

    QFile exported(exportPath);
    QVERIFY(exported.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString text = QString::fromUtf8(exported.readAll());
    QVERIFY(!text.contains(QStringLiteral("live 0")));
    QVERIFY(text.contains(QStringLiteral("live 2000")));
}

QTEST_APPLESS_MAIN(LogModelTests)

#include "LogModelTests.moc"
