#ifndef PLAYFIELD_H
#define PLAYFIELD_H

#include <qcolor.h>

class Piece;

class Playfield
{
public:
    Playfield(int cols = 12, int rows = 22);
    ~Playfield();

    int cols() const { return m_cols; }
    int rows() const { return m_rows; }

    bool canPlacePiece(const Piece *piece, int x, int y) const;
    void lockPiece(const Piece *piece);
    int clearFullLines();
    void clear();

    QColor cellColor(int row, int col) const;
    bool isEmpty(int row, int col) const;

private:
    int m_cols, m_rows;
    QColor **m_cells;

    void allocateField();
    void freeField();
};

#endif // PLAYFIELD_H
