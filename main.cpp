#include "main.h"
#include "playfield.h"
#include "piece.h"
#include "ttftext.h"
#include "GameConfig.h"
#include "PieceData.h"
#include "HighScoreManager.h"
#include "HighScoreDialog.h"

// Qt headers
#include <qapplication.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qkeycode.h>

// Standard library
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Debug output control
#define PENTIX_DEBUG 0
#if PENTIX_DEBUG
  #define DEBUG_LOG(fmt, ...) fprintf(stderr, "DEBUG: " fmt "\n", ##__VA_ARGS__)
#else
  #define DEBUG_LOG(fmt, ...) do {} while(0)
#endif

// All 29 piece templates now defined in PieceData.h
// Level speeds now defined in GameConfig.h

// Font path resolution helper
// Searches for font files in the following order:
// 1. FONTPATH environment variable (if set)
// 2. FONTPATH_DEFAULT (compile-time define from Makefile)
// 3. Returns NULL if not found
//
// The returned string must be freed by the caller using delete[]
static char* findFont(const char* fontName)
{
    // Buffer to hold full path
    char* fullPath = new char[512];
    FILE* testFile = 0;

    // First, check FONTPATH environment variable
    const char* envFontPath = getenv("FONTPATH");
    if (envFontPath) {
        snprintf(fullPath, 512, "%s/%s", envFontPath, fontName);
        testFile = fopen(fullPath, "r");
        if (testFile) {
            fclose(testFile);
            DEBUG_LOG("Found font via FONTPATH env: %s", fullPath);
            return fullPath;
        }
    }

    // Second, check compile-time FONTPATH_DEFAULT
#ifdef FONTPATH_DEFAULT
    snprintf(fullPath, 512, "%s/%s", FONTPATH_DEFAULT, fontName);
    testFile = fopen(fullPath, "r");
    if (testFile) {
        fclose(testFile);
        DEBUG_LOG("Found font via FONTPATH_DEFAULT: %s", fullPath);
        return fullPath;
    }
#endif

    // Not found in any location
    DEBUG_LOG("Font not found: %s", fontName);
    delete[] fullPath;
    return 0;
}

// GameWidget implementation
GameWidget::GameWidget(QWidget *parent, const char *name)
    : QWidget(parent, name),
      playfield(0),
      currentPiece(0),
      nextPiece(0),
      gameTimer(0),
      score(0),
      level(1),
      linesCleared(0),
      cellSize(GameConfig::CELL_SIZE),
      timerInterval(1000),
      gameActive(false),
      gamePaused(false),
      showNextPreview(true),  // Show next piece by default
      buffer(0),
      labelFont(0),
      numberFont(0),
      splashCache(0),
      statsCache(0),
      splashCacheDirty(true),
      playfieldDirty(true),
      statsDirty(true)
{
    // Initialize cell cache array
    for (int i = 0; i < GameConfig::CELL_CACHE_SIZE; i++) {
        cellCache[i] = 0;
    }

    playfield = new Playfield(GameConfig::PLAYFIELD_COLUMNS, GameConfig::PLAYFIELD_ROWS);

    setBackgroundColor(Qt::black);
    setBackgroundMode(NoBackground);  // Prevent Qt from erasing background - full repaint used
    setMinimumSize(GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT);
    setMaximumSize(GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT);
    setFocusPolicy(StrongFocus);

    buffer = new QPixmap(GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT);

    // Load TTF fonts - primary for labels, secondary for numbers
    labelFont = new TtfText();
    numberFont = new TtfText();

    // Try to load primary font (Orbitron) for labels
    char* primaryPath = findFont(GameConfig::FONT_PRIMARY_NAME);
    bool primaryLoaded = false;
    if (primaryPath) {
        primaryLoaded = labelFont->loadFont(primaryPath, GameConfig::FONT_LABEL_SIZE);
        delete[] primaryPath;
    }

    if (!primaryLoaded) {
        // Try fallback font
        char* fallbackPath = findFont(GameConfig::FONT_FALLBACK_NAME);
        if (fallbackPath) {
            if (!labelFont->loadFont(fallbackPath, GameConfig::FONT_FALLBACK_SIZE)) {
                DEBUG_LOG("Could not load label font");
                delete labelFont;
                labelFont = 0;
            }
            delete[] fallbackPath;
        } else {
            DEBUG_LOG("Could not find any label font");
            delete labelFont;
            labelFont = 0;
        }
    } else {
        DEBUG_LOG("Loaded primary font for labels");
    }

    // Load secondary font (DejaVu Sans Bold) for numbers
    char* secondaryPath = findFont(GameConfig::FONT_SECONDARY_NAME);
    if (secondaryPath) {
        if (!numberFont->loadFont(secondaryPath, GameConfig::FONT_NUMBER_SIZE)) {
            DEBUG_LOG("Could not load number font");
            delete numberFont;
            numberFont = 0;
        } else {
            DEBUG_LOG("Loaded secondary font for numbers");
        }
        delete[] secondaryPath;
    } else {
        DEBUG_LOG("Could not find number font");
        delete numberFont;
        numberFont = 0;
    }

    gameTimer = new QTimer(this);
    connect(gameTimer, SIGNAL(timeout()), SLOT(advanceGame()));

    // Initialize cached cell graphics with gradients
    initCellCache();

    srand(time(0));
}

