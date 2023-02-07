#include <cx16.h>
#include <joystick.h>
#include <string.h>
#include <stdlib.h>

// Libs
#include "x16graphics.h"
#include "sprites.h"

// Game specific
#include "startup.h"
#include "gametiles.h"
#include "gamesprites.h"
#include "memmap.h"
#include "level.h"
#include "layoutdefs.h"
#include "welcome.h"
#include "levelmgr.h"

// The "waitvsync" function is broken in r41
// People say to use this until fixed
#include "waitforjiffy.h"

#define PLAYER_FALL_SPEED 20
#define PLAYER_WATER_FALL_SPEED 4

#define PLAYER_JUMP_SPEED 20
#define PLAYER_WATER_JUMP_SPEED 12

#define PLAYER_JUMP_FRAMES 16
#define PLAYER_WATER_JUMP_FRAMES 16

// Import the levels
// extern GameLayout gameLayout;
// extern LevelOveralLayout level1;

// TODO: Fixed array to hold AISprites
// Need something more dynamic but this works for now
AISprite masterEnemiesList[16];
Sprite enemyLasers[16];

Sprite player, bullet, expSmall;
LevelOveralLayout* level;

void layerMapsAddSomeStuff(LevelOveralLayout *level) {
    unsigned short x, y;
    
    vMemSetIncMode(1);

    // Clear Layer 0 (background)
    vMemSetBank(LAYER0_MAP_MEM_BANK);
    vMemSetAddr(LAYER0_MAP_MEM);
    for (y=0; y<32; y++) {
        for (x=0; x<64; x++) {
            vMemSetData0(0);
            vMemSetData0(0);
        }
    }

    // Clear Layer 1 (foreground)
    vMemSetBank(LAYER1_MAP_MEM_BANK);
    vMemSetAddr(LAYER1_MAP_MEM);
    for (y=0; y<32; y++) {
        for (x=0; x<64; x++) {     
            vMemSetData0(0);
            vMemSetData0(0);
        }
    }

    addLevelTiles(level->tilesList->length, level->tilesList->tiles);
}

Exit *playerTouchingExit(LevelOveralLayout *level, Sprite *sprite) {
    unsigned char i,x,y;
    
    x = ((sprite->x + TILE_PIXEL_WIDTH_HALF) / TILE_PIXEL_WIDTH);
    y = ((sprite->y + TILE_PIXEL_HEIGHT_HALF) / TILE_PIXEL_HEIGHT);

    for (i=0; i<level->exitList->length; i++) {
        if (y == level->exitList->exits[i].y &&
            x == level->exitList->exits[i].x) {
                return &level->exitList->exits[i];
            }
    }

    return 0;
}

void spriteTouchingTile(LevelOveralLayout *level, Sprite *sprite, TileInfo *tileCollision) {
    tileCollision->x = ((sprite->x + TILE_PIXEL_WIDTH_HALF) / TILE_PIXEL_WIDTH);
    tileCollision->y = (sprite->y + pixelSizes[sprite->height]) / TILE_PIXEL_HEIGHT;

    tileCollision->type = ((unsigned char[30][40])level->movementTypes)[tileCollision->y][tileCollision->x];
}

unsigned char enemiesCreate(LevelOveralLayout *level, AISprite enemies[], unsigned char nextSpriteIndex) {
    unsigned char i;
    void (*enemyCreate[])(AISprite*, EnemyLayout*, unsigned char) = {
        snakeCreate, beeCreate, ghostCreate, scorpionCreate, waspCreate, fish1Create, bigGhostCreate
    };

    for (i=0; i<level->enemiesList->length; i++) {
        (*enemyCreate[level->enemiesList->enemies[i].enemyType])(
            &enemies[i], &level->enemiesList->enemies[i], nextSpriteIndex+i
        );
    }

    return level->enemiesList->length;
}

