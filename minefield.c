//
// Created by blomq on 2026-06-16.
//
#include "minefield.h"

#include <assert.h>

#include "set.h"
#include "vector.h"

#include <stdlib.h>
#include <time.h>

#include "SDL3/SDL_log.h"
#include "SDL3/SDL_rect.h"

// struct Minefield {
//     int width_;
//     int height_;
//     int numMines_;
//     bool firstOpen_;
//     bool isGameOver_;
//     bool isGameWon_;
//     Tile *tiles_;
//     SDL_Point explosionPos_;
// };

VECTOR_DEFINE(SDL_Point, PointVector);

static uint32_t int_hash(const int value) {
    return (uint32_t) value * 2654435761u;
}

static bool int_equals(const int a, const int b) {
    return a == b;
}

SET_DEFINE(int, IntSet, int_hash, int_equals);

void placeMines(const Minefield *minefield, const int firstX, const int firstY) {
    srand(time(NULL));

    size_t minesToPlace = minefield->numMines;
    while (minesToPlace > 0) {
        const size_t randomIndex = rand() % (minefield->width * minefield->height);
        const int x = (int) randomIndex % minefield->width;
        const int y = (int) randomIndex / minefield->width;

        if (x == firstX && y == firstY)
            continue;

        if (!minefield->tiles[randomIndex].isMine) {
            minefield->tiles[randomIndex].isMine = true;
            --minesToPlace;
        }
    }
}

uint8_t minesNearTile(const Minefield *minefield, const int xPos, const int yPos) {
    uint8_t mines = 0;
    for (int y = yPos - 1; y <= yPos + 1; ++y) {
        for (int x = xPos - 1; x <= xPos + 1; ++x) {
            if (x == xPos && y == yPos)
                continue;

            if (x >= 0 && x < minefield->width && y >= 0 && y < minefield->height) {
                if (minefield->tiles[y * minefield->width + x].isMine) {
                    ++mines;
                }
            }
        }
    }
    return mines;
}

void countAdjacentMines(const Minefield *minefield) {
    for (int i = 0; i < minefield->width * minefield->height; ++i) {
        minefield->tiles[i].adjacentMines = minesNearTile(minefield, i % minefield->width, i / minefield->width);
    }
}

void openNearbyTiles(const Minefield *minefield, const int xPos, const int yPos) {
    PointVector tilesToOpen;
    if (!PointVector_create(&tilesToOpen, (minefield->width * minefield->height) >> 2)) {
        SDL_Log("Failed to allocate memory for tilesToOpen");
        return;
    }

    IntSet visitedTiles;
    if (!IntSet_create(&visitedTiles, (minefield->width * minefield->height) >> 2)) {
        SDL_Log("Failed to allocate memory for visitedTiles");
        PointVector_destroy(&tilesToOpen);
        return;
    }

    PointVector_push(&tilesToOpen, (SDL_Point){xPos, yPos});
    IntSet_insert(&visitedTiles, yPos * minefield->width + xPos);

    while (!PointVector_is_empty(&tilesToOpen)) {
        SDL_Point tilePos;
        PointVector_pop(&tilesToOpen, &tilePos);

        Tile *currentTile = &minefield->tiles[tilePos.y * minefield->width + tilePos.x];
        if (currentTile->isOpen || currentTile->isFlagged || currentTile->isMine) continue;

        currentTile->isOpen = true;
        if (currentTile->adjacentMines > 0) continue;

        for (int y = -1; y <= 1; ++y) {
            for (int x = -1; x <= 1; ++x) {
                if (x == 0 && y == 0) continue;
                const int neighbourX = tilePos.x + x;
                const int neighbourY = tilePos.y + y;
                if (neighbourX < 0 || neighbourX >= minefield->width || neighbourY < 0 || neighbourY >= minefield->
                    height)
                    continue;
                if (IntSet_contains(&visitedTiles, neighbourY * minefield->width + neighbourX)) continue;

                PointVector_push(&tilesToOpen, (SDL_Point){neighbourX, neighbourY});
                IntSet_insert(&visitedTiles, neighbourY * minefield->width + neighbourX);
            }
        }
    }

    PointVector_destroy(&tilesToOpen);
    IntSet_destroy(&visitedTiles);
}

void openAllMines(const Minefield *minefield) {
    for (int i = 0; i < minefield->width * minefield->height; ++i) {
        if (minefield->tiles[i].isMine && !minefield->tiles[i].isFlagged) {
            minefield->tiles[i].isOpen = true;
        }
    }
}

void checkWinCondition(Minefield *minefield) {
    for (int i = 0; i < minefield->width * minefield->height; ++i) {
        const Tile *tile = &minefield->tiles[i];
        if (!tile->isMine && !tile->isOpen) {
            return;
        }
    }

    openAllMines(minefield);
    minefield->isGameWon = true;
}

