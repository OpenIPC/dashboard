#include <QtTest>

#include "CameraModel.h"

class CameraModelTests : public QObject
{
    Q_OBJECT

private slots:
    void exposesDiscoveryOnboardingRoles();
};

void CameraModelTests::exposesDiscoveryOnboardingRoles()
{
    CameraModel model;
    Camera camera;
    camera.name = QStringLiteral("openipc-hi3516ev200");
    camera.ip = QStringLiteral("192.168.0.219");
    camera.onboardingProfile = QStringLiteral("openipc");
    camera.validationStatus = QStringLiteral("ok");
    camera.validationMessage = QStringLiteral("Majestic endpoint available");
    camera.alreadyAdded = true;

    model.addCamera(camera);

    const QModelIndex index = model.index(0, 0);
    QVERIFY(index.isValid());
    QCOMPARE(index.data(CameraModel::OnboardingProfileRole).toString(), QStringLiteral("openipc"));
    QCOMPARE(index.data(CameraModel::ValidationStatusRole).toString(), QStringLiteral("ok"));
    QCOMPARE(index.data(CameraModel::ValidationMessageRole).toString(), QStringLiteral("Majestic endpoint available"));
    QCOMPARE(index.data(CameraModel::AlreadyAddedRole).toBool(), true);

    const QHash<int, QByteArray> roles = model.roleNames();
    QCOMPARE(roles.value(CameraModel::OnboardingProfileRole), QByteArray("onboardingProfile"));
    QCOMPARE(roles.value(CameraModel::ValidationStatusRole), QByteArray("validationStatus"));
    QCOMPARE(roles.value(CameraModel::ValidationMessageRole), QByteArray("validationMessage"));
    QCOMPARE(roles.value(CameraModel::AlreadyAddedRole), QByteArray("alreadyAdded"));
}

QTEST_GUILESS_MAIN(CameraModelTests)

#include "CameraModelTests.moc"
