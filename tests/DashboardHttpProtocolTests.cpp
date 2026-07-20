#include <QtTest>

#include "DashboardHttpProtocol.h"

class DashboardHttpProtocolTests : public QObject
{
    Q_OBJECT

private slots:
    void parsesRequestWithQueryAndBody();
    void waitsForIncompleteBody();
    void rejectsOversizedAndChunkedRequests();
    void extractsCookiesAndBearerToken();
    void validatesSameOriginHeaders();
    void serializesResponseWithoutHeaderInjectionSurface();
    void rejectsInvalidHeaderNamesAndResponseInjection();
};

void DashboardHttpProtocolTests::parsesRequestWithQueryAndBody()
{
    const QByteArray body = R"({"profile":"quick"})";
    const QByteArray request = "POST /api/v1/health/run?source=web HTTP/1.1\r\n"
        "Host: 127.0.0.1:8080\r\nContent-Type: application/json\r\nContent-Length: "
        + QByteArray::number(body.size()) + "\r\n\r\n" + body;
    const auto result = DashboardHttpProtocol::parseRequest(request);
    QVERIFY(result.complete);
    QVERIFY(result.valid);
    QCOMPARE(result.request.method, QByteArray("POST"));
    QCOMPARE(result.request.path, QStringLiteral("/api/v1/health/run"));
    QCOMPARE(result.request.query.queryItemValue(QStringLiteral("source")), QStringLiteral("web"));
    QCOMPARE(result.request.body, body);
    QCOMPARE(result.request.header("HOST"), QByteArray("127.0.0.1:8080"));
}

void DashboardHttpProtocolTests::waitsForIncompleteBody()
{
    const auto result = DashboardHttpProtocol::parseRequest(
        "POST /api HTTP/1.1\r\nContent-Length: 8\r\n\r\n123");
    QVERIFY(!result.complete);
    QVERIFY(!result.valid);
}

void DashboardHttpProtocolTests::rejectsOversizedAndChunkedRequests()
{
    auto result = DashboardHttpProtocol::parseRequest(
        "POST /api HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n");
    QVERIFY(result.complete);
    QVERIFY(!result.valid);
    QCOMPARE(result.status, 415);

    result = DashboardHttpProtocol::parseRequest(
        "POST /api HTTP/1.1\r\nContent-Length: 100\r\n\r\n", 1024, 10);
    QVERIFY(result.complete);
    QCOMPARE(result.status, 413);

    result = DashboardHttpProtocol::parseRequest(
        "POST /api HTTP/1.1\r\nContent-Length: 1\r\nContent-Length: 2\r\n\r\n12");
    QVERIFY(result.complete);
    QVERIFY(!result.valid);
    QCOMPARE(result.status, 400);
}

void DashboardHttpProtocolTests::extractsCookiesAndBearerToken()
{
    const auto cookies = DashboardHttpProtocol::parseCookies(
        "theme=dark; openipc_session=abc_123; language=ru");
    QCOMPARE(cookies.value("openipc_session"), QByteArray("abc_123"));

    DashboardHttpProtocol::Request request;
    request.headers.insert("authorization", "Bearer token-value");
    QCOMPARE(DashboardHttpProtocol::bearerToken(request), QByteArray("token-value"));
}

void DashboardHttpProtocolTests::validatesSameOriginHeaders()
{
    using DashboardHttpProtocol::originMatchesHost;

    QVERIFY(originMatchesHost("http://127.0.0.1:8080", "127.0.0.1:8080"));
    QVERIFY(originMatchesHost("http://localhost:8080", "LOCALHOST:8080"));
    QVERIFY(originMatchesHost("http://[::1]:8080", "[::1]:8080"));
    QVERIFY(originMatchesHost("https://dashboard.local", "dashboard.local:443"));

    QVERIFY(!originMatchesHost("null", "127.0.0.1:8080"));
    QVERIFY(!originMatchesHost("file://127.0.0.1", "127.0.0.1"));
    QVERIFY(!originMatchesHost("http://127.0.0.1:8081", "127.0.0.1:8080"));
    QVERIFY(!originMatchesHost("http://localhost:8080", "127.0.0.1:8080"));
    QVERIFY(!originMatchesHost("http://user@127.0.0.1:8080", "127.0.0.1:8080"));
    QVERIFY(!originMatchesHost("http://127.0.0.1:8080/path", "127.0.0.1:8080"));
}

void DashboardHttpProtocolTests::serializesResponseWithoutHeaderInjectionSurface()
{
    DashboardHttpProtocol::Response response;
    response.body = "{}";
    response.headers.insert("X-Test", "ok");
    const QByteArray bytes = response.serialize();
    QVERIFY(bytes.startsWith("HTTP/1.1 200 OK\r\n"));
    QVERIFY(bytes.contains("Content-Length: 2\r\n"));
    QVERIFY(bytes.endsWith("\r\n\r\n{}"));
}

void DashboardHttpProtocolTests::rejectsInvalidHeaderNamesAndResponseInjection()
{
    auto result = DashboardHttpProtocol::parseRequest(
        "GET / HTTP/1.1\r\nHost: localhost\r\nBad(Name): value\r\n\r\n");
    QVERIFY(result.complete);
    QVERIFY(!result.valid);
    QCOMPARE(result.status, 400);

    DashboardHttpProtocol::Response response;
    response.reason = "OK\r\nX-Injected: yes";
    response.contentType = "text/plain\r\nX-Type-Injected: yes";
    response.headers.insert("X-Safe", "yes");
    response.headers.insert("X-Bad", "yes\r\nX-Injected: yes");
    response.headers.insert("Bad(Name)", "yes");
    const QByteArray serialized = response.serialize();
    QVERIFY(serialized.startsWith("HTTP/1.1 200 OK\r\n"));
    QVERIFY(serialized.contains("Content-Type: application/octet-stream\r\n"));
    QVERIFY(serialized.contains("X-Safe: yes\r\n"));
    QVERIFY(!serialized.contains("X-Injected"));
    QVERIFY(!serialized.contains("Bad(Name)"));
}

QTEST_MAIN(DashboardHttpProtocolTests)
#include "DashboardHttpProtocolTests.moc"
