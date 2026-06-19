#include <QtTest>

#include <QDir>

#include "PathUtils.h"

class PathUtilsTests : public QObject
{
    Q_OBJECT

private slots:
    void localFileUrlPreservesLinuxRoot();
    void localFileUrlDecodesSpaces();
    void relativePathStaysRelative();
    void tildePathExpandsToHome();
#ifdef Q_OS_WIN
    void windowsDriveFileUrlPreservesDrive();
#endif
};

void PathUtilsTests::localFileUrlPreservesLinuxRoot()
{
    QCOMPARE(PathUtils::localPathFromUserInput(QStringLiteral("file:///mnt/video")),
             QDir::cleanPath(QStringLiteral("/mnt/video")));
}

void PathUtilsTests::localFileUrlDecodesSpaces()
{
    QCOMPARE(PathUtils::localPathFromUserInput(QStringLiteral("file:///mnt/video%20archive")),
             QDir::cleanPath(QStringLiteral("/mnt/video archive")));
}

void PathUtilsTests::relativePathStaysRelative()
{
    QCOMPARE(PathUtils::localPathFromUserInput(QStringLiteral("relative/video")),
             QDir::cleanPath(QStringLiteral("relative/video")));
}

void PathUtilsTests::tildePathExpandsToHome()
{
    QCOMPARE(PathUtils::localPathFromUserInput(QStringLiteral("~/OpenIPC")),
             QDir::cleanPath(QDir::home().filePath(QStringLiteral("OpenIPC"))));
}

#ifdef Q_OS_WIN
void PathUtilsTests::windowsDriveFileUrlPreservesDrive()
{
    QCOMPARE(PathUtils::localPathFromUserInput(QStringLiteral("file:///C:/Users/Test/Videos")),
             QDir::cleanPath(QStringLiteral("C:/Users/Test/Videos")));
}
#endif

QTEST_APPLESS_MAIN(PathUtilsTests)

#include "PathUtilsTests.moc"
