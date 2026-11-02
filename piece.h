#ifndef PIECE_H
#define PIECE_H

#include <qcolor.h>

class Playfield;

class Piece
{
public:
    Piece();
    Piece(int size, const bool *shapeData, const QColor &color);
    ~Piece();

    void rotate();
    bool canMove(int dx, int dy, Playfield *playfield) const;
    void move(int dx, int dy);
    void hardDrop(Playfield *playfield);  // Hard drop (Space key)

    int x() const { return m_x; }
    int y() const { return m_y; }
    int size() const { return m_size; }
    bool cell(int row, int col) const;
    QColor color() const { return m_color; }
    int hardDropStartY() const { return m_hardDropStartY; }

    void setPosition(int x, int y) { m_x = x; m_y = y; }
    void setHardDropStartY(int y) { m_hardDropStartY = y; }

private:
    int m_x, m_y;
    int m_hardDropStartY;  // Position before hard drop, -1 if no hard drop
    int m_size;
    bool **m_shape;
    QColor m_color;

    void allocateShape();
    void freeShape();
    void copyShape(bool **from, bool **to);
};

#endif // PIECE_H
