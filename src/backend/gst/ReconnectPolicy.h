#pragma once

#include <QString>

class ReconnectPolicy
{
public:
    static int delayMs(int attempt);
    static bool isAuthenticationError(const QString &message, const QString &debug = {});
};
