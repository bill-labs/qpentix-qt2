#ifndef PIECEDATA_H
#define PIECEDATA_H

#include <qcolor.h>

/**
 * PieceData.h
 *
 * All 29 pentomino/polyomino piece templates for Pentix.
 *
 * Each template is a square grid (1x1, 2x2, 3x3, 4x4, or 5x5)
 * where true (1) represents a filled block and false (0) is empty space.
 */

namespace PieceData {

// ===== Piece Template Structure =====
struct PieceTemplate {
    int size;           // Grid size (1-5)
    const bool* shape;  // Shape data array
    QColor color;       // Piece color
};

// ===== All 29 Shape Arrays =====

// Size 1x1 (1 block)
static const bool SHAPE_SINGLE[] = {
    1
};

// Size 2x2 (2 blocks)
static const bool SHAPE_VERTICAL_TWO[] = {
    0, 1,
    0, 1
};

static const bool SHAPE_SQUARE_TWO[] = {
    1, 1,
    1, 1
};

static const bool SHAPE_CORNER[] = {
    1, 0,
    1, 0
};

// Size 3x3 (3-4 blocks)
static const bool SHAPE_VERTICAL_THREE[] = {
    0, 0, 1,
    0, 0, 1,
    0, 0, 1
};

static const bool SHAPE_L_LEFT[] = {
    0, 1, 0,
    0, 1, 0,
    0, 1, 1
};

static const bool SHAPE_L_RIGHT[] = {
    0, 1, 0,
    0, 1, 0,
    1, 1, 0
};

static const bool SHAPE_Z_LEFT[] = {
    1, 0, 0,
    1, 1, 0,
    0, 1, 0
};

static const bool SHAPE_Z_RIGHT[] = {
    0, 1, 0,
    1, 1, 0,
    1, 0, 0
};

static const bool SHAPE_T[] = {
    0, 1, 0,
    1, 1, 1,
    0, 0, 0
};

static const bool SHAPE_CROSS[] = {
    0, 1, 0,
    1, 1, 1,
    0, 1, 0
};

static const bool SHAPE_PLUS_RIGHT[] = {
    0, 1, 0,
    1, 1, 1,
    0, 0, 1
};

static const bool SHAPE_CORNER_LARGE[] = {
    1, 1, 1,
    1, 0, 0,
    1, 0, 0
};

static const bool SHAPE_L_WIDE[] = {
    0, 1, 1,
    1, 1, 0,
    0, 1, 0
};

static const bool SHAPE_U_LEFT[] = {
    0, 1, 0,
    1, 1, 0,
    1, 1, 0
};

static const bool SHAPE_U_RIGHT[] = {
    1, 1, 0,
    0, 1, 0,
    1, 1, 0
};

static const bool SHAPE_BRIDGE_LEFT[] = {
    1, 1, 0,
    0, 1, 0,
    0, 1, 1
};

static const bool SHAPE_BRIDGE_RIGHT[] = {
    0, 1, 1,
    0, 1, 0,
    1, 1, 0
};

static const bool SHAPE_W_LEFT[] = {
    0, 1, 0,
    1, 1, 0,
    1, 0, 0,
    0, 0, 1
};

static const bool SHAPE_W_RIGHT[] = {
    0, 1, 0,
    1, 1, 0,
    1, 1, 0
};

static const bool SHAPE_STAIRS[] = {
    1, 1, 0,
    0, 1, 1,
    0, 0, 1
};

static const bool SHAPE_HOOK_LARGE[] = {
    0, 1, 1,
    0, 1, 0,
    1, 1, 0
};

// Size 4x4 (4-5 blocks)
static const bool SHAPE_VERTICAL_FOUR[] = {
    0, 0, 0, 1,
    0, 0, 0, 1,
    0, 0, 0, 1,
    0, 0, 0, 1
};

static const bool SHAPE_L_LARGE_LEFT[] = {
    0, 0, 1, 0,
    0, 0, 1, 0,
    0, 0, 1, 0,
    0, 0, 1, 1
};

static const bool SHAPE_L_LARGE_RIGHT[] = {
    0, 1, 0, 0,
    0, 1, 0, 0,
    0, 1, 0, 0,
    1, 1, 0, 0
};

static const bool SHAPE_CRUTCH_LEFT[] = {
    0, 1, 0, 0,
    1, 1, 0, 0,
    0, 1, 0, 0,
    0, 1, 0, 0
};

static const bool SHAPE_CRUTCH_RIGHT[] = {
    0, 0, 1, 0,
    0, 0, 1, 1,
    0, 0, 1, 0,
    0, 0, 1, 0
};

static const bool SHAPE_Z_LARGE_LEFT[] = {
    0, 1, 0, 0,
    1, 1, 0, 0,
    1, 0, 0, 0,
    1, 0, 0, 0
};

static const bool SHAPE_Z_LARGE_RIGHT[] = {
    0, 0, 1, 0,
    0, 0, 1, 1,
    0, 0, 0, 1,
    0, 0, 0, 1
};

static const bool SHAPE_S_LARGE[] = {
    0, 1, 0, 0,
    0, 1, 1, 0,
    0, 0, 1, 0,
    0, 0, 1, 0
};

// Size 5x5 (5 blocks)
static const bool SHAPE_VERTICAL_FIVE[] = {
    0, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    0, 0, 0, 0, 1
};

// ===== Piece Template Lookup Table =====
// Index 0 is unused (1-based indexing: pieces 1-29)
static const PieceTemplate PIECE_TEMPLATES[30] = {
    // [0] - Unused (padding for 1-based indexing)
    {0, 0, Qt::black},

    // [1] - Single block
    {1, SHAPE_SINGLE, Qt::blue},

    // [2-5] - 2x2 pieces
    {2, SHAPE_VERTICAL_TWO, Qt::red},
    {3, SHAPE_VERTICAL_THREE, Qt::red},
    {2, SHAPE_CORNER, Qt::lightGray},
    {2, SHAPE_SQUARE_TWO, Qt::blue},

    // [6-10] - 3x3 pieces (set 1)
    {3, SHAPE_L_LEFT, Qt::magenta},
    {3, SHAPE_L_RIGHT, Qt::lightGray},
    {3, SHAPE_Z_LEFT, Qt::cyan},
    {3, SHAPE_Z_RIGHT, Qt::green},
    {3, SHAPE_T, Qt::yellow},

    // [11-12] - Vertical 4 and 5
    {4, SHAPE_VERTICAL_FOUR, Qt::red},
    {5, SHAPE_VERTICAL_FIVE, Qt::red},

    // [13-18] - 4x4 pieces
    {4, SHAPE_L_LARGE_LEFT, Qt::magenta},
    {4, SHAPE_L_LARGE_RIGHT, Qt::yellow},
    {4, SHAPE_CRUTCH_RIGHT, Qt::yellow},
    {4, SHAPE_CRUTCH_LEFT, Qt::lightGray},
    {4, SHAPE_Z_LARGE_RIGHT, Qt::cyan},
    {4, SHAPE_Z_LARGE_LEFT, Qt::green},

    // [19-29] - 3x3 pieces (set 2)
    {3, SHAPE_CROSS, Qt::lightGray},
    {3, SHAPE_PLUS_RIGHT, Qt::magenta},
    {3, SHAPE_HOOK_LARGE, Qt::yellow},
    {3, SHAPE_U_LEFT, Qt::blue},
    {3, SHAPE_U_RIGHT, Qt::blue},
    {3, SHAPE_BRIDGE_LEFT, Qt::magenta},
    {3, SHAPE_BRIDGE_RIGHT, Qt::green},
    {3, SHAPE_W_LEFT, Qt::green},
    {3, SHAPE_W_RIGHT, Qt::cyan},
    {3, SHAPE_CORNER_LARGE, Qt::lightGray},
    {3, SHAPE_STAIRS, Qt::yellow}
};

// Total number of piece types (excluding index 0)
const int PIECE_COUNT = 29;

} // namespace PieceData

#endif // PIECEDATA_H
