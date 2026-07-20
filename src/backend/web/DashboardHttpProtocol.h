#pragma once

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QUrlQuery>

namespace DashboardHttpProtocol {

struct Request {
    QByteArray method;
    QString path;
    QUrlQuery query;
    QHash<QByteArray, QByteArray> headers;
    QByteArray body;

    QByteArray header(const QByteArray &name) const;
};

struct ParseResult {
    bool complete = false;
    bool valid = false;
    int status = 400;
    QString error;
    Request request;
};

struct Response {
    int status = 200;
    QByteArray reason = "OK";
    QByteArray contentType = "application/json; charset=utf-8";
    QHash<QByteArray, QByteArray> headers;
    QByteArray body;

    QByteArray serialize(bool includeBody = true) const;
};

ParseResult parseRequest(const QByteArray &buffer,
                         qsizetype maxHeaderBytes = 32 * 1024,
                         qsizetype maxBodyBytes = 1024 * 1024);
QHash<QByteArray, QByteArray> parseCookies(const QByteArray &header);
QByteArray bearerToken(const Request &request);
bool originMatchesHost(const QByteArray &originHeader, const QByteArray &hostHeader);
QByteArray statusReason(int status);

} // namespace DashboardHttpProtocol
