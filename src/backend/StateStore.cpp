#include "StateStore.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace {

void setError(QString *destination, const QString &message)
{
    if (destination) *destination = message;
}

} // namespace

StateStore::StateStore(QString databasePath)
    : m_databasePath(std::move(databasePath))
{
}

QString StateStore::connectionName() const
{
    const QByteArray digest = QCryptographicHash::hash(
        m_databasePath.toUtf8(), QCryptographicHash::Sha256).toHex().left(16);
    return QStringLiteral("openipc-state-%1").arg(QString::fromLatin1(digest));
}

bool StateStore::ensureSchema(QSqlDatabase &database, QString *errorMessage) const
{
    QSqlQuery query(database);
    if (!query.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS schema_migrations ("
            "version INTEGER PRIMARY KEY, applied_at TEXT NOT NULL)"))) {
        setError(errorMessage, query.lastError().text());
        return false;
    }

    int version = 0;
    if (!query.exec(QStringLiteral("SELECT COALESCE(MAX(version), 0) FROM schema_migrations"))
        || !query.next()) {
        setError(errorMessage, query.lastError().text());
        return false;
    }
    version = query.value(0).toInt();

    if (version > currentSchemaVersion()) {
        setError(errorMessage, QStringLiteral("State database schema %1 is newer than supported schema %2")
                                   .arg(version)
                                   .arg(currentSchemaVersion()));
        return false;
    }

    if (version < 1) {
        if (!database.transaction()) {
            setError(errorMessage, database.lastError().text());
            return false;
        }

        const bool tableCreated = query.exec(QStringLiteral(
            "CREATE TABLE app_state ("
            "id INTEGER PRIMARY KEY CHECK(id = 1),"
            "schema_version INTEGER NOT NULL,"
            "payload_json TEXT NOT NULL,"
            "updated_at TEXT NOT NULL)"));
        if (!tableCreated) {
            database.rollback();
            setError(errorMessage, query.lastError().text());
            return false;
        }

        query.prepare(QStringLiteral(
            "INSERT INTO schema_migrations(version, applied_at) VALUES(?, ?)"));
        query.addBindValue(1);
        query.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
        if (!query.exec() || !database.commit()) {
            const QString error = query.lastError().text().isEmpty()
                ? database.lastError().text() : query.lastError().text();
            database.rollback();
            setError(errorMessage, error);
            return false;
        }
    }

    return true;
}

std::optional<QJsonObject> StateStore::load(QString *errorMessage) const
{
    const QString name = connectionName();
    std::optional<QJsonObject> result;

    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), name);
        database.setDatabaseName(m_databasePath);
        if (!database.open()) {
            setError(errorMessage, database.lastError().text());
        } else if (ensureSchema(database, errorMessage)) {
            QSqlQuery query(database);
            if (!query.exec(QStringLiteral("SELECT payload_json FROM app_state WHERE id = 1"))) {
                setError(errorMessage, query.lastError().text());
            } else if (query.next()) {
                QJsonParseError parseError;
                const QJsonDocument document = QJsonDocument::fromJson(
                    query.value(0).toString().toUtf8(), &parseError);
                if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
                    setError(errorMessage, QStringLiteral("Invalid state payload: %1")
                                               .arg(parseError.errorString()));
                } else {
                    result = document.object();
                }
            }
        }
        database.close();
    }
    QSqlDatabase::removeDatabase(name);
    return result;
}

bool StateStore::save(const QJsonObject &state, QString *errorMessage) const
{
    const QFileInfo databaseInfo(m_databasePath);
    if (!QDir().mkpath(databaseInfo.absolutePath())) {
        setError(errorMessage, QStringLiteral("Unable to create state directory"));
        return false;
    }

    const QString name = connectionName();
    bool saved = false;
    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), name);
        database.setDatabaseName(m_databasePath);
        if (!database.open()) {
            setError(errorMessage, database.lastError().text());
        } else if (ensureSchema(database, errorMessage) && database.transaction()) {
            QSqlQuery query(database);
            query.prepare(QStringLiteral(
                "INSERT INTO app_state(id, schema_version, payload_json, updated_at) "
                "VALUES(1, ?, ?, ?) "
                "ON CONFLICT(id) DO UPDATE SET "
                "schema_version=excluded.schema_version, "
                "payload_json=excluded.payload_json, updated_at=excluded.updated_at"));
            query.addBindValue(currentSchemaVersion());
            query.addBindValue(QString::fromUtf8(
                QJsonDocument(state).toJson(QJsonDocument::Compact)));
            query.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

            if (query.exec() && database.commit()) {
                saved = true;
            } else {
                const QString error = query.lastError().text().isEmpty()
                    ? database.lastError().text() : query.lastError().text();
                database.rollback();
                setError(errorMessage, error);
            }
        } else if (database.isOpen() && errorMessage && errorMessage->isEmpty()) {
            setError(errorMessage, database.lastError().text());
        }
        database.close();
    }
    QSqlDatabase::removeDatabase(name);
    return saved;
}
