#pragma once

#include <QChar>
#include <QString>

namespace TextTransform {

inline QString uppercase(const QString& text)
{
    return text.toUpper();
}

inline QString lowercase(const QString& text)
{
    return text.toLower();
}

inline QString capitalize(const QString& text)
{
    QString result = text.toLower();
    bool next = true;
    for (QChar& ch : result) {
        if (ch.isLetter()) {
            if (next)
                ch = ch.toUpper();
            next = false;
        } else if (ch.isSpace()) {
            next = true;
        }
    }
    return result;
}

inline QString sentenceCase(const QString& text)
{
    QString result = text.toLower();
    bool next = true;
    for (QChar& ch : result) {
        if (ch.isLetter()) {
            if (next) {
                ch = ch.toUpper();
                next = false;
            }
        } else if (ch == '.' || ch == '!' || ch == '?') {
            next = true;
        }
    }
    return result;
}

inline QString swapCase(const QString& text)
{
    QString result = text;
    for (QChar& ch : result) {
        if (ch.isLower())
            ch = ch.toUpper();
        else if (ch.isUpper())
            ch = ch.toLower();
    }
    return result;
}

}