GameWidget::~GameWidget()
{
    delete currentPiece;
    delete nextPiece;
    delete playfield;
    delete buffer;
    delete labelFont;
    delete numberFont;

    // Clean up cached cell graphics
    for (int i = 0; i < GameConfig::CELL_CACHE_SIZE; i++) {
        delete cellCache[i];
    }

    // Clean up performance caches
    delete splashCache;
    delete statsCache;
}

// Helper function to get cache index for a color
static int getCacheIndex(const QColor &color)
{
    if (color == Qt::blue) return 0;
    if (color == Qt::red) return 1;
    if (color == Qt::lightGray) return 2;
    if (color == Qt::magenta) return 3;
    if (color == Qt::cyan) return 4;
    if (color == Qt::green) return 5;
    if (color == Qt::yellow) return 6;
    // Default fallback
    return 0;
}

void GameWidget::initCellCache()
{
    // Pre-render cells with gradients for all piece colors
    // This improves rendering performance significantly
    QColor colors[GameConfig::CELL_CACHE_SIZE] = {
        Qt::blue,              // 0 - Standard blue pieces
        Qt::red,               // 1 - Standard red pieces
        Qt::lightGray,         // 2 - Standard gray pieces
        Qt::magenta,           // 3 - Standard magenta pieces
        Qt::cyan,              // 4 - Standard cyan pieces
        Qt::green,             // 5 - Standard green pieces
        Qt::yellow,            // 6 - Standard yellow pieces
        QColor(255, 120, 100), // 7 - Orange/red (unused but reserved)
        QColor(100, 255, 100), // 8 - Bright green (unused but reserved)
        QColor(100, 200, 255)  // 9 - Bright cyan (unused but reserved)
    };

    for (int idx = 0; idx < GameConfig::CELL_CACHE_SIZE; idx++) {
        cellCache[idx] = new QPixmap(cellSize - 1, cellSize - 1);
        QPainter p(cellCache[idx]);

        QColor color = colors[idx];

        // Draw vertical gradient (lighter at top, darker at bottom)
        for (int i = 0; i < cellSize - 1; i++) {
            float ratio = (float)i / (cellSize - 1);
            int r = color.red() - (int)(color.red() * ratio * GameConfig::GRADIENT_DARKNESS);
            int g = color.green() - (int)(color.green() * ratio * GameConfig::GRADIENT_DARKNESS);
            int b = color.blue() - (int)(color.blue() * ratio * GameConfig::GRADIENT_DARKNESS);
            p.setPen(QColor(r, g, b));
            p.drawLine(0, i, cellSize - 2, i);
        }

        // Add thin border for cell definition
        p.setPen(Qt::black);
        p.setBrush(NoBrush);
        p.drawRect(0, 0, cellSize - 2, cellSize - 2);

        p.end();
    }
}