void enemiesReset(AISprite enemies[], unsigned char length) {
    unsigned char i;
    AISprite *enemy;
    Sprite *laser;

    // Reset enemies
    for (i=0; i<length; i++) {
        enemy = &enemies[i];
        spriteMove(&enemy->sprite, 0, 0);
        enemy->sprite.active = 0;
        enemy->sprite.zDepth = Disabled;
        x16SpriteIdxSetZDepth(enemy->sprite.index, Disabled);
        x16SpriteIdxSetXY(enemy->sprite.index, enemy->sprite.x, enemy->sprite.y);
    }

    // Reset lasers
    for (i=0; i<16; i++) {
        laser = &enemyLasers[i];
        spriteMove(laser, 0, 0);
        laser->active = 0;
        laser->zDepth = Disabled;
        x16SpriteIdxSetZDepth(laser->index, Disabled);
        x16SpriteIdxSetXY(laser->index, laser->x, laser->y);
    }
}

unsigned char playerNear(short x, short y) {
    return (abs(player.x-x) <= 170 && abs(player.y-y)<=40) ? 1 : 0;
}

unsigned char facePlayer(short x) {
    return x > player.x ? 0 : 1;
}

void enemyShot(short x, short y, unsigned char direction) {
    unsigned char i;
    Sprite *laser;

    for (i=0; i<16; i++) {
        laser = &enemyLasers[i];

        if (laser->active == 0) {
            laser->active = 1;
            laser->animationDirection = direction;
            laser->zDepth = BetweenL0L1;
            laser->lastTileX=0;
            laser->lastTileY=0;
            spriteMove(laser, x, y);
            x16SpriteIdxSetXY(laser->index, laser->x, laser->y);
            x16SpriteIdxSetHFlip(laser->index, laser->animationDirection);
            x16SpriteIdxSetZDepth(laser->index, laser->zDepth);
            return;
        }
    }
}

void resetClosestLaser(short x, short y) {
    unsigned char i;
    Sprite *laser;

    for (i=0; i<16; i++) {
        laser = &enemyLasers[i];

        if (laser->active == 1 && abs(laser->x-x) <= TILE_PIXEL_WIDTH && abs(laser->y-y) <= TILE_PIXEL_HEIGHT) {
            break;
        } else {
            laser = 0;
        }
    }

    if (laser) {
        laser->active = 0;
        laser->zDepth = Disabled;
        spriteMove(laser, 0, 0);
        x16SpriteIdxSetZDepth(laser->index, laser->zDepth);
        x16SpriteIdxSetXY(laser->index, laser->x, laser->y);
    }
}

void enemiesMove(AISprite enemies[], unsigned char length) {
    unsigned char i;
    signed char tileCalc;
    AISprite *enemy;

    // Move enemies
    for (i=0; i<length; i++) {
        enemy = &enemies[i];
        if (enemy->sprite.active == 1) {
            spriteMoveXL(&enemy->sprite, enemy->sprite.animationDirection == 0 ? enemy->sprite.xL-enemy->sprite.speed : enemy->sprite.xL+enemy->sprite.speed);
            enemy->sprite.animationCount++;
            if (enemy->sprite.animationCount == enemy->sprite.animationSpeed) {
                enemy->sprite.animationCount=0;
                enemy->sprite.animationFrame++;
                if (enemy->sprite.animationFrame == enemy->sprite.frames) {
                    enemy->sprite.animationFrame = 0;
                }
                x16SpriteIdxSetGraphicsPointer(enemy->sprite.index, enemy->sprite.clrMode, enemy->sprite.graphicsBank,
                    enemy->sprite.graphicsAddress+(enemy->sprite.animationFrame * enemy->sprite.frameSize));
            }
            tileCalc = enemy->sprite.x / TILE_PIXEL_WIDTH;
            // Careful, can be -1 if on left edge (signed char)
            if (tileCalc <= (signed char)enemy->xTileStart - 1) {
                enemy->sprite.animationDirection = 1;
                x16SpriteIdxSetHFlip(enemy->sprite.index, enemy->sprite.animationDirection);
            } else if (tileCalc >= enemy->xTileEnd) {
                enemy->sprite.animationDirection = 0;
                x16SpriteIdxSetHFlip(enemy->sprite.index, enemy->sprite.animationDirection);
            }
            x16SpriteIdxSetXY(enemy->sprite.index, enemy->sprite.x, enemy->sprite.y);

            if (playerNear(enemy->sprite.x, enemy->sprite.y)) {
                // Player is near, face him
                if (enemy->framesUntilFacePlayer == 0) {
                    // Can't face player again until this hits 0
                    enemy->framesUntilFacePlayer = enemy->framesBetweenFacePlayer;

                    // Flip the enemy towards the player
                    enemy->sprite.animationDirection = facePlayer(enemy->sprite.x);
                    x16SpriteIdxSetHFlip(enemy->sprite.index, enemy->sprite.animationDirection);
                }
                // Shoot
                if (enemy->framesUntilNextShot == 0) {
                    enemy->framesUntilNextShot = enemy->framesBetweenShots;
                    // HACK: Move y 1px up to avoid ground collision
                    enemyShot(enemy->sprite.x, (enemy->sprite.y + enemy->yLaserAdjust)-1, enemy->sprite.animationDirection);
                } else {
                    enemy->framesUntilNextShot--;
                }

                // Enemy can face the player again when this hits 0
                enemy->framesUntilFacePlayer--;
            } else {
                // Can face the player immediately next time in range
                enemy->framesUntilFacePlayer = 0;
            }
        }
    }
}

