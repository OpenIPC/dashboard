#ifndef STREAMQUALITYPOLICY_H
#define STREAMQUALITYPOLICY_H

#include <QString>

class StreamQualityPolicy
{
public:
    enum class Quality {
        Main,
        Sub
    };

    static Quality resolvePreviewQuality(const QString &preferredStream,
                                         int gridRows,
                                         int gridCols,
                                         int spanRows,
                                         int spanCols,
                                         bool forceMain);

    static QString selectPreviewUrl(const QString &streamUrl,
                                    const QString &sdStreamUrl,
                                    const QString &hdStreamUrl,
                                    const QString &preferredStream,
                                    int gridRows,
                                    int gridCols,
                                    int spanRows,
                                    int spanCols,
                                    bool forceMain);

    static QString selectManualUrl(const QString &streamUrl,
                                   const QString &sdStreamUrl,
                                   const QString &hdStreamUrl,
                                   bool preferMain);

    static QString qualityLabel(Quality quality);
};

#endif // STREAMQUALITYPOLICY_H