void GameWidget::initSplashCache()
{
    // Create and cache the splash screen to avoid recreating TtfText objects on every paintEvent
    if (!splashCache) {
        splashCache = new QPixmap(GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT);
    }

    QPainter p(splashCache);
    p.fillRect(0, 0, GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT, Qt::black);

    if (labelFont && numberFont) {
        // Draw large "PENTIX" title with colorful letters
        const char* letters = "PENTIX";
        QColor colors[] = {
            QColor(255, 120, 100),  // P - red
            QColor(100, 255, 100),  // E - green
            QColor(100, 200, 255),  // N - cyan
            QColor(255, 255, 100),  // T - yellow
            QColor(240, 240, 240),  // I - white
            QColor(255, 100, 255)   // X - magenta
        };

        // Create larger font for title
        TtfText *titleFont = new TtfText();
        char* titlePath = findFont(GameConfig::FONT_PRIMARY_NAME);
        if (titlePath) {
            if (titleFont->loadFont(titlePath, GameConfig::FONT_TITLE_SIZE)) {
                for (int i = 0; i < 6; i++) {
                    char letter[2] = {letters[i], 0};
                    titleFont->drawText(p, GameConfig::TITLE_LETTER_POSITIONS[i],
                                       GameConfig::TITLE_Y_POSITION, letter, colors[i]);
                }
            }
            delete[] titlePath;
        }
        delete titleFont;

        // Credits
        TtfText *creditsFont = new TtfText();
        char* creditsPath = findFont(GameConfig::FONT_FALLBACK_NAME);
        if (creditsPath) {
            if (creditsFont->loadFont(creditsPath, GameConfig::FONT_CREDITS_SIZE)) {
                creditsFont->drawText(p, GameConfig::CREDITS_LINE1_X, GameConfig::CREDITS_LINE1_Y,
                                     "Based on the 1986 game by", QColor(120, 255, 120));
                creditsFont->drawText(p, GameConfig::CREDITS_LINE2_X, GameConfig::CREDITS_LINE2_Y,
                                     "Marta & Adrian Soncodi", QColor(120, 255, 120));
            }
            delete[] creditsPath;
        }
        delete creditsFont;

        // Instructions
        TtfText *instructFont = new TtfText();
        char* instructPath = findFont(GameConfig::FONT_FALLBACK_NAME);
        if (instructPath) {
            if (instructFont->loadFont(instructPath, GameConfig::FONT_INSTRUCTIONS_SIZE)) {
                instructFont->drawText(p, GameConfig::INSTRUCTIONS_LINE1_X, GameConfig::INSTRUCTIONS_LINE1_Y,
                                      "Press 0-9 to choose a starting level", QColor(200, 200, 200));
                instructFont->drawText(p, GameConfig::INSTRUCTIONS_LINE2_X, GameConfig::INSTRUCTIONS_LINE2_Y,
                                      "or press Ctrl+N to start at level 0", QColor(180, 180, 180));
                instructFont->drawText(p, GameConfig::INSTRUCTIONS_LINE3_X, GameConfig::INSTRUCTIONS_LINE3_Y,
                                      "Arrow keys - Move and rotate the piece", QColor(160, 160, 160));
                instructFont->drawText(p, GameConfig::INSTRUCTIONS_LINE4_X, GameConfig::INSTRUCTIONS_LINE4_Y,
                                      "Space - Hard drop", QColor(160, 160, 160));
                instructFont->drawText(p, GameConfig::INSTRUCTIONS_LINE5_X, GameConfig::INSTRUCTIONS_LINE5_Y,
                                      "1 - Toggle next piece preview", QColor(160, 160, 160));
            }
            delete[] instructPath;
        }
        delete instructFont;
    } else {
        // Fallback
        p.setPen(Qt::white);
        p.setFont(QFont("Helvetica", 12));
        p.drawText(0, 0, GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT, AlignCenter,
                   "PENTIX\n\nPress 0-9 to choose a starting level\nor press Ctrl+N to start at level 0\n\n"
                   "Controls:\n"
                   "Left/Right - Move\n"
                   "Down - Soft drop\n"
                   "Up/Space - Rotate");
    }

    p.end();
    splashCacheDirty = false;
}

