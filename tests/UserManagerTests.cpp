#include <QtTest>

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QStandardPaths>

#include "UserManager.h"

namespace {
constexpr int kExpectedMinIterations = 100000;

QString legacyHash(const QString &password)
{
    return QString::fromLatin1(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}
}

class UserManagerTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    void noDefaultAdminOnFirstRun();
    void setupInitialAdminPersistsSecureHashWithoutAutoLogin();
    void legacyPasswordIsUpgradedAndAdminPermissionsAreRepaired();
    void loginWithoutRememberMeClearsStoredUsername();

private:
    QString m_settingsRoot;

    QString usersFilePath() const;
    QString appConfigRoot() const;
    QString appDataRoot() const;
    void resetStorage() const;
    QJsonArray loadUsers() const;
    void writeUsers(const QJsonArray &users) const;
};

void UserManagerTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("OpenIPC"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("openipc.test"));
    QCoreApplication::setApplicationName(QStringLiteral("UserManagerTests"));
    QStandardPaths::setTestModeEnabled(true);

    m_settingsRoot = QDir::cleanPath(QDir::tempPath() + "/OpenIPC-UserManagerTests-settings");
    QDir(m_settingsRoot).removeRecursively();
    QDir().mkpath(m_settingsRoot);

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settingsRoot);

    resetStorage();
}

void UserManagerTests::init()
{
    resetStorage();
}

void UserManagerTests::noDefaultAdminOnFirstRun()
{
    UserManager manager;

    QVERIFY(!manager.hasUsers());
    QVERIFY(manager.users().isEmpty());
    QVERIFY(!manager.isLoggedIn());
    QVERIFY(manager.rememberedUsername().isEmpty());
    QVERIFY(!manager.login(QStringLiteral("admin"), QStringLiteral("admin"), false));
}

void UserManagerTests::setupInitialAdminPersistsSecureHashWithoutAutoLogin()
{
    {
        UserManager manager;
        QVERIFY(manager.setupInitialAdmin(QStringLiteral("admin"), QStringLiteral("Str0ngP@ss!"), true));
        QVERIFY(manager.hasUsers());
        QVERIFY(manager.isLoggedIn());
        QCOMPARE(manager.rememberedUsername(), QStringLiteral("admin"));

        const QJsonArray users = loadUsers();
        QCOMPARE(users.size(), 1);

        const QJsonObject admin = users.at(0).toObject();
        QCOMPARE(admin.value(QStringLiteral("username")).toString(), QStringLiteral("admin"));
        QCOMPARE(admin.value(QStringLiteral("role")).toString(), QStringLiteral("admin"));
        QCOMPARE(admin.value(QStringLiteral("permissions")).toInt(), int(UserManager::Perm_All));
        QVERIFY(!admin.contains(QStringLiteral("password")));
        QVERIFY(!admin.value(QStringLiteral("passwordHash")).toString().isEmpty());
        QCOMPARE(admin.value(QStringLiteral("passwordAlgorithm")).toString(), QStringLiteral("pbkdf2-sha256"));
        QVERIFY(!admin.value(QStringLiteral("passwordSalt")).toString().isEmpty());
        QVERIFY(admin.value(QStringLiteral("passwordIterations")).toInt() >= kExpectedMinIterations);

        QSettings settings;
        QVERIFY(settings.value(QStringLiteral("auth/remember"), false).toBool());
        QCOMPARE(settings.value(QStringLiteral("auth/username")).toString(), QStringLiteral("admin"));
    }

    UserManager restarted;
    QVERIFY(restarted.hasUsers());
    QVERIFY(!restarted.isLoggedIn());
    QCOMPARE(restarted.rememberedUsername(), QStringLiteral("admin"));
}

