#ifndef MAIN_H
#define MAIN_H

#include <qmainwindow.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qtimer.h>

class Playfield;
class Piece;
class TtfText;
class HighScoreManager;

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    GameWidget(QWidget *parent = 0, const char *name = 0);
    ~GameWidget();

    void startGame(int startLevel = 0);
    void stopGame();
    void pauseGame();
    void resumeGame();
    bool isPaused() const { return gamePaused; }

    // Get current game state for high scores
    long getScore() const { return score; }
    int getLevel() const { return level; }
    int getLinesCleared() const { return linesCleared; }

protected:
    void paintEvent(QPaintEvent *e);
    void keyPressEvent(QKeyEvent *e);

private slots:
    void advanceGame();

private:
    Playfield *playfield;
    Piece *currentPiece;
    Piece *nextPiece;   // Next piece to show
    QTimer *gameTimer;
    long score;
    int level;
    int linesCleared; // Total lines cleared
    int cellSize;
    int timerInterval; // Current timer interval (for pause/resume)
    bool gameActive;
    bool gamePaused;  // Pause state
    bool showNextPreview;    // Show next piece preview
    QPixmap *buffer;
    TtfText *labelFont;      // TTF font for labels
    TtfText *numberFont;  // TTF font for numbers

    // Cache for pre-rendered cells with gradients
    QPixmap *cellCache[10];  // Cache for different colored cells

    // Performance optimization: cache rendered screens
    QPixmap *splashCache;  // Cached splash screen (rendered once)
    QPixmap *statsCache;   // Cached stats panel

    // Dirty flags for selective repainting
    bool splashCacheDirty;
    bool playfieldDirty;
    bool statsDirty;

    void spawnNextPiece();
    void initCellCache();    // Initialize cached cell graphics
    void initSplashCache();  // Initialize cached splash screen
    void drawPlayfield(QPainter &p);
    void drawPiece(QPainter &p, const Piece *piece);
    void drawNextPiece(QPainter &p, const Piece *piece);  // Draw next piece preview
    void drawStatsPanel(QPainter &p);  // Draw stats panel (cached)
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0, const char *name = 0);
    ~MainWindow();

    void checkHighScore();  // Check if current game is a high score

protected:
    void closeEvent(QCloseEvent *e);

private slots:
    void newGame();
    void newGameAtLevel();
    void pauseResumeGame();
    void showHighScores();
    void showAbout();
    void showHowToPlay();

private:
    GameWidget *gameWidget;
    HighScoreManager *scoreManager;
    void setupMenuBar();
};

#endif // MAIN_H
