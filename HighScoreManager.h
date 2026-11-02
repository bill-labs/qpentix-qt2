#ifndef HIGHSCOREMANAGER_H
#define HIGHSCOREMANAGER_H

#include <qstring.h>
#include <qvaluelist.h>
#include <qdatetime.h>

/**
 * HighScoreManager
 *
 * Manages high score persistence and ranking.
 * Stores top 10 scores in ~/.qpentix_scores file.
 */

struct HighScoreEntry {
    QString name;      // Player name (3 characters, arcade style)
    long score;        // Final score
    int level;         // Level reached
    int lines;         // Lines cleared
    QDate date;        // Date achieved

    HighScoreEntry() : score(0), level(0), lines(0) {}

    HighScoreEntry(const QString &n, long s, int lvl, int ln, const QDate &d)
        : name(n), score(s), level(lvl), lines(ln), date(d) {}
};

class HighScoreManager {
public:
    HighScoreManager();
    ~HighScoreManager();

    // Load scores from file
    bool loadScores();

    // Save scores to file
    bool saveScores();

    // Check if score qualifies for high score table
    bool isHighScore(long score) const;

    // Get rank for a given score (1-10, or 0 if not in top 10)
    int getRank(long score) const;

    // Add new high score entry
    // Returns rank (1-10) or 0 if didn't make it
    int addScore(const QString &name, long score, int level, int lines);

    // Get all high scores (up to 10)
    const QValueList<HighScoreEntry>& getScores() const { return scores; }

    // Get top N scores
    QValueList<HighScoreEntry> getTopScores(int n) const;

    // Clear all scores
    void clearScores();

private:
    QValueList<HighScoreEntry> scores;
    QString scoresFilePath;

    static const int MAX_SCORES = 10;

    // Get default scores file path (~/.qpentix_scores)
    QString getDefaultScoresPath() const;

    // Sort scores by score (descending)
    void sortScores();
};

#endif // HIGHSCOREMANAGER_H