void GameWidget::startGame(int startLevel)
{
    DEBUG_LOG("startGame() called with level %d", startLevel);
    gameActive = true;
    gamePaused = false;
    score = 0;
    level = startLevel;  // Start at requested level
    linesCleared = 0;
    playfield->clear();

    // Invalidate caches for a new game
    playfieldDirty = true;
    statsDirty = true;
    delete currentPiece;
    delete nextPiece;
    currentPiece = 0;
    nextPiece = 0;

    // Initialize the next piece first, then create the current piece from it
    spawnNextPiece();  // Creates nextPiece
    spawnNextPiece();  // Current = next, creates new next

    // Set timer speed based on starting level (uses GameConfig::LEVEL_SPEEDS)
    timerInterval = (level <= GameConfig::MAX_LEVEL) ? GameConfig::LEVEL_SPEEDS[level] : GameConfig::LEVEL_SPEEDS[GameConfig::MAX_LEVEL];
    gameTimer->start(timerInterval, false);  // false = repeating timer
    update();
}

void GameWidget::stopGame()
{
    DEBUG_LOG("stopGame() called");
    gameActive = false;
    gamePaused = false;
    gameTimer->stop();
    splashCacheDirty = true;  // Need to redraw splash screen
    update();
}

void GameWidget::pauseGame()
{
    if (!gameActive || gamePaused) return;

    DEBUG_LOG("pauseGame() called");
    gamePaused = true;
    gameTimer->stop();
    update();
}

void GameWidget::resumeGame()
{
    if (!gameActive || !gamePaused) return;

    DEBUG_LOG("resumeGame() called");
    gamePaused = false;
    gameTimer->start(timerInterval, false);  // Resume with saved interval
    update();
}

/**
 * Creates a random piece from the 29 available piece types.
 * Uses lookup table from PieceData for clean implementation.
 * Returns pieces numbered 1-29 (index 0 in table is unused for 1-based indexing)
 */
static Piece* createRandomPiece(Playfield *playfield)
{
    // Select random piece from 1 to 29
    int pieceType = (rand() % PieceData::PIECE_COUNT) + 1;

    // Look up template from the piece data table
    const PieceData::PieceTemplate &templateData = PieceData::PIECE_TEMPLATES[pieceType];

    // Create piece from template
    Piece *piece = new Piece(templateData.size, templateData.shape, templateData.color);

    // Calculate starting X position (centered horizontally)
    // Formula: x = round((columns / 2 - size / 2) + 0.5)
    // Example for 12 columns, size 3: (12/2 - 3/2) + 0.5 = 5.5 → rounds to 6
    int startX = (int)((playfield->cols() / 2 - templateData.size / 2) + 0.5);
    piece->setPosition(startX, 0);

    return piece;
}

/**
 * Generates a new piece for play.
 * - currentPiece = nextPiece (what was previewed becomes current)
 * - nextPiece = new random piece
 */
void GameWidget::spawnNextPiece()
{
    if (!nextPiece) {
        // First call - create nextPiece only
        nextPiece = createRandomPiece(playfield);
        return;
    }

    // Advance: current = next, next = new random
    delete currentPiece;
    currentPiece = nextPiece;
    nextPiece = createRandomPiece(playfield);

    // Check if game over
    if (!playfield->canPlacePiece(currentPiece, currentPiece->x(), currentPiece->y())) {
        stopGame();

        // Call parent's checkHighScore method
        MainWindow *mainWin = (MainWindow*)parentWidget();
        if (mainWin) {
            mainWin->checkHighScore();
        }
    }
}

