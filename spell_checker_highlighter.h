#pragma once

#include "spell_checker.h"

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class SpellCheckerHighlighter : public QSyntaxHighlighter {
public:
    SpellCheckerHighlighter(QTextDocument* document, const SpellChecker* spellChecker)
        : QSyntaxHighlighter(document)
        , m_spellChecker(spellChecker)
    {
        m_spellFormat.setUnderlineColor(Qt::red);
        m_spellFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

        m_keywordFormat.setForeground(QColor(0, 92, 175));
        m_keywordFormat.setFontWeight(QFont::Bold);
        const QStringList keywords = {
            "alignas", "alignof", "and", "asm", "auto", "bool", "break", "case", "catch",
            "char", "class", "const", "constexpr", "continue", "decltype", "default", "delete",
            "do", "double", "else", "enum", "explicit", "export", "extern", "false", "float",
            "for", "friend", "goto", "if", "inline", "int", "long", "namespace", "new",
            "noexcept", "nullptr", "operator", "or", "private", "protected", "public", "return",
            "short", "signed", "sizeof", "static", "struct", "switch", "template", "this",
            "throw", "true", "try", "typedef", "typename", "union", "unsigned", "using",
            "virtual", "void", "volatile", "while"
        };
        for (const QString& keyword : keywords)
            m_keywordPatterns.append(QRegularExpression("\\b" + keyword + "\\b"));
    }

protected:
    void highlightBlock(const QString& text) override
    {
        for (const QRegularExpression& pattern : m_keywordPatterns) {
            QRegularExpressionMatchIterator iterator = pattern.globalMatch(text);
            while (iterator.hasNext()) {
                QRegularExpressionMatch match = iterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), m_keywordFormat);
            }
        }

        QRegularExpression wordExpression("\\b[\\p{L}']+\\b");
        QRegularExpressionMatchIterator iterator = wordExpression.globalMatch(text);
        while (iterator.hasNext()) {
            QRegularExpressionMatch match = iterator.next();
            QString word = match.captured(0);
            if (m_spellChecker && !m_spellChecker->isCorrect(word))
                setFormat(match.capturedStart(), match.capturedLength(), m_spellFormat);
        }
    }

private:
    const SpellChecker* m_spellChecker;
    QTextCharFormat m_spellFormat;
    QTextCharFormat m_keywordFormat;
    QList<QRegularExpression> m_keywordPatterns;
};