bool minefieldCreate(Minefield *minefield, const int width, const int height, const int numMines) {
    memset(minefield, 0, sizeof(Minefield));
    minefield->height = height;
    minefield->width = width;
    minefield->numMines = SDL_clamp(numMines, 1, (width * height) >> 1);
    minefield->firstOpen = true;
    minefield->isGameOver = false;
    minefield->isGameWon = false;
    minefield->explosionPos = (SDL_Point){-1, -1};

    minefield->tiles = (Tile *) calloc(width * height, sizeof(Tile));
    if (!minefield->tiles) {
        free(minefield->tiles);
        return false;
    }

    memset(minefield->tiles, 0, width * height * sizeof(Tile));
    return true;
}

void minefieldDestroy(const Minefield *minefield) {
    if (minefield && minefield->tiles) {
        free(minefield->tiles);
    }
}

void minefieldOpenTile(Minefield *minefield, const int xPos, const int yPos) {
    if (minefield->isGameOver) return;
    if (minefield->isGameWon) return;

    if (minefield->firstOpen) {
        placeMines(minefield, xPos, yPos);
        countAdjacentMines(minefield);
        minefield->firstOpen = false;
    }

    Tile *tile = &minefield->tiles[yPos * minefield->width + xPos];
    if (tile->isFlagged || tile->isOpen) return;

    if (tile->isMine) {
        minefield->isGameOver = true;
        tile->isOpen = true;
        tile->isQuestionMarked = false;
        minefield->explosionPos.x = xPos;
        minefield->explosionPos.y = yPos;
        openAllMines(minefield);
    } else {
        openNearbyTiles(minefield, xPos, yPos);
        checkWinCondition(minefield);
    }
}

void minefieldToggleFlag(const Minefield *minefield, const int xPos, const int yPos) {
    if (minefield->isGameOver || minefield->isGameWon)
        return;

    Tile *tile = &minefield->tiles[yPos * minefield->width + xPos];
    if (tile->isOpen)
        return;

    if (!tile->isFlagged && !tile->isQuestionMarked) {
        tile->isFlagged = true;
    } else if (tile->isFlagged) {
        tile->isFlagged = false;
        tile->isQuestionMarked = true;
    } else {
        tile->isQuestionMarked = false;
    }
}

TileType minefieldGetTileType(const Minefield *minefield, int xPos, int yPos) {
    const Tile *tile = &minefield->tiles[yPos * minefield->width + xPos];
    if (tile->isOpen) {
        if (tile->isMine) {
            if (minefield->explosionPos.x == xPos && minefield->explosionPos.y == yPos) {
                return TILE_TYPE_EXPLODED_MINE;
            }
            return TILE_TYPE_MINE;
        }
        if (tile->adjacentMines > 0) {
            switch (tile->adjacentMines) {
                case 1: return TILE_TYPE_NUMBER_ONE;
                case 2: return TILE_TYPE_NUMBER_TWO;
                case 3: return TILE_TYPE_NUMBER_THREE;
                case 4: return TILE_TYPE_NUMBER_FOUR;
                case 5: return TILE_TYPE_NUMBER_FIVE;
                case 6: return TILE_TYPE_NUMBER_SIX;
                case 7: return TILE_TYPE_NUMBER_SEVEN;
                case 8: return TILE_TYPE_NUMBER_EIGHT;
                default: return TILE_TYPE_OPEN;
            }
        }
        if (tile->isQuestionMarked) {
            return TILE_TYPE_OPEN_QUESTION_MARK;
        }
        return TILE_TYPE_OPEN;
    }
    if (tile->isFlagged) {
        if (minefield->isGameOver && !tile->isMine) {
            return TILE_TYPE_FALSE_MINE;
        }
        return TILE_TYPE_FLAGGED;
    }
    if (tile->isQuestionMarked) {
        return TILE_TYPE_CLOSED_QUESTION_MARK;
    }

    return TILE_TYPE_CLOSED;
}

const Tile *minefieldGetTile(const Minefield *minefield, const int xPos, const int yPos) {
    return &minefield->tiles[yPos * minefield->width + xPos];
}

const Tile *minefieldGetTileFromIndex(const Minefield *minefield, const int index) {
    assert(index >= 0 && index < minefield->width * minefield->height);
    return &minefield->tiles[index];
}

bool minefieldIsGameOver(const Minefield *minefield) {
    return minefield->isGameOver;
}

bool minefieldIsWin(const Minefield *minefield) {
    return minefield->isGameWon;
}

SDL_Point minefieldGetExplosionPoint(const Minefield *minefield) {
    return minefield->explosionPos;
}