void enemyLasersMove() {
    unsigned char i;
    TileInfo tileCollision;
    Sprite *laser;

    for (i=0; i<16; i++) {
        laser = &enemyLasers[i];
        if (laser->active == 1) {
            spriteMoveX(laser, laser->animationDirection == 0 ? laser->x-laser->speed : laser->x+laser->speed);
            x16SpriteIdxSetXY(laser->index, laser->x, laser->y);
            
            spriteTouchingTile(level, laser, &tileCollision);
            if (tileCollision.type == Ground || laser->x < 0 || laser->x > 639) {
                // TODO: Explosion for enemy lasers?
                // Need more explosion sprites
                // if (tileCollision.type == Ground) {
                //     // Explosion
                //     smallExplosion(&expSmall, InFrontOfL1, laser->x, laser->y);
                // }
                laser->active = 0;
                laser->zDepth = Disabled;
                spriteMove(laser, 0, 0);
                x16SpriteIdxSetZDepth(laser->index, laser->zDepth);
                x16SpriteIdxSetXY(laser->index, laser->x, laser->y);
            }
        }
    }
}

void smallExplosion(Sprite *expSmall, ZDepth zDepth, short x, short y) {
    // Explosion
    spriteMove(expSmall, x, y);
    x16SpriteIdxSetGraphicsPointer(expSmall->index, expSmall->clrMode, expSmall->graphicsBank, expSmall->graphicsAddress);
    x16SpriteIdxSetXY(expSmall->index, expSmall->x, expSmall->y);
    expSmall->zDepth = zDepth;
    x16SpriteIdxSetZDepth(expSmall->index, expSmall->zDepth);
    expSmall->animationCount = 0;
    expSmall->animationFrame = 0;
}

AISprite *findEnemyCollision(Sprite *s) {
    unsigned char i;
    AISprite *enemy;

    for (i=0; i<16; i++) {
        enemy = &masterEnemiesList[i];
        // TODO: Use the actual sprite width (need conversion function from)
        if (doOverlap(
            s->x, s->y, pixelSizes[s->width], pixelSizes[s->height],
            enemy->sprite.x, enemy->sprite.y, pixelSizes[enemy->sprite.width], pixelSizes[enemy->sprite.height])) {
                return enemy;
            }
    }

    return 0;
}

Entrance *findEntranceForExit(EntranceList *entranceList, unsigned char entranceId) {
    unsigned char i;

    for (i=0; i<entranceList->length; i++) {
        if (entranceList->entrances[i].id == entranceId) {
            return &entranceList->entrances[i];
        }
    }

    return 0;
}

