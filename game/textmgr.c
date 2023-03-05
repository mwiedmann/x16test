#include <stdio.h>
#include <cbm.h>
#include <cx16.h>

#include <peekpoke.h>
#include <joystick.h>
#include "waitforjiffy.h"
#include "loopmgr.h"
#include "x16graphics.h"
#include "memmap.h"
#include "imageload.h"
#include "fontmgr.h"
#include "sprites.h"
#include "levelutils.h"
#include "gamesprites.h"
#include "levelmgr.h"
#include "soundmgr.h"

#define SELECTION_START_GAME 1
#define SELECTION_JUKEBOX 2
#define SELECTION_TEST_MODE 3

void showTitleScreen() {
    unsigned char joy = 0;
    unsigned char wait = 60;

    // Clear the VRAM we will use for the bitmap mode title
    clearFullVRAM(0);
    clearVRAM(1, 0, 11264);

    // Set to 320x240 size
    POKE(0x9F2A, 64);
    POKE(0x9F2B, 64);

    // Enable Layer 1 only (probably already set this way but just in case) (leave other settings the same)
    POKE(VMEM_VIDEO, PEEK(VMEM_VIDEO) | 0b00100000);
    POKE(VMEM_VIDEO, PEEK(VMEM_VIDEO) & 0b11101111); // Disable layer 0 if its on

    // Config Layer 1 - H/W, 256clr, Bitmap Mode, 8bpp color depth
    POKE(LAYER_1_CONFIG, 0b00001111);

    // Bitmap mode loading a 320x240 image takes most of VRAM
    // Start in VBank 0, Addr 0
    POKE(LAYER_1_TILEBASE, 0);

    imageFileLoad(IMAGE_LOAD_BANK, 0, 0, "images/title.bin");
    preloadTextFiles();

    // Short delay before playing the music and button press skips title screen
    while (wait > 0) {
        waitforjiffy();
        wait--;
    }

    loadTitleMusic();

    while(1) {
        loopUpdates();

        joy = joy_read(0);

        // Exit when either button is pressed
        // Exit with testMode ON if UP is pressed
        if (JOY_BTN_1(joy)) {
            while(JOY_BTN_1(joy)) {
                loopUpdates();
        
                joy = joy_read(0);
            }
            break;
        } else if (JOY_BTN_2(joy)) {
            while(JOY_BTN_2(joy)) {
                loopUpdates();
        
                joy = joy_read(0);
            }
            break;
        }
    }

    // Turn off Layer 1 (both layers will be off while things are loading)
    POKE(VMEM_VIDEO, PEEK(VMEM_VIDEO) ^ 0b00100000);
}

void waitWithShipAnimation(Sprite *ship) {
    while(1) {
        // Wait for screen to finish drawing since we are animating the ship
        loopUpdates();
        spriteAnimationAdvance(ship);

        // Exit when either button is pressed
        if (readButtonPress()) {
            break;
        }
    }
}

unsigned char showSelectionScreen() {
    unsigned char selection = 1;
    unsigned char joy = 0;

    layerMapsClear();

    while (1) {
        drawCenteredTextRow("PIXEUS", 0, 6);
        drawCenteredTextRow("-GAME DESIGN AND CODING-", 0, 8);
        drawCenteredTextRow("MARK WIEDMANN", 0, 9);

        drawCenteredTextRow("-MUSIC-", 0, 11);
        drawCenteredTextRow("CRISPS", 0, 12);
        
        drawCenteredTextRow("USE THE JOYSTICK TO SELECT", 0, 15);

        drawCenteredTextRow(selection == 1 ? "::START GAME::" : "  START GAME  ", 0, 17);
        drawCenteredTextRow(selection == 2 ? "::JUKEBOX::" : "  JUKEBOX  ", 0, 18);
        drawCenteredTextRow(selection == 3 ? "::TEST MODE::" : "  TEST MODE  ", 0, 19);

        while(1) {
            loopUpdates();
            
            joy = joy_read(0);

            if (JOY_UP(joy)) {
                selection--;
                if (selection==0) {
                    selection = 3;
                }
                while(JOY_UP(joy)) {
                    loopUpdates();
            
                    joy = joy_read(0);
                }
                break;
            } else if (JOY_DOWN(joy)) {
                selection++;
                if (selection>3) {
                    selection = 1;
                }
                while(JOY_DOWN(joy)) {
                    loopUpdates();
            
                    joy = joy_read(0);
                }
                break;
            }

            if (readButtonPress()) {
                return selection;
            }
        }
    }
}

