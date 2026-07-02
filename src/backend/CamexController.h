#ifndef CAMEXCONTROLLER_H
#define CAMEXCONTROLLER_H

#include <QObject>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

class CamexController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use SystemController.camexController")

public:
    explicit CamexController(QObject *parent = nullptr);

    Q_INVOKABLE QString normalizeClientId(const QString &value) const;
    Q_INVOKABLE QString buildServerCommand(const QVariantMap &settings) const;
    Q_INVOKABLE QString buildClientCommand(const QVariantMap &settings) const;
    Q_INVOKABLE QString buildServerConfig(const QVariantMap &settings) const;
    Q_INVOKABLE bool saveTextFile(const QString &pathOrUrl, const QString &content) const;
    Q_INVOKABLE QVariantMap checkTcpPort(const QString &host, int port, int timeoutMs = 1500) const;

private:
    static QString shellQuote(const QString &value);
    static QString optionValue(const QVariantMap &settings, const QString &key, const QString &fallback = QString());
    static int optionInt(const QVariantMap &settings, const QString &key, int fallback);
    static bool optionBool(const QVariantMap &settings, const QString &key, bool fallback = false);
    static QStringList routeList(const QVariant &value);
};

#endif // CAMEXCONTROLLER_H
