#include <QtTest>

#include <QFile>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>

#ifndef I18N_QML_PATH
#error "I18N_QML_PATH must point to src/ui/I18n.qml"
#endif

class I18nCatalogTests : public QObject
{
    Q_OBJECT

private slots:
    void hasNoDuplicateKeys();
    void hasNoMojibakeMarkers();

private:
    QStringList catalogLines(const QString &catalogName) const;
};

QStringList I18nCatalogTests::catalogLines(const QString &catalogName) const
{
    QFile file(QString::fromUtf8(I18N_QML_PATH));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTest::qFail(qPrintable(QStringLiteral("Unable to open I18n.qml: %1").arg(file.errorString())),
                     __FILE__,
                     __LINE__);
        return {};
    }

    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char('\n'));
    const QString startNeedle = QStringLiteral("readonly property var %1").arg(catalogName);
    int start = -1;
    for (int i = 0; i < lines.size(); ++i) {
        if (lines.at(i).contains(startNeedle)) {
            start = i + 1;
            break;
        }
    }
    if (start < 0) {
        QTest::qFail(qPrintable(QStringLiteral("Catalog not found: %1").arg(catalogName)),
                     __FILE__,
                     __LINE__);
        return {};
    }

    int end = lines.size();
    for (int i = start; i < lines.size(); ++i) {
        if (lines.at(i).contains(QStringLiteral("readonly property var ")) || lines.at(i).contains(QStringLiteral("function t("))) {
            end = i;
            break;
        }
    }

    return lines.mid(start, end - start);
}

void I18nCatalogTests::hasNoDuplicateKeys()
{
    const QRegularExpression keyExpression(QStringLiteral("^\\s*\"((?:[^\"\\\\]|\\\\.)+)\"\\s*:"));

    for (const QString &catalogName : {QStringLiteral("trMapEn"), QStringLiteral("trMapRu")}) {
        QSet<QString> seenKeys;
        QStringList duplicates;

        for (const QString &line : catalogLines(catalogName)) {
            const QRegularExpressionMatch match = keyExpression.match(line);
            if (!match.hasMatch()) {
                continue;
            }

            const QString key = match.captured(1);
            if (seenKeys.contains(key)) {
                duplicates.append(key);
            } else {
                seenKeys.insert(key);
            }
        }

        QVERIFY2(duplicates.isEmpty(),
                 qPrintable(QStringLiteral("%1 contains duplicate keys: %2")
                                .arg(catalogName, duplicates.join(QStringLiteral(", ")))));
    }
}

void I18nCatalogTests::hasNoMojibakeMarkers()
{
    QFile file(QString::fromUtf8(I18N_QML_PATH));
    QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(file.errorString()));
    const QByteArray content = file.readAll();

    const QList<QByteArray> markers = {
        QByteArray::fromHex("c383"),
        QByteArray::fromHex("c390"),
        QByteArray::fromHex("c391"),
        QByteArray::fromHex("c3a2"),
        QByteArray::fromHex("efbfbd")
    };

    for (const QByteArray &marker : markers) {
        QVERIFY2(!content.contains(marker), "I18n.qml contains likely mojibake markers");
    }
}

QTEST_APPLESS_MAIN(I18nCatalogTests)

#include "I18nCatalogTests.moc"
