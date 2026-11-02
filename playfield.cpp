#include "playfield.h"
#include "piece.h"
#include <stdio.h>

Playfield::Playfield(int cols, int rows)
    : m_cols(cols), m_rows(rows), m_cells(0)
{
    allocateField();
    clear();
}

Playfield::~Playfield()
{
    freeField();
}

void Playfield::allocateField()
{
    m_cells = new QColor*[m_rows];
    for (int i = 0; i < m_rows; i++) {
        m_cells[i] = new QColor[m_cols];
    }
}

void Playfield::freeField()
{
    if (m_cells) {
        for (int i = 0; i < m_rows; i++) {
            delete[] m_cells[i];
        }
        delete[] m_cells;
        m_cells = 0;
    }
}

void Playfield::clear()
{
    for (int i = 0; i < m_rows; i++) {
        for (int j = 0; j < m_cols; j++) {
            m_cells[i][j] = Qt::black;
        }
    }
}

bool Playfield::canPlacePiece(const Piece *piece, int x, int y) const
{
    if (!piece) return false;

    // X coordinate is 1-based, Y is 0-based

    for (int i = 0; i < piece->size(); i++) {
        for (int j = 0; j < piece->size(); j++) {
            if (!piece->cell(i, j)) continue;

            int fieldRow = y + i;
            int fieldCol = x + j - 1;  // x is 1-based, subtract 1 for 0-based array

            // Check bounds
            if (fieldCol < 0 || fieldCol >= m_cols) return false;
            if (fieldRow < 0) continue; // Allow the piece to be partially above the playfield
            if (fieldRow >= m_rows) return false;

            // Check collision with existing blocks
            if (m_cells[fieldRow][fieldCol] != Qt::black) return false;
        }
    }

    return true;
}

void Playfield::lockPiece(const Piece *piece)
{
    if (!piece) return;

    // After collision detection, we roll back with move(0, -1)
    // So use the same indexing as canPlacePiece()

    for (int i = 0; i < piece->size(); i++) {
        for (int j = 0; j < piece->size(); j++) {
            if (!piece->cell(i, j)) continue;

            // Use the same indexing as canPlacePiece(): [y + i][x + j - 1]
            int fieldRow = piece->y() + i;      // No -1 because we already rolled back
            int fieldCol = piece->x() + j - 1;  // x is 1-based

            if (fieldRow >= 0 && fieldRow < m_rows &&
                fieldCol >= 0 && fieldCol < m_cols) {
                m_cells[fieldRow][fieldCol] = piece->color();
            }
        }
    }
}

int Playfield::clearFullLines()
{
    int removed = 0;

    for (int row = m_rows - 1; row >= 0; row--) {
        bool full = true;
        for (int col = 0; col < m_cols; col++) {
            if (m_cells[row][col] == Qt::black) {
                full = false;
                break;
            }
        }

        if (full) {
            removed++;
            // Move all rows above down
            for (int r = row; r > 0; r--) {
                for (int col = 0; col < m_cols; col++) {
                    m_cells[r][col] = m_cells[r - 1][col];
                }
            }
            // Clear top row
            for (int col = 0; col < m_cols; col++) {
                m_cells[0][col] = Qt::black;
            }
            row++; // Check this row again
        }
    }

    return removed;
}

QColor Playfield::cellColor(int row, int col) const
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols)
        return Qt::black;
    return m_cells[row][col];
}

bool Playfield::isEmpty(int row, int col) const
{
    return cellColor(row, col) == Qt::black;
}
