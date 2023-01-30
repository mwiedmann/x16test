#include "gamelevels.h"
#include "level.h"
#include "commontiles.h"

extern TileLayout cloudTiles1[];
extern TileLayout moonTiles[];

SolidLayout testLevelSolid[TEST_LEVEL_SOLID_LENGTH] = {
    { 7, 9, 1, 0},
    { 10, 8, 3, 0},
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

    { 10, 8, 3, 255, 0}, // Floor

    { 0, 10, 20, 254, 1}, // Floor
    { 19, 11, 1, 79, 1},
    { 6, 12, 14, 254, 1, 1, 0}, // Floor mirror
    { 5, 12, 1, 85, 1},
    { 5, 13, 1, 79, 1},
    { 5, 14, 1, 80, 1},
    { 0, 15, 6, 254, 1, 1, 0},

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

    { 8, 22, 2, 29, 1 },
    { 10, 22, 1, 30, 0 },
    { 11, 22, 1, 29, 1 },
    
    { 12, 20, 1, 34, 0 },
    { 12, 21, 1, 40, 0 },
    { 12, 22, 1, 46, 0 }, // Dead tree

    { 13, 22, 2, 30, 1 },
    { 15, 22, 2, 19, 0 },
    { 14, 22, 2, 29, 0 },

    { 7, 23, 25, 255, 1}, // Floor

    { 2, 22, 1, 14, 1}, // Tree start
    { 3, 22, 1, 15, 1},
    { 4, 22, 1, 16, 1},

    { 2, 23, 1, 20, 1},
    { 3, 23, 1, 21, 1},
    { 4, 23, 1, 22, 1},

    { 2, 24, 1, 26, 1},
    { 3, 24, 1, 27, 1},
    { 4, 24, 1, 28, 1},

    { 3, 25, 1, 42, 1},
    { 3, 26, 1, 42, 1},
    { 3, 27, 1, 42, 1},
    { 3, 28, 1, 48, 1}, // Tree End

    { 0, 29, 40, 255, 1} // Bottom
};

SolidLayoutList testLevelAllSolids[1] = {
    { TEST_LEVEL_SOLID_LENGTH, testLevelSolid }
};

TileLayoutList testLevelAllTiles[3] = {
    { TEST_LEVEL_TILES_LENGTH, testLevelTiles },
    { CLOUD_TILES_1_LENGTH, cloudTiles1 },
    { MOON_TILES_LENGTH, moonTiles }
};

EnemyLayout snakes[3] = {
    { 8, 9, 10, Snake, 0},
    { 7, 22, 25, Snake, 0},
    { 0, 28, 40, Snake, 0}
};

EnemyLayout bees[2] = {
    { 14, 15, 17, Bee, 0},
    { 0, 28, 40, Bee, 0}
};

EnemyLayout ghosts[2] = {
    { 8, 9, 12, Ghost, 0},
    { 7, 22, 25, Ghost, 0}
};

EnemyLayout scopions[1] = {
    { 14, 15, 17, Scorpion, 0}
};

EnemyLayout wasps[1] = {
    { 7, 22, 25, Wasp, 0}
};

EnemyLayoutList testLevelEnemies[5] = {
    { 3, snakes },
    { 2, bees },
    { 2, ghosts },
    { 1, scopions },
    { 1, wasps }
};

LevelOveralLayout testLevel = {
    1,
    testLevelAllSolids,
    3,
    testLevelAllTiles,
    5,
    testLevelEnemies
};