#pragma once

#include "notepad_exception.h"

#include <QFile>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <algorithm>
#include <set>
#include <string>
#include <vector>

class SpellChecker {
public:
    void loadDictionary(const QString& path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            throw notepad_exception(("Cannot open dictionary: " + path).toStdString());

        QTextStream stream(&file);
        while (!stream.atEnd()) {
            QString word = stream.readLine().trimmed().toLower();
            if (!word.isEmpty())
                m_words.insert(word.toStdString());
        }
    }

    bool isCorrect(const QString& word) const
    {
        QString normalized = normalize(word);
        if (normalized.isEmpty())
            return true;
        return m_words.find(normalized.toStdString()) != m_words.end();
    }

    QStringList suggestions(const QString& word, int limit = 5) const
    {
        QString normalized = normalize(word);
        std::vector<std::pair<int, std::string>> scored;
        scored.reserve(64);
        std::string normalizedWord = normalized.toStdString();
        std::string firstLetter = normalized.left(1).toStdString();

        for (const std::string& candidate : m_words) {
            if (static_cast<int>(scored.size()) >= limit && candidate.rfind(firstLetter, 0) != 0)
                continue;

            int diff = std::abs(static_cast<int>(candidate.size()) - normalized.size());
            if (diff > 2)
                continue;

            if (!normalizedWord.empty() && candidate[0] != normalizedWord[0])
                continue;

            int score = distance(normalizedWord, candidate, 2);
            if (score <= 2)
                scored.emplace_back(score, candidate);
        }

        std::sort(scored.begin(), scored.end(), [](const auto& left, const auto& right) {
            if (left.first == right.first)
                return left.second < right.second;
            return left.first < right.first;
        });

        QStringList result;
        for (const auto& entry : scored) {
            result << QString::fromStdString(entry.second);
            if (result.size() == limit)
                break;
        }
        return result;
    }

    QString firstMisspelled(const QString& text) const
    {
        QRegularExpression expression("\\b[\\p{L}']+\\b");
        QRegularExpressionMatchIterator iterator = expression.globalMatch(text);
        while (iterator.hasNext()) {
            QString word = iterator.next().captured(0);
            if (!isCorrect(word))
                return word;
        }
        return {};
    }

private:
    QString normalize(const QString& word) const
    {
        QString result = word.toLower();
        result.remove(QRegularExpression("^[^\\p{L}]+|[^\\p{L}]+$"));
        return result;
    }

    int distance(const std::string& source, const std::string& target, int maxDistance) const
    {
        std::vector<int> previous(target.size() + 1);
        std::vector<int> current(target.size() + 1);
        for (size_t i = 0; i <= target.size(); ++i)
            previous[i] = static_cast<int>(i);

        for (size_t i = 1; i <= source.size(); ++i) {
            current[0] = static_cast<int>(i);
            int rowMinimum = current[0];
            for (size_t j = 1; j <= target.size(); ++j) {
                int insertCost = current[j - 1] + 1;
                int deleteCost = previous[j] + 1;
                int replaceCost = previous[j - 1] + (source[i - 1] == target[j - 1] ? 0 : 1);
                current[j] = std::min({ insertCost, deleteCost, replaceCost });
                rowMinimum = std::min(rowMinimum, current[j]);
            }
            if (rowMinimum > maxDistance)
                return maxDistance + 1;
            previous.swap(current);
        }

        return previous[target.size()];
    }

    std::set<std::string> m_words;
};