Exit* runLevel(unsigned char nextSpriteIndex) {
    unsigned char collision, joy, enemyCount;
    unsigned char jumpFrames = 0;
    unsigned char releasedBtnAfterJump = 1;
    
    TileInfo tileCollision;
    Exit *exitCollision;
    AISprite *hitEnemy;
    Entrance *entrance;

    enemyCount = enemiesCreate(level, masterEnemiesList, nextSpriteIndex);
    nextSpriteIndex+= enemyCount;

    while (1) {
        waitforjiffy();

        // See if player is touching an exit
        exitCollision = playerTouchingExit(level, &player);
        if (exitCollision != 0) {
            // If this jumps somewhere on the same level, just move the player
            if (exitCollision->level == level->levelNum) {
                entrance = findEntranceForExit(level->entranceList, exitCollision->entranceId);
                spriteMoveToTile(&player, entrance->x, entrance->y, TILE_PIXEL_WIDTH, TILE_PIXEL_HEIGHT);    
                x16SpriteIdxSetXY(player.index, player.x, player.y);
            } else {
                // This is jumping to another level. We need to cleanup this level.
                // Clean up the enemies and return the exit info
                enemiesReset(masterEnemiesList, enemyCount);

                // Reset any active bullets
                bullet.active = 0;
                bullet.zDepth = Disabled;
                x16SpriteIdxSetZDepth(bullet.index, bullet.zDepth);

                return exitCollision;
            }
        }

        enemiesMove(masterEnemiesList, enemyCount);
        enemyLasersMove();

        // Count game loops so we can animate sprites.
        // Only animate if the guy is "going" somewhere.
        player.going=0;
        player.animationCount++;

        // We are changing the guys animation every 6 game loops
        // but hold at this count as we only animate if moving.
        if (player.animationCount > player.animationSpeed) {
            player.animationCount=player.animationSpeed;
        }

        joy = joy_read(0);

        // Falling
        // jumpFrames are the number of frames the player moves UP (jumps)
        // At 0, the player is always falling, even if on the ground.
        if (jumpFrames == 0) {
            // See what the player is currently touching to see his fall (or swim-fall) speed
            spriteTouchingTile(level, &player, &tileCollision);

            // The player falls in water too, just more slowly
            spriteMoveYL(&player, player.yL + (tileCollision.type == Empty ? PLAYER_FALL_SPEED : PLAYER_WATER_FALL_SPEED));
            spriteTouchingTile(level, &player, &tileCollision);

            // If the player is standing on a tile, a few things happen
            if (tileCollision.type != Empty) {
                // Move the player to the top of the tile
                // We do this because as the player falls they may end up wedged a few pixels into a tile
                if (tileCollision.type == Ground) {
                    spriteMoveY(&player, ((tileCollision.y * TILE_PIXEL_HEIGHT) - pixelSizes[player.height]) - 1);
                }

                // Player is touching something, see if they pressed jump button
                if (JOY_BTN_1(joy) || JOY_UP(joy)) {
                    // Only let them jump if they released the jump button since the last jump.
                    // Without this, the player just hops as you hold the button.
                    if (releasedBtnAfterJump == 1) {
                        jumpFrames = tileCollision.type == Water ? PLAYER_WATER_JUMP_FRAMES : PLAYER_JUMP_FRAMES;
                        releasedBtnAfterJump = 0;
                        // The jump animation wasn't great. The normal animation is fine
                    }
                } else {
                    // Player is on ground and isn't holding the button
                    // They can jump again next game loop.
                    releasedBtnAfterJump = 1;
                }
            }
        } else {
            // Player is jumping, move them up
            jumpFrames--;
            player.going==1;
            spriteMoveYL(&player, player.yL-(tileCollision.type == Water ? PLAYER_WATER_JUMP_SPEED : PLAYER_JUMP_SPEED));
            spriteTouchingTile(level, &player, &tileCollision);
            if (tileCollision.type == Ground) {
                spriteMoveBackY(&player);
            }
        }

        // Check the final tile to see if the graphics need to change to scuba or back to running
        spriteTouchingTile(level, &player, &tileCollision);
        if ((tileCollision.type == Ground || tileCollision.type == Empty) && player.graphicsAddress != SPRITE_MEM_PLAYER) {
            player.graphicsAddress = SPRITE_MEM_PLAYER;
            x16SpriteIdxSetGraphicsPointer(player.index, player.clrMode, player.graphicsBank,
                player.graphicsAddress);
        } else if (tileCollision.type == Water && player.graphicsAddress != SPRITE_MEM_PLAYER_SCUBA) {
            player.graphicsAddress = SPRITE_MEM_PLAYER_SCUBA;
            x16SpriteIdxSetGraphicsPointer(player.index, player.clrMode, player.graphicsBank,
                player.graphicsAddress);
        }

        // Check if player is moving left/right
        if (JOY_LEFT(joy)) {
            player.going=1;
            // We also flip the animation depending on direction
            if (player.animationDirection != 0) {
                player.animationDirection=0;
                x16SpriteIdxSetHFlip(player.index, player.animationDirection);
            }
            spriteMoveXL(&player, player.xL-(tileCollision.type == Water ? player.swimSpeed : player.speed));
        } else if (JOY_RIGHT(joy)) {
            player.going=1;
            // Maybe flip animation
            if (player.animationDirection != 1) {
                player.animationDirection=1;
                x16SpriteIdxSetHFlip(player.index, player.animationDirection);
            }
            spriteMoveXL(&player, player.xL+(tileCollision.type == Water ? player.swimSpeed : player.speed));
        }

        // Change animation if jumping or moving and hit loop count
        if (jumpFrames > 0) {
            // No animation change because it looked dumb
        } else if (player.going==1 && player.animationCount == player.animationSpeed) {
            player.animationCount=0;
            player.animationFrame++;
            if (player.animationFrame == player.frames) {
                player.animationFrame = 0;
            }
            x16SpriteIdxSetGraphicsPointer(player.index, player.clrMode, player.graphicsBank,
                player.graphicsAddress+(player.animationFrame * player.frameSize));
        } else if (player.animationCount == player.animationSpeed && player.animationFrame != player.animationStopFrame) {
            // If the guy is standing still, always show a certain frame
            // In future this could be a totally different animation
            // We have a yawn animation for instance for when waiting too long.
            player.animationFrame = player.animationStopFrame;
            x16SpriteIdxSetGraphicsPointer(player.index, player.clrMode, player.graphicsBank,
                player.graphicsAddress+(player.animationFrame * player.frameSize));
        }
        
        // Move the bullet
        if (bullet.active == 1) {
            spriteMoveX(&bullet, bullet.animationDirection == 0 ? bullet.x-bullet.speed : bullet.x+bullet.speed);
            x16SpriteIdxSetXY(bullet.index, bullet.x, bullet.y);
        }

        // See if player is shooting
        if (JOY_BTN_2(joy) && bullet.active==0) {
            bullet.active = 1;
            bullet.animationDirection = player.animationDirection;
            bullet.startX = player.x;
            spriteMove(&bullet, player.x, player.y);
            // Show the bullet
            bullet.zDepth = BetweenL0L1;
            x16SpriteIdxSetZDepth(bullet.index, bullet.zDepth);
            x16SpriteIdxSetXY(bullet.index, bullet.x, bullet.y);
            x16SpriteIdxSetHFlip(bullet.index, bullet.animationDirection);
        }

        // See if the player is touching any tiles left/right
        // Move back if so.
        spriteTouchingTile(level, &player, &tileCollision);
        if (tileCollision.type == Ground) {
            spriteMoveBackX(&player);
        }

        // Final position of the player. Update their sprite
        x16SpriteIdxSetXY(player.index, player.x, player.y);

        // Manage explosion sprites
        expSmall.animationCount++;
        if (expSmall.animationCount == expSmall.animationSpeed) {
            expSmall.animationCount=0;
            expSmall.animationFrame++;
            x16SpriteIdxSetGraphicsPointer(expSmall.index, expSmall.clrMode, expSmall.graphicsBank,
                expSmall.graphicsAddress+(expSmall.animationFrame * expSmall.frameSize));

            if (expSmall.animationFrame == expSmall.frames) {
                expSmall.zDepth = Disabled;
                x16SpriteIdxSetZDepth(expSmall.index, expSmall.zDepth);
            }
        }
            
        // Get the Collision bits and shift them down
        collision = x16SpriteCollisionBitsGet();

        // The collision bits will be the OVERLAP of the Collision Masks of the two sprites.
        // We can then look at all the sprites that have that bit in their mask
        // NOTE: Collisions seem to be triggered twice.
        // Could be because there are 2 spries involved?
        // OR maybe at this point we haven't Disabled the sprite yet and its too late?
        // HACK: I check the "active" status on the bullet before considering this a valid collision

        // Player and Enemy/Laser collisions
        if (collision == 0b1001 || collision == 0b0101) {
            // TODO: If laser hit, find and reset that laser
            // Right now it just kills the player so its ok if it continues and dies when it runs out of steam
            resetClosestLaser(player.x, player.y);

            // TODO: Getting some false collisions, check the collisions here to make sure its valid
            // Move the sprite back to the start
            spriteMoveToTile(&player, level->entranceList->entrances[0].x, level->entranceList->entrances[0].y, TILE_PIXEL_WIDTH, TILE_PIXEL_HEIGHT);
            x16SpriteIdxSetXY(player.index, player.x, player.y);

            
        } else if (bullet.active == 1 && collision == 0b1010) {
            // Explosion
            smallExplosion(&expSmall, BetweenL0L1, bullet.x, bullet.y);
            
            // Find the enemy
            hitEnemy = findEnemyCollision(&bullet);
            if (hitEnemy != 0) {
                hitEnemy->health--;
                if (hitEnemy->health == 0) {
                    hitEnemy->sprite.active = 0;
                    hitEnemy->sprite.zDepth = Disabled;
                    x16SpriteIdxSetZDepth(hitEnemy->sprite.index, hitEnemy->sprite.zDepth);
                }
            }

            // Bullet hit an enemy
            bullet.active = 0;
            bullet.zDepth = Disabled;
            x16SpriteIdxSetXY(bullet.index, 0, 0);
            x16SpriteIdxSetZDepth(bullet.index, bullet.zDepth);
        }

        // Bullet is off screen or collided with a solid tile
        if (bullet.active == 1) {
            spriteTouchingTile(level, &bullet, &tileCollision);
            if (tileCollision.type == Ground || bullet.x < 0 || bullet.x > 639 || abs(bullet.x - bullet.startX) >= 128) {
                if (tileCollision.type == Ground) {
                    // Explosion
                    smallExplosion(&expSmall, InFrontOfL1, bullet.x, bullet.y);
                }
                bullet.active = 0;
                bullet.zDepth = Disabled;
                x16SpriteIdxSetZDepth(bullet.index, bullet.zDepth);
            }
        }
    }
}

