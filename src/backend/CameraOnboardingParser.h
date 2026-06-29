#pragma once

#include <QString>
#include <QVariantMap>

class CameraOnboardingParser
{
public:
    static QVariantMap parse(const QString &payload);
};
