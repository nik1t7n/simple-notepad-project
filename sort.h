#pragma once

#include <QString>
#include <QStringList>

#include <algorithm>

namespace TextSort {

inline QString sortLines(const QString& text)
{
    QStringList lines = text.split('\n');
    std::sort(lines.begin(), lines.end(), [](const QString& left, const QString& right) {
        return QString::localeAwareCompare(left, right) < 0;
    });
    return lines.join('\n');
}

}
