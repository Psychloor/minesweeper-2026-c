//
// Created by blomq on 2026-06-16.
//

#ifndef MINESWEEPER_C_MINEFIELD_H
#define MINESWEEPER_C_MINEFIELD_H

#include "tile.h"
#include "SDL3/SDL_rect.h"

typedef struct Minefield {
    int tileCount;
    int width;
    int height;
    int numMines;
    bool firstOpen;
    bool isGameOver;
    bool isGameWon;
    Tile *tiles;
    SDL_Point explosionPos;
} Minefield;

bool minefieldCreate(Minefield *minefield, int width, int height, int numMines);

bool minefieldReset(Minefield *minefield, int width, int height, int numMines);

void minefieldDestroy(const Minefield *minefield);

void minefieldOpenTile(Minefield *minefield, int xPos, int yPos);

void minefieldToggleFlag(const Minefield *minefield, int xPos, int yPos);

TileType minefieldGetTileType(const Minefield *minefield, int xPos, int yPos);

bool minefieldWithinField(const Minefield *minefield, int xPos, int yPos);

#endif //MINESWEEPER_C_MINEFIELD_H