void UserManagerTests::legacyPasswordIsUpgradedAndAdminPermissionsAreRepaired()
{
    const QString password = QStringLiteral("LegacyPass123");
    writeUsers(QJsonArray{
        QJsonObject{
            {QStringLiteral("username"), QStringLiteral("admin")},
            {QStringLiteral("passwordHash"), legacyHash(password)},
            {QStringLiteral("role"), QStringLiteral("admin")},
            {QStringLiteral("permissions"), 0}
        }
    });

    UserManager manager;
    QVERIFY(manager.login(QStringLiteral("admin"), password, false));
    QVERIFY(manager.isLoggedIn());
    QCOMPARE(manager.currentPermissions(), int(UserManager::Perm_All));
    QVERIFY(manager.rememberedUsername().isEmpty());

    const QJsonArray users = loadUsers();
    QCOMPARE(users.size(), 1);

    const QJsonObject upgraded = users.at(0).toObject();
    QCOMPARE(upgraded.value(QStringLiteral("permissions")).toInt(), int(UserManager::Perm_All));
    QCOMPARE(upgraded.value(QStringLiteral("passwordAlgorithm")).toString(), QStringLiteral("pbkdf2-sha256"));
    QVERIFY(!upgraded.value(QStringLiteral("passwordSalt")).toString().isEmpty());
    QVERIFY(upgraded.value(QStringLiteral("passwordIterations")).toInt() >= kExpectedMinIterations);
    QVERIFY(upgraded.value(QStringLiteral("passwordHash")).toString() != legacyHash(password));

    QSettings settings;
    QVERIFY(!settings.value(QStringLiteral("auth/remember"), false).toBool());
    QCOMPARE(settings.value(QStringLiteral("auth/username")).toString(), QString());
}

void UserManagerTests::loginWithoutRememberMeClearsStoredUsername()
{
    {
        UserManager manager;
        QVERIFY(manager.setupInitialAdmin(QStringLiteral("operator"), QStringLiteral("AnotherStr0ngP@ss"), true));
        QCOMPARE(manager.rememberedUsername(), QStringLiteral("operator"));
        manager.logout();
        QVERIFY(manager.login(QStringLiteral("operator"), QStringLiteral("AnotherStr0ngP@ss"), false));
        QVERIFY(manager.rememberedUsername().isEmpty());

        QSettings settings;
        QVERIFY(!settings.value(QStringLiteral("auth/remember"), false).toBool());
        QCOMPARE(settings.value(QStringLiteral("auth/username")).toString(), QString());
    }

    UserManager restarted;
    QVERIFY(restarted.hasUsers());
    QVERIFY(!restarted.isLoggedIn());
    QVERIFY(restarted.rememberedUsername().isEmpty());
}

QString UserManagerTests::usersFilePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QStringLiteral("/users.json");
}

QString UserManagerTests::appConfigRoot() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QString UserManagerTests::appDataRoot() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

void UserManagerTests::resetStorage() const
{
    QSettings settings;
    settings.clear();
    settings.sync();

    const QStringList paths{appConfigRoot(), appDataRoot(), m_settingsRoot};
    for (const QString &path : paths) {
        if (path.isEmpty()) {
            continue;
        }

        QDir dir(path);
        if (dir.exists()) {
            QVERIFY2(dir.removeRecursively(), qPrintable(QStringLiteral("Failed to remove path: %1").arg(path)));
        }
    }

    QDir().mkpath(m_settingsRoot);
}

QJsonArray UserManagerTests::loadUsers() const
{
    QFile file(usersFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        QTest::qFail(qPrintable(QStringLiteral("Failed to open users file: %1").arg(usersFilePath())), __FILE__, __LINE__);
        return {};
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isArray()) {
        QTest::qFail("Users document is not a JSON array", __FILE__, __LINE__);
        return {};
    }

    return document.array();
}

void UserManagerTests::writeUsers(const QJsonArray &users) const
{
    const QString path = usersFilePath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QFile file(path);
    QVERIFY2(file.open(QIODevice::WriteOnly | QIODevice::Truncate), qPrintable(QStringLiteral("Failed to write users file: %1").arg(path)));
    file.write(QJsonDocument(users).toJson(QJsonDocument::Compact));
    file.close();
}

QTEST_APPLESS_MAIN(UserManagerTests)

#include "UserManagerTests.moc"