void showJukebox() {
    unsigned char selection = 1;
    unsigned char playMusic;
    unsigned char joy = 0;

    layerMapsClear();

    while (1) {
        drawCenteredTextRow("JUKEBOX", 0, 6);
        drawCenteredTextRow("ENJOY THESE AMAZING TUNES", 0, 8);
        drawCenteredTextRow("BY CRISPS", 0, 9);

        drawCenteredTextRow("USE THE JOYSTICK TO SELECT", 0, 14);

        drawCenteredTextRow(selection == 1 ? "::TITLE::" : "  TITLE  ", 0, 16);
        drawCenteredTextRow(selection == 2 ? "::FOREST::" : "  FOREST  ", 0, 17);
        drawCenteredTextRow(selection == 3 ? "::DESERT::" : "  DESERT  ", 0, 18);

        drawCenteredTextRow(selection == 4 ? "::STOP MUSIC::" : "  STOP MUSIC  ", 0, 20);
        drawCenteredTextRow(selection == 5 ? "::EXIT::" : "  EXIT  ", 0, 21);

        playMusic = 0;

        while(1) {
            loopUpdates();
            
            joy = joy_read(0);

            if (JOY_UP(joy)) {
                selection--;
                if (selection==0) {
                    selection = 5;
                }
                while(JOY_UP(joy)) {
                    loopUpdates();
            
                    joy = joy_read(0);
                }
                break;
            } else if (JOY_DOWN(joy)) {
                selection++;
                if (selection>5) {
                    selection = 1;
                }
                while(JOY_DOWN(joy)) {
                    loopUpdates();
            
                    joy = joy_read(0);
                }
                break;
            }

            if (readButtonPress()) {
                playMusic = 1;
                break;
            }
        }

        if (playMusic) {
            switch(selection) {
                case 1: loadTitleMusic(); break;
                case 2: loadForestMusic(); break;
                case 3: loadDesertMusic(); break;
                case 4: /*pauseSounds();*/ break; // This breaks everything right now...WTF!
                case 5: return;
            }

            // zsound bug throws junk into the layer map when you stop/switch music
            // Just clear it as a workaround.
            layerMapsClear();
            loopUpdates();
        }
    }
}

unsigned char showIntroScene(Sprite *ship) {
    unsigned char selection;

    while (1) {
        selection = showSelectionScreen();

        // Test Mode - Jump right to the game
        if (selection == SELECTION_TEST_MODE) {
            return 1;
        } else if (selection == SELECTION_JUKEBOX) {
            showJukebox();
            continue;
        }

        layerMapsClear();
        drawTextFileFromBank(WELCOME_BANK, 0);

        spriteMove(ship, 288, 350);
        x16SpriteIdxSetXY(ship->index, ship->x, ship->y);
        ship->zDepth = BetweenL0L1;
        x16SpriteIdxSetZDepth(ship->index, ship->zDepth);

        waitWithShipAnimation(ship);

        layerMapsClear();
        drawTextFileFromBank(INSTRUCTIONS_BANK, 0);

        waitWithShipAnimation(ship);

        spriteMoveBack(ship);
        x16SpriteIdxSetXY(ship->index, ship->x, ship->y);

        if (selection == SELECTION_START_GAME) {
            return 0;
        }
    }
}

void gameOverScreen(unsigned short gold, unsigned char energy) {
    unsigned char scoreRow[41];

    layerMapsClear();

    sprintf(scoreRow, "GOLD:%04u ENERGY:%03u", gold, energy);
    drawCenteredTextRow(scoreRow, 0, 4);

    drawTextFile("text/gameover.bin", 0);

    while(1) {
        loopUpdates();
        
        // Exit when either button is pressed
        if (readButtonPress()) {
            break;
        }
    }
}

void victoryScreen(Sprite *ship, unsigned short gold) {
    unsigned char text[41];

    layerMapsClear();

    if (gold == 0) {
        drawTextFile("text/nogold.bin", 0);
    } else {
        sprintf(text, "PIXEUS SPENDS %u GOLD ON:", gold);
        drawCenteredTextRow(text, 0, 6);

        drawTextFile("text/victory.bin", gold);
    }
   
    spriteMove(ship, 288, 350);
    x16SpriteIdxSetXY(ship->index, ship->x, ship->y);
    ship->zDepth = BetweenL0L1;
    x16SpriteIdxSetZDepth(ship->index, ship->zDepth);

    while(1) {
        // Wait for screen to finish drawing since we are animating the ship
        loopUpdates();
        
        spriteAnimationAdvance(ship);

        // Exit when either button is pressed
        if (readButtonPress()) {
            break;
        }
    }
}