void GameWidget::advanceGame()
{
    if (!currentPiece || !gameActive) return;

    // Move down one row
    currentPiece->move(0, 1);

    // Check if the piece can be placed at the new position
    if (!playfield->canPlacePiece(currentPiece, currentPiece->x(), currentPiece->y())) {
        // Can't place at new position - move back and fix there
        currentPiece->move(0, -1);

        // Score calculation:
        // Score = BASE_SCORE + drop_distance + LEVEL_MULTIPLIER * level
        // Penalty for showing next piece
        int hardDropStartY = currentPiece->hardDropStartY();
        if (hardDropStartY == -1) {
            hardDropStartY = currentPiece->y();
        }
        score += (long)(GameConfig::BASE_SCORE + currentPiece->y() - hardDropStartY + GameConfig::LEVEL_MULTIPLIER * level);
        // Apply penalty if next piece preview is enabled
        if (showNextPreview) {
            score -= GameConfig::NEXT_PREVIEW_PENALTY;
        }
        DEBUG_LOG("y=%d hard_drop_start_y=%d level=%d score=%ld showNextPreview=%d",
                currentPiece->y(), hardDropStartY, level, score, showNextPreview);

        statsDirty = true;  // Score changed
        playfieldDirty = true;  // Playfield will change after locking piece

        playfield->lockPiece(currentPiece);

        // Remove full lines
        int removed = playfield->clearFullLines();
        if (removed > 0) {
            linesCleared += removed;
            DEBUG_LOG("removed %d lines, total=%d", removed, linesCleared);
            statsDirty = true;  // Lines changed
            // Level = min(max(linesCleared / 10, level), MAX_LEVEL)
            int levelFromLines = linesCleared / GameConfig::LINES_PER_LEVEL;
            int maxLevel = (levelFromLines > level) ? levelFromLines : level;
            int oldLevel = level;
            level = (maxLevel < GameConfig::MAX_LEVEL) ? maxLevel : GameConfig::MAX_LEVEL;
            DEBUG_LOG("new level=%d", level);
            // Update timer speed if level changed
            if (level != oldLevel) {
                timerInterval = (level <= GameConfig::MAX_LEVEL) ? GameConfig::LEVEL_SPEEDS[level] : GameConfig::LEVEL_SPEEDS[GameConfig::MAX_LEVEL];
                gameTimer->changeInterval(timerInterval);
                statsDirty = true;  // Level changed
            }
        }

        spawnNextPiece();
    }

    update();
}

void GameWidget::keyPressEvent(QKeyEvent *e)
{
    // Filter out keyboard auto-repeat to prevent input lag (Solution A)
    if (e->isAutoRepeat()) {
        e->ignore();
        return;
    }

    // Handle level selection (0-9) on splash screen
    if (!gameActive) {
        int selectedLevel = -1;
        switch(e->key()) {
        case Key_0: selectedLevel = 0; break;
        case Key_1: selectedLevel = 1; break;
        case Key_2: selectedLevel = 2; break;
        case Key_3: selectedLevel = 3; break;
        case Key_4: selectedLevel = 4; break;
        case Key_5: selectedLevel = 5; break;
        case Key_6: selectedLevel = 6; break;
        case Key_7: selectedLevel = 7; break;
        case Key_8: selectedLevel = 8; break;
        case Key_9: selectedLevel = 9; break;
        }

        if (selectedLevel >= 0 && selectedLevel <= GameConfig::MAX_LEVEL) {
            DEBUG_LOG("Level %d selected, starting game", selectedLevel);
            startGame(selectedLevel);
            e->accept();
            return;
        }
        e->ignore();
        return;
    }

    // Game is active - handle game controls
    if (!currentPiece) return;

    switch(e->key()) {
    case Key_Left:
        if (currentPiece->canMove(-1, 0, playfield)) {
            currentPiece->move(-1, 0);
            update();  // Async - Qt optimizes multiple calls
        }
        break;
    case Key_Right:
        if (currentPiece->canMove(1, 0, playfield)) {
            currentPiece->move(1, 0);
            update();
        }
        break;
    case Key_Down:
        if (currentPiece->canMove(0, 1, playfield)) {
            currentPiece->move(0, 1);
            update();
        }
        break;
    case Key_Up:
        // Up key rotates the piece
        currentPiece->rotate();
        // Check if rotation is valid
        if (!playfield->canPlacePiece(currentPiece, currentPiece->x(), currentPiece->y())) {
            // Rotate back (rotate 3 more times)
            currentPiece->rotate();
            currentPiece->rotate();
            currentPiece->rotate();
        }
        update();
        break;
    case Key_Space:
        // Space key drops the piece instantly
        currentPiece->hardDrop(playfield);
        update();
        break;
    case Key_1:
        // Toggle next piece preview display
        showNextPreview = !showNextPreview;
        update();
        break;
    }

    // Accept event to prevent propagation (Solution C)
    e->accept();
}

