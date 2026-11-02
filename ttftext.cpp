#include "ttftext.h"
#include <stdio.h>
#include <string.h>

TtfText::TtfText()
    : m_library(0), m_face(0), m_initialized(false)
{
    // Initialize FreeType library
    if (FT_Init_FreeType(&m_library) != 0) {
        fprintf(stderr, "ERROR: Failed to initialize FreeType\n");
        return;
    }
    m_initialized = true;
}

TtfText::~TtfText()
{
    if (m_face) {
        FT_Done_Face(m_face);
    }
    if (m_library) {
        FT_Done_FreeType(m_library);
    }
}

bool TtfText::loadFont(const char *fontPath, int pixelSize)
{
    if (!m_initialized) return false;

    if (m_face) {
        FT_Done_Face(m_face);
        m_face = 0;
    }

    // Load font file
    if (FT_New_Face(m_library, fontPath, 0, &m_face) != 0) {
        fprintf(stderr, "ERROR: Failed to load font: %s\n", fontPath);
        return false;
    }

    // Set pixel size
    if (FT_Set_Pixel_Sizes(m_face, 0, pixelSize) != 0) {
        fprintf(stderr, "ERROR: Failed to set font size\n");
        FT_Done_Face(m_face);
        m_face = 0;
        return false;
    }

    return true;
}

void TtfText::drawText(QPainter &painter, int x, int y, const char *text, const QColor &color)
{
    if (!m_face || !text) return;

    int penX = x;
    int penY = y;

    // Render each character
    for (const char *p = text; *p; p++) {
        // Load glyph
        if (FT_Load_Char(m_face, *p, FT_LOAD_RENDER) != 0) {
            continue;
        }

        FT_GlyphSlot slot = m_face->glyph;
        FT_Bitmap &bitmap = slot->bitmap;

        // Calculate position
        int bx = penX + slot->bitmap_left;
        int by = penY - slot->bitmap_top;

        // Draw bitmap pixel by pixel
        int rows = (int)bitmap.rows;
        int cols = (int)bitmap.width;
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                unsigned char alpha = bitmap.buffer[row * bitmap.pitch + col];
                if (alpha > 0) {
                    // Blend with background using alpha
                    QColor blended = color;
                    if (alpha < 255) {
                        // Simple alpha blending (could be improved)
                        blended = QColor(
                            (color.red() * alpha) / 255,
                            (color.green() * alpha) / 255,
                            (color.blue() * alpha) / 255
                        );
                    }
                    painter.setPen(blended);
                    painter.drawPoint(bx + col, by + row);
                }
            }
        }

        // Advance pen position
        penX += slot->advance.x >> 6;
    }
}

int TtfText::textWidth(const char *text)
{
    if (!m_face || !text) return 0;

    int width = 0;
    for (const char *p = text; *p; p++) {
        if (FT_Load_Char(m_face, *p, FT_LOAD_DEFAULT) != 0) {
            continue;
        }
        width += m_face->glyph->advance.x >> 6;
    }
    return width;
}
