#ifndef HIGHSCOREDIALOG_H
#define HIGHSCOREDIALOG_H

#include <qdialog.h>
#include <qtable.h>
#include <qpushbutton.h>
#include "HighScoreManager.h"

/**
 * HighScoreDialog
 *
 * Displays the high score table in a simple Qt dialog.
 * Shows rank, name, score, level, lines cleared, and date.
 */

class HighScoreDialog : public QDialog {
    Q_OBJECT

public:
    HighScoreDialog(HighScoreManager *manager, QWidget *parent = 0, const char *name = 0);
    ~HighScoreDialog();

    // Refresh the table with current high scores
    void refreshScores();

private:
    HighScoreManager *scoreManager;
    QTable *table;
    QPushButton *closeButton;

    void setupUI();
    void populateTable();
};

#endif // HIGHSCOREDIALOG_H
