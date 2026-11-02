#ifndef GAMECONFIG_H
#define GAMECONFIG_H

/**
 * GameConfig.h
 *
 * Central configuration for Pentix game constants.
 * All magic numbers and tunable parameters are defined here.
 */

namespace GameConfig {
    // ===== Playfield Dimensions =====
    const int PLAYFIELD_COLUMNS = 12;
    const int PLAYFIELD_ROWS = 22;
    const int CELL_SIZE = 20;  // pixels per cell

    // ===== Window Dimensions =====
    const int WINDOW_WIDTH = 560;
    const int WINDOW_HEIGHT = 520;

    // ===== Layout Offsets =====
    const int PLAYFIELD_OFFSET_X = 120;  // X position of playfield
    const int PLAYFIELD_OFFSET_Y = 20;   // Y position of playfield
    const int STATS_OFFSET_X = 10;   // X position of stats panel
    const int STATS_OFFSET_Y = 80;   // Y position of stats panel

    // ===== Scoring Constants =====
    // Score formula: BASE_SCORE + drop_distance + (LEVEL_MULTIPLIER * level)
    const int BASE_SCORE = 5;
    const int LEVEL_MULTIPLIER = 3;
    const int NEXT_PREVIEW_PENALTY = 5;  // Score penalty for showing next piece

    // ===== Level System =====
    const int MAX_LEVEL = 9;
    const int LINES_PER_LEVEL = 10;

    // Drop speeds (milliseconds) for each level 0-9
    // Speeds get progressively faster as level increases
    const int LEVEL_SPEEDS[MAX_LEVEL + 1] = {
        1900,  // Level 0
        1600,  // Level 1
        1200,  // Level 2
        1000,  // Level 3
        850,   // Level 4
        600,   // Level 5
        390,   // Level 6
        252,   // Level 7
        140,   // Level 8
        100    // Level 9
    };

    // ===== Font Configuration =====
    // Font filenames (full paths resolved at runtime via FONTPATH)
    // Primary font (used for title and labels)
    const char* FONT_PRIMARY_NAME = "Orbitron-VariableFont_wght.ttf";
    const int FONT_LABEL_SIZE = 14;
    const int FONT_TITLE_SIZE = 72;

    // Secondary font (used for numbers and credits)
    const char* FONT_SECONDARY_NAME = "DejaVuSans-Bold.ttf";
    const int FONT_NUMBER_SIZE = 18;

    // Fallback font (if primary/secondary fail to load)
    const char* FONT_FALLBACK_NAME = "DejaVuSans.ttf";
    const int FONT_FALLBACK_SIZE = 14;
    const int FONT_CREDITS_SIZE = 16;
    const int FONT_INSTRUCTIONS_SIZE = 12;

    // ===== Rendering Settings =====
    const float GRADIENT_DARKNESS = 0.3f;  // Cell gradient darkness factor (0.0-1.0)
    const int CELL_CACHE_SIZE = 10;        // Number of pre-rendered cell colors

    // ===== Splash Screen Layout =====
    // Title letter X positions (for "PENTIX")
    const int TITLE_LETTER_POSITIONS[6] = {70, 130, 185, 245, 315, 360};
    const int TITLE_Y_POSITION = 170;

    // Credits section
    const int CREDITS_LINE1_X = 125;
    const int CREDITS_LINE1_Y = 310;
    const int CREDITS_LINE2_X = 105;
    const int CREDITS_LINE2_Y = 335;

    // Instructions section
    const int INSTRUCTIONS_LINE1_X = 125;
    const int INSTRUCTIONS_LINE1_Y = 380;
    const int INSTRUCTIONS_LINE2_X = 140;
    const int INSTRUCTIONS_LINE2_Y = 405;
    const int INSTRUCTIONS_LINE3_X = 110;
    const int INSTRUCTIONS_LINE3_Y = 435;
    const int INSTRUCTIONS_LINE4_X = 160;
    const int INSTRUCTIONS_LINE4_Y = 460;
    const int INSTRUCTIONS_LINE5_X = 150;
    const int INSTRUCTIONS_LINE5_Y = 485;

    // ===== Next Piece Preview =====
    const int NEXT_PREVIEW_X_OFFSET = 1;   // Cells from right edge of playfield
    const int NEXT_PREVIEW_Y_OFFSET = -40; // Pixels from playfield center
    const int NEXT_LABEL_X_OFFSET = 20;    // Pixels from preview label
    const int NEXT_LABEL_Y_OFFSET = 35;    // Pixels from preview label

    // ===== Stats Panel Layout =====
    const int STATS_LEVEL_Y = 80;
    const int STATS_LINES_Y = 150;
    const int STATS_SCORE_Y = 220;
    const int STATS_NUMBER_X_OFFSET = 5;
    const int STATS_NUMBER_Y_OFFSET = 25;

    // ===== Bottom Message =====
    const int BOTTOM_MESSAGE_X = 190;
    const int BOTTOM_MESSAGE_Y = 495;
}

#endif // GAMECONFIG_H
