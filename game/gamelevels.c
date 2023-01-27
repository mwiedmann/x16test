#include "gamelevels.h"

LevelLayout testLevelSolid[TEST_LEVEL_SOLID_LENGTH] = {
    { 7, 9, 1, 0},
    { 0, 10, 20, 0},
    { 14, 16, 17, 0},
    { 7, 23, 25, 0},
    { 0, 29, 40, 0}
};

TileLayout testLevelTiles[TEST_LEVEL_TILES_LENGTH] = {
    { 7, 7, 1, 36, 0},
    { 7, 8, 1, 42, 0},
    { 7, 9, 1, 48, 0}, // Cactus blocks
    { 13, 9, 1, 6, 1},
    { 15, 9, 1, 19, 0},
    { 0, 10, 20, 255, 1}, // Floor
    { 16, 15, 1, 17, 1},
    { 23, 15, 1, 49, 0},
    { 24, 15, 1, 50, 1},


    { 27, 11, 1, 31, 0}, // Tree start
    { 28, 11, 1, 32, 0},
    { 29, 11, 1, 33, 0},

    { 27, 12, 1, 37, 0},
    { 28, 12, 1, 38, 0},
    { 29, 12, 1, 39, 0},

    { 27, 13, 1, 43, 0},
    { 28, 13, 1, 44, 0},
    { 29, 13, 1, 45, 0},

    { 28, 14, 1, 42, 0},
    { 28, 15, 1, 48, 0}, // Tree End

    { 14, 16, 17, 255, 1}, // Floor
    { 7, 23, 25, 255, 1}, // Floor

    { 0, 29, 40, 255, 1} // Bottom
};

TileLayout cloudTiles1[CLOUD_TILES_1_LENGTH] = {
    { 2, 2, 1, 7, 0},

    { 15, 3, 1, 8, 0},
    { 16, 3, 1, 9, 0},

    { 24, 2, 1, 10, 0},
    { 25, 2, 1, 11, 0},

    { 9, 5, 1, 12, 0},
    { 10, 5, 1, 13, 0},

    { 20, 7, 1, 12, 0},
    { 21, 7, 1, 13, 0}
};