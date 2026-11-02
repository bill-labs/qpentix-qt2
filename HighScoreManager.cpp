#include "HighScoreManager.h"
#include <qfile.h>
#include <qtextstream.h>
#include <qdir.h>
#include <stdlib.h>

HighScoreManager::HighScoreManager()
{
    scoresFilePath = getDefaultScoresPath();
    loadScores();
}

HighScoreManager::~HighScoreManager()
{
    saveScores();
}

QString HighScoreManager::getDefaultScoresPath() const
{
    // Get home directory
    const char *home = getenv("HOME");
    QString homePath = home ? QString(home) : QString("/tmp");

    return homePath + "/.qpentix_scores";
}

bool HighScoreManager::loadScores()
{
    scores.clear();

    QFile file(scoresFilePath);
    if (!file.exists()) {
        return false;  // No scores file yet, that is fine
    }

    if (!file.open(IO_ReadOnly)) {
        return false;
    }

    QTextStream stream(&file);
    QString line;

    while (!stream.atEnd()) {
        line = stream.readLine();

        // Skip comments and empty lines
        if (line.isEmpty() || line[0] == '#') {
            continue;
        }

        // Parse line: Name|Score|Level|Lines|Date
        // Manual parsing since QStringList::split doesn't exist in Qt2
        int pos = 0;
        QString parts[5];
        int partIndex = 0;
        for (uint i = 0; i < line.length() && partIndex < 5; i++) {
            if (line[i] == '|') {
                parts[partIndex++] = line.mid(pos, i - pos);
                pos = i + 1;
            }
        }
        if (partIndex == 4) {
            parts[4] = line.mid(pos);  // Last part
        }

        if (partIndex != 4) {
            continue;  // Invalid line, skip
        }

        HighScoreEntry entry;
        entry.name = parts[0];
        entry.score = parts[1].toLong();
        entry.level = parts[2].toInt();
        entry.lines = parts[3].toInt();
        // Simple date parsing: YYYY-MM-DD
        if (parts[4].length() >= 10) {
            int year = parts[4].mid(0, 4).toInt();
            int month = parts[4].mid(5, 2).toInt();
            int day = parts[4].mid(8, 2).toInt();
            entry.date.setYMD(year, month, day);
        }

        scores.append(entry);
    }

    file.close();

    sortScores();
    return true;
}

bool HighScoreManager::saveScores()
{
    QFile file(scoresFilePath);
    if (!file.open(IO_WriteOnly)) {
        return false;
    }

    QTextStream stream(&file);

    // Write header
    stream << "# Pentix High Scores\n";
    stream << "# Format: Name|Score|Level|Lines|Date\n";

    // Write scores
    for (QValueList<HighScoreEntry>::ConstIterator it = scores.begin();
         it != scores.end(); ++it) {
        // Format date as YYYY-MM-DD
        QString dateStr;
        dateStr.sprintf("%04d-%02d-%02d",
                       (*it).date.year(),
                       (*it).date.month(),
                       (*it).date.day());

        stream << (*it).name << "|"
               << (*it).score << "|"
               << (*it).level << "|"
               << (*it).lines << "|"
               << dateStr << "\n";
    }

    file.close();
    return true;
}

void HighScoreManager::sortScores()
{
    // Simple bubble sort; only 10 entries, so efficiency doesn't matter
    int n = (int)scores.count();
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (scores[j].score < scores[j + 1].score) {
                // Swap
                HighScoreEntry temp = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = temp;
            }
        }
    }

    // Keep only top 10
    while ((int)scores.count() > MAX_SCORES) {
        scores.remove(scores.fromLast());
    }
}

bool HighScoreManager::isHighScore(long score) const
{
    // If we have less than MAX_SCORES entries, any score qualifies
    if ((int)scores.count() < MAX_SCORES) {
        return true;
    }

    // Otherwise, check if score beats the lowest high score
    return score > scores[MAX_SCORES - 1].score;
}

int HighScoreManager::getRank(long score) const
{
    int count = (int)scores.count();
    for (int i = 0; i < count; i++) {
        if (score > scores[i].score) {
            return i + 1;  // Rank is 1-based
        }
    }

    // If not better than any existing score
    if ((int)scores.count() < MAX_SCORES) {
        return (int)scores.count() + 1;
    }

    return 0;  // Didn't make top 10
}

int HighScoreManager::addScore(const QString &name, long score, int level, int lines)
{
    HighScoreEntry entry(name, score, level, lines, QDate::currentDate());

    scores.append(entry);
    sortScores();

    // Find rank of newly added entry
    int count = (int)scores.count();
    for (int i = 0; i < count; i++) {
        if (scores[i].score == score &&
            scores[i].name == name &&
            scores[i].level == level &&
            scores[i].lines == lines) {
            return i + 1;  // Return rank (1-based)
        }
    }

    return 0;  // Didn't make it (shouldn't happen if isHighScore was checked)
}

QValueList<HighScoreEntry> HighScoreManager::getTopScores(int n) const
{
    QValueList<HighScoreEntry> topScores;

    int scoreCount = (int)scores.count();
    int count = (n < scoreCount) ? n : scoreCount;
    for (int i = 0; i < count; i++) {
        topScores.append(scores[i]);
    }

    return topScores;
}

void HighScoreManager::clearScores()
{
    scores.clear();
    saveScores();
}