void GameWidget::paintEvent(QPaintEvent *)
{
    if (!buffer) return;

    // Draw to offscreen buffer first
    QPainter bufPainter(buffer);

    if (!gameActive) {
    // Use the cached splash screen; render it once.
        if (splashCacheDirty) {
            initSplashCache();
        }
        if (splashCache) {
            bufPainter.drawPixmap(0, 0, *splashCache);
        }
    } else {
        // Simple black background
        bufPainter.fillRect(0, 0, width(), height(), Qt::black);

        // Draw playfield
        drawPlayfield(bufPainter);

        // Draw current piece
        if (currentPiece) {
            drawPiece(bufPainter, currentPiece);
        }

        // Draw next piece preview (only if enabled)
        if (showNextPreview && nextPiece) {
            drawNextPiece(bufPainter, nextPiece);
        }

        // Draw stats panel (cached for performance)
        drawStatsPanel(bufPainter);
    }

    bufPainter.end();

    // Now blit the buffer to screen in one operation
    QPainter p(this);
    p.drawPixmap(0, 0, *buffer);
}

void GameWidget::drawPlayfield(QPainter &p)
{
    int playfieldX = GameConfig::PLAYFIELD_OFFSET_X;
    int playfieldY = GameConfig::PLAYFIELD_OFFSET_Y;

    int playfieldWidth = playfield->cols() * cellSize;
    int playfieldHeight = playfield->rows() * cellSize;

    // Draw playfield background (dark gray)
    p.fillRect(playfieldX, playfieldY, playfieldWidth, playfieldHeight, QColor(20, 20, 20));

    // Draw cells using cached pixmaps with gradients
    for (int row = 0; row < playfield->rows(); row++) {
        for (int col = 0; col < playfield->cols(); col++) {
            QColor color = playfield->cellColor(row, col);
            if (color != Qt::black) {
                int cellX = playfieldX + col * cellSize;
                int cellY = playfieldY + row * cellSize;

                // Use cached pixmap with gradient
                int cacheIdx = getCacheIndex(color);
                if (cellCache[cacheIdx]) {
                    p.drawPixmap(cellX, cellY, *cellCache[cacheIdx]);
                }
            }
        }
    }
}

void GameWidget::drawPiece(QPainter &p, const Piece *piece)
{
    if (!piece) return;

    int playfieldX = GameConfig::PLAYFIELD_OFFSET_X;
    int playfieldY = GameConfig::PLAYFIELD_OFFSET_Y;

    // X coordinate is 1-based, adjust for 0-based array
    QColor color = piece->color();
    int cacheIdx = getCacheIndex(color);

    for (int i = 0; i < piece->size(); i++) {
        for (int j = 0; j < piece->size(); j++) {
            if (piece->cell(i, j)) {
                int blockX = playfieldX + (piece->x() + j - 1) * cellSize;  // x is 1-based, need -1
                int blockY = playfieldY + (piece->y() + i) * cellSize;

                // Use cached pixmap with gradient
                if (cellCache[cacheIdx]) {
                    p.drawPixmap(blockX, blockY, *cellCache[cacheIdx]);
                }
            }
        }
    }
}

void GameWidget::drawNextPiece(QPainter &p, const Piece *piece)
{
    if (!piece) return;

    int playfieldX = GameConfig::PLAYFIELD_OFFSET_X;
    int playfieldY = GameConfig::PLAYFIELD_OFFSET_Y;

    // Position next preview to the right of the playfield
    int nextX = playfieldX + cellSize * (playfield->cols() + GameConfig::NEXT_PREVIEW_X_OFFSET);
    int nextY = playfieldY + cellSize * playfield->rows() / 2 + GameConfig::NEXT_PREVIEW_Y_OFFSET;

    // "NEXT" label (simple version, no card background)
    if (labelFont) {
        labelFont->drawText(p, nextX, nextY, "NEXT", QColor(100, 150, 255));
    }

    // Draw the next piece using cached pixmaps
    int pieceOffsetX = nextX + GameConfig::NEXT_LABEL_X_OFFSET;
    int pieceOffsetY = nextY + GameConfig::NEXT_LABEL_Y_OFFSET;
    QColor color = piece->color();
    int cacheIdx = getCacheIndex(color);

    for (int i = 0; i < piece->size(); i++) {
        for (int j = 0; j < piece->size(); j++) {
            if (piece->cell(i, j)) {
                int blockX = pieceOffsetX + j * cellSize;
                int blockY = pieceOffsetY + i * cellSize;

                // Use cached pixmap with gradient
                if (cellCache[cacheIdx]) {
                    p.drawPixmap(blockX, blockY, *cellCache[cacheIdx]);
                }
            }
        }
    }
}

