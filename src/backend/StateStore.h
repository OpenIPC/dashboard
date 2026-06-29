#pragma once

#include <QJsonObject>
#include <QString>

#include <optional>

class QSqlDatabase;

class StateStore
{
public:
    explicit StateStore(QString databasePath);

    std::optional<QJsonObject> load(QString *errorMessage = nullptr) const;
    bool save(const QJsonObject &state, QString *errorMessage = nullptr) const;

    static constexpr int currentSchemaVersion() { return 1; }

private:
    bool ensureSchema(QSqlDatabase &database, QString *errorMessage) const;
    QString connectionName() const;

    QString m_databasePath;
};
