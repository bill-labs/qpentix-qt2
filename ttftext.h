#ifndef TTFTEXT_H
#define TTFTEXT_H

#include <qpixmap.h>
#include <qpainter.h>
#include <qcolor.h>
#include <ft2build.h>
#include FT_FREETYPE_H

// Simple TTF text renderer using FreeType 2
class TtfText
{
public:
    TtfText();
    ~TtfText();

    // Load TTF font file
    bool loadFont(const char *fontPath, int pixelSize);

    // Render text to painter at specified position
    void drawText(QPainter &painter, int x, int y, const char *text, const QColor &color);

    // Get text width for layout calculations
    int textWidth(const char *text);

private:
    FT_Library m_library;
    FT_Face m_face;
    bool m_initialized;
};

#endif // TTFTEXT_H