void GameWidget::drawStatsPanel(QPainter &p)
{
    // Check if stats have changed - only redraw if necessary
    if (statsDirty) {
        // Create stats cache if needed
        if (!statsCache) {
            statsCache = new QPixmap(110, 300);
        }

        // Render stats to cache
        QPainter sp(statsCache);
        sp.fillRect(0, 0, 110, 300, Qt::black);

        int textX = 0;
        int textYStart = 15;  // Offset to prevent text clipping at top

        if (labelFont && numberFont) {
            char buf[100];

            // Level
            labelFont->drawText(sp, textX, textYStart, "LEVEL", QColor(100, 180, 255));
            sprintf(buf, "%d", level);
            numberFont->drawText(sp, textX + GameConfig::STATS_NUMBER_X_OFFSET,
                                  textYStart + GameConfig::STATS_NUMBER_Y_OFFSET, buf, Qt::white);

            // Lines
            labelFont->drawText(sp, textX, textYStart + 70, "LINES", QColor(100, 255, 180));
            sprintf(buf, "%d", linesCleared);
            numberFont->drawText(sp, textX + GameConfig::STATS_NUMBER_X_OFFSET,
                                  textYStart + 95, buf, Qt::white);

            // Score
            labelFont->drawText(sp, textX, textYStart + 140, "SCORE", QColor(255, 255, 100));
            sprintf(buf, "%ld", score);
            numberFont->drawText(sp, textX + GameConfig::STATS_NUMBER_X_OFFSET,
                                  textYStart + 165, buf, Qt::white);
        } else {
            // Fallback to Qt default font
            sp.setPen(Qt::white);
            sp.drawText(textX, 2 * cellSize, QString("Level: %1").arg(level));
            sp.drawText(textX, 3 * cellSize, QString("Lines: %1").arg(linesCleared));
            sp.drawText(textX, 5 * cellSize, QString("Score: %1").arg(score));
        }

        sp.end();
        statsDirty = false;
    }

    // Draw cached stats panel
    if (statsCache) {
        p.drawPixmap(10, 80, *statsCache);
    }

    // Draw static bottom text (minimal overhead - single TTF call)
    if (labelFont) {
        labelFont->drawText(p, GameConfig::BOTTOM_MESSAGE_X, GameConfig::BOTTOM_MESSAGE_Y,
                         "Play Pentix!", QColor(0, 255, 0));
    } else {
        p.setPen(Qt::green);
        p.drawText(GameConfig::BOTTOM_MESSAGE_X, GameConfig::BOTTOM_MESSAGE_Y, "Play Pentix!");
    }
}

// MainWindow implementation
MainWindow::MainWindow(QWidget *parent, const char *name)
    : QMainWindow(parent, name),
      scoreManager(0)
{
    DEBUG_LOG("MainWindow constructor");

    setCaption("Pentix");

    // Create high score manager
    scoreManager = new HighScoreManager();

    gameWidget = new GameWidget(this);
    setCentralWidget(gameWidget);

    setupMenuBar();

    DEBUG_LOG("MainWindow constructor done");
}

MainWindow::~MainWindow()
{
    DEBUG_LOG("MainWindow destructor");
    delete scoreManager;
}

void MainWindow::setupMenuBar()
{
    QPopupMenu *gameMenu = new QPopupMenu(this);
    gameMenu->insertItem("&Start Game", this, SLOT(newGame()), CTRL+Key_N);
    gameMenu->insertItem("Start at &Level...", this, SLOT(newGameAtLevel()), CTRL+Key_L);
    gameMenu->insertItem("&Pause/Resume", this, SLOT(pauseResumeGame()), CTRL+Key_P);
    gameMenu->insertSeparator();
    gameMenu->insertItem("&High Scores...", this, SLOT(showHighScores()), CTRL+Key_H);
    gameMenu->insertSeparator();
    gameMenu->insertItem("&Quit", qApp, SLOT(quit()), CTRL+Key_Q);

    QPopupMenu *helpMenu = new QPopupMenu(this);
    helpMenu->insertItem("&How to Play...", this, SLOT(showHowToPlay()));
    helpMenu->insertItem("&About", this, SLOT(showAbout()));

    menuBar()->insertItem("&Game", gameMenu);
    menuBar()->insertItem("&Help", helpMenu);
}