void main() {
    unsigned char nextSpriteIndex = 0;
    unsigned char i;
    Exit *exitCollision;
    Entrance *entrance;
    
    // Get the starting level and main entrance
    level = levelGet(0);
    entrance = findEntranceForExit(level->entranceList, 0);
    
    // Configure the joysticks
    joy_install(cx16_std_joy);
    
    showTitleScreen();
    
    tilesConfig();
    spriteDataLoad();
    spriteIRQConfig();

    // Wait to switch to game mode until everything is loaded
    // If you switch video modes first, you get crazy stuff on screen (kind cool?)
    videoConfig();

    // Create the sprites
    playerCreate(&player, entrance, nextSpriteIndex++);
    bulletCreate(&bullet, nextSpriteIndex++);
    explosionSmallCreate(&expSmall, nextSpriteIndex++);
    for (i=0; i<16; i++) {
        laserCreate(&enemyLasers[i], nextSpriteIndex++);
    }

    while(1) {
        layerMapsAddSomeStuff(level);
        exitCollision = runLevel(nextSpriteIndex);

        // Free the memory for the last level and load the next one
        freeLevel(level);
        level = levelGet(exitCollision->level);

        // Find the entrance the player is linking to and place them there
        entrance = findEntranceForExit(level->entranceList, exitCollision->entranceId);
        spriteMoveToTile(&player, entrance->x, entrance->y, TILE_PIXEL_WIDTH, TILE_PIXEL_HEIGHT);    
        x16SpriteIdxSetXY(player.index, player.x, player.y);
    }

    // Disable sprite collisions before quitting
    // or the UI hangs if sprites are still touching.
    x16SpriteCollisionsDisable();
}