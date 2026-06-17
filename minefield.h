//
// Created by blomq on 2026-06-16.
//

#ifndef MINESWEEPER_C_MINEFIELD_H
#define MINESWEEPER_C_MINEFIELD_H

#include "tile.h"

typedef struct MS_Point {
    int x;
    int y;
} MS_Point;

typedef enum MinesweeperState {
    MINESWEEPER_STATE_PLAYING,
    MINESWEEPER_STATE_LOST,
    MINESWEEPER_STATE_WON
} MinesweeperState;

typedef struct Minefield {
    int tileCount;
    int width;
    int height;
    int numMines;
    bool firstOpen;
    Tile *tiles;
    MS_Point explosionPos;
    MinesweeperState state;
} Minefield;

bool minefieldCreate(Minefield *minefield, int width, int height, int numMines);

bool minefieldReset(Minefield *minefield, int width, int height, int numMines);

void minefieldDestroy(const Minefield *minefield);

MinesweeperState minefieldOpenTile(Minefield *minefield, int xPos, int yPos);

void minefieldToggleFlag(const Minefield *minefield, int xPos, int yPos);

TileType minefieldGetTileType(const Minefield *minefield, int xPos, int yPos);

bool minefieldWithinField(const Minefield *minefield, int xPos, int yPos);

#endif //MINESWEEPER_C_MINEFIELD_H