void MainWindow::newGame()
{
    DEBUG_LOG("newGame() called");
    gameWidget->startGame(0);  // Start at level 0
}

void MainWindow::newGameAtLevel()
{
    bool ok;
    int level = QInputDialog::getInteger(
        "Choose a Starting Level",
        "Choose a starting level (0-9):\n0 = Beginner, 9 = Expert",
        0,     // default value
        0,     // min
        GameConfig::MAX_LEVEL,  // max
        1,     // step
        &ok,
        this
    );

    if (ok) {
        gameWidget->startGame(level);
    }
}

void MainWindow::pauseResumeGame()
{
    if (gameWidget->isPaused()) {
        gameWidget->resumeGame();
    } else {
        gameWidget->pauseGame();
    }
}

void MainWindow::showHighScores()
{
    HighScoreDialog dialog(scoreManager, this);
    dialog.exec();
}

void MainWindow::showHowToPlay()
{
    QMessageBox::information(this, "How to Play",
        "Pentix - Pentomino Puzzle Game\n\n"
        "CONTROLS:\n"
        "  Arrow keys - Move the piece left/right/down\n"
        "  Up arrow or Space - Rotate the piece\n"
        "  1 - Toggle next piece preview\n\n"
        "GOAL:\n"
        "  Clear lines by filling rows completely\n"
        "  Higher levels are faster\n\n"
        "SCORING:\n"
        "  Base: 5 points per piece\n"
        "  Bonus: Distance dropped\n"
        "  Level multiplier: 3x per level");
}

void MainWindow::showAbout()
{
    DEBUG_LOG("showAbout() called");
    QMessageBox::about(this, "About Pentix",
                       "Pentix v1.0\n\n"
                       "A Qt2 Pentomino game\n"
                       "Based on the 1986 original by Marta & Adrian Soncodi");
}

void MainWindow::checkHighScore()
{
    long finalScore = gameWidget->getScore();
    int finalLevel = gameWidget->getLevel();
    int finalLines = gameWidget->getLinesCleared();

    // Show game over message
    QString gameOverMsg = QString("Game Over!\n\nScore: %1\nLevel: %2\nLines: %3")
                          .arg(finalScore)
                          .arg(finalLevel)
                          .arg(finalLines);

    // Check if it's a high score
    if (scoreManager->isHighScore(finalScore)) {
        int rank = scoreManager->getRank(finalScore);
        gameOverMsg += QString("\n\nCongratulations!\nYou ranked #%1!").arg(rank);

        QMessageBox::information(this, "Game Over", gameOverMsg);

        // Get player name (3 characters, arcade style)
        bool ok;
        QString name = QInputDialog::getText(
            "New High Score!",
            "Enter your name (3 characters):",
            QLineEdit::Normal,
            "",
            &ok,
            this
        );

        if (ok && !name.isEmpty()) {
            // Limit to 3 characters
            if (name.length() > 3) {
                name = name.left(3);
            }
            name = name.upper();  // Uppercase, arcade style

            // Add to high scores
            scoreManager->addScore(name, finalScore, finalLevel, finalLines);
            scoreManager->saveScores();

            // Show updated high scores table
            showHighScores();
        }
    } else {
        QMessageBox::information(this, "Game Over", gameOverMsg);
    }
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    DEBUG_LOG("MainWindow::closeEvent() called");
    e->accept();
    DEBUG_LOG("MainWindow::closeEvent() accepted, calling qApp->quit()");
    qApp->quit();
}

// Main function
int main(int argc, char **argv)
{
    DEBUG_LOG("main() starting");

    QApplication app(argc, argv);

    MainWindow *mainWin = new MainWindow();
    app.setMainWidget(mainWin);
    mainWin->show();

    DEBUG_LOG("main() entering event loop");
    int ret = app.exec();

    DEBUG_LOG("main() event loop exited with code %d", ret);
    delete mainWin;
    DEBUG_LOG("main() exiting");

    return ret;
}

#include "main.moc"
