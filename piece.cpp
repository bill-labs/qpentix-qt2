#include "piece.h"
#include "playfield.h"
#include <stdio.h>

Piece::Piece()
    : m_x(0), m_y(0), m_hardDropStartY(-1), m_size(0), m_shape(0), m_color(Qt::black)
{
}

Piece::Piece(int size, const bool *shapeData, const QColor &color)
    : m_x(0), m_y(0), m_hardDropStartY(-1), m_size(size), m_color(color)
{
    allocateShape();

    // Copy shape data
    for (int i = 0; i < m_size; i++) {
        for (int j = 0; j < m_size; j++) {
            m_shape[i][j] = shapeData[i * m_size + j];
        }
    }
}

Piece::~Piece()
{
    freeShape();
}

void Piece::allocateShape()
{
    if (m_size <= 0) return;

    m_shape = new bool*[m_size];
    for (int i = 0; i < m_size; i++) {
        m_shape[i] = new bool[m_size];
        for (int j = 0; j < m_size; j++) {
            m_shape[i][j] = false;
        }
    }
}

void Piece::freeShape()
{
    if (m_shape) {
        for (int i = 0; i < m_size; i++) {
            delete[] m_shape[i];
        }
        delete[] m_shape;
        m_shape = 0;
    }
}

void Piece::copyShape(bool **from, bool **to)
{
    for (int i = 0; i < m_size; i++) {
        for (int j = 0; j < m_size; j++) {
            to[i][j] = from[i][j];
        }
    }
}

void Piece::rotate()
{
    if (!m_shape || m_size <= 0) return;

    // Create temporary array for rotation
    bool **temp = new bool*[m_size];
    for (int i = 0; i < m_size; i++) {
        temp[i] = new bool[m_size];
    }

    // Rotate 90 degrees counter-clockwise: temp[n-j-1][i] = shape[i][j]
    for (int i = 0; i < m_size; i++) {
        for (int j = 0; j < m_size; j++) {
            temp[m_size - j - 1][i] = m_shape[i][j];
        }
    }

    // Copy back
    copyShape(temp, m_shape);

    // Free temporary array
    for (int i = 0; i < m_size; i++) {
        delete[] temp[i];
    }
    delete[] temp;
}

bool Piece::canMove(int dx, int dy, Playfield *playfield) const
{
    if (!playfield) return false;
    return playfield->canPlacePiece(this, m_x + dx, m_y + dy);
}

void Piece::move(int dx, int dy)
{
    m_x += dx;
    m_y += dy;
}

void Piece::hardDrop(Playfield *playfield)
{
    // Drop piece instantly
    if (!playfield) return;

    // Save position before the hard drop for score calculation
    m_hardDropStartY = m_y;

    while (playfield->canPlacePiece(this, m_x, m_y + 1)) {
        m_y++;
    }
}

bool Piece::cell(int row, int col) const
{
    if (!m_shape || row < 0 || row >= m_size || col < 0 || col >= m_size)
        return false;
    return m_shape[row][col];
}
