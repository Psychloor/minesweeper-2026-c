//
// Created by blomq on 2026-06-16.
//
#include "minefield.h"

#include <assert.h>

#include "set.h"
#include "vector.h"

#include <stdlib.h>
#include <time.h>

VECTOR_DEFINE(MS_Point, PointVector);

static uint32_t int_hash(const int value) {
    return (uint32_t) value * 2654435761u;
}

static bool int_equals(const int a, const int b) {
    return a == b;
}

SET_DEFINE(int, IntSet, int_hash, int_equals);

bool isInsideRange(const Minefield *minefield, const int xPos, const int yPos) {
    return xPos >= 0 && xPos < minefield->width && yPos >= 0 && yPos < minefield->height;
}

int mf_clamp(const int value, const int min, const int max) {
    return value < min ? min : value > max ? max : value;
}

void placeMines(const Minefield *minefield, const int firstX, const int firstY) {
    srand(time(NULL));

    size_t minesToPlace = minefield->numMines;
    const int firstIndex = firstY * minefield->width + firstX;
    while (minesToPlace > 0) {
        const size_t randomIndex = rand() % minefield->tileCount;

        if (randomIndex == firstIndex)
            continue;

        if (!minefield->tiles[randomIndex].isMine) {
            minefield->tiles[randomIndex].isMine = true;
            --minesToPlace;
        }
    }
}

uint8_t inline minesNearTile(const Minefield *minefield, const int xPos, const int yPos) {
    if (minefield->tiles[yPos * minefield->width + xPos].isMine)
        return 0;

    uint8_t mines = 0;
    for (int y = yPos - 1; y <= yPos + 1; ++y) {
        for (int x = xPos - 1; x <= xPos + 1; ++x) {
            if (x == xPos && y == yPos)
                continue;

            if (isInsideRange(minefield, x, y)) {
                if (minefield->tiles[y * minefield->width + x].isMine) {
                    ++mines;
                }
            }
        }
    }
    return mines;
}

void countAdjacentMines(const Minefield *minefield) {
    for (int i = 0; i < minefield->tileCount; ++i) {
        minefield->tiles[i].adjacentMines = minesNearTile(minefield, i % minefield->width, i / minefield->width);
    }
}

void openNearbyTiles(const Minefield *minefield, const int xPos, const int yPos) {
    PointVector tilesToOpen;
    if (!PointVector_create(&tilesToOpen, (minefield->tileCount) >> 2)) {
        return;
    }

    IntSet visitedTiles;
    if (!IntSet_create(&visitedTiles, (minefield->tileCount) >> 2)) {
        PointVector_destroy(&tilesToOpen);
        return;
    }

    PointVector_push(&tilesToOpen, (MS_Point){xPos, yPos});
    IntSet_insert(&visitedTiles, yPos * minefield->width + xPos);

    while (!PointVector_is_empty(&tilesToOpen)) {
        MS_Point tilePos;
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
                if (!isInsideRange(minefield, neighbourX, neighbourY))
                    continue;
                if (IntSet_contains(&visitedTiles, neighbourY * minefield->width + neighbourX)) continue;

                PointVector_push(&tilesToOpen, (MS_Point){neighbourX, neighbourY});
                IntSet_insert(&visitedTiles, neighbourY * minefield->width + neighbourX);
            }
        }
    }

    PointVector_destroy(&tilesToOpen);
    IntSet_destroy(&visitedTiles);
}

void openAllMines(const Minefield *minefield) {
    for (int i = 0; i < minefield->tileCount; ++i) {
        if (minefield->tiles[i].isMine && !minefield->tiles[i].isFlagged) {
            minefield->tiles[i].isOpen = true;
        }
    }
}

void checkWinCondition(Minefield *minefield) {
    for (int i = 0; i < minefield->tileCount; ++i) {
        const Tile *tile = &minefield->tiles[i];
        if (!tile->isMine && !tile->isOpen) {
            return;
        }
    }

    openAllMines(minefield);
    minefield->state = MINESWEEPER_STATE_WON;
}

bool minefieldCreate(Minefield *minefield, const int width, const int height, const int numMines) {
    assert(minefield != NULL && "Minefield cannot be null");
    if (!minefield) {
        return false;
    }

    minefield->tileCount = 0;
    minefield->tiles = NULL;
    return minefieldReset(minefield, width, height, numMines);
}

bool minefieldReset(Minefield *minefield, const int width, const int height, const int numMines) {
    assert(minefield != NULL && "Minefield cannot be null");
    assert(width > 1 && "Width must be greater than 1");
    assert(height > 1 && "Height must be greater than 1");
    assert(numMines > 0 && "Number of mines must be greater than 0");

    if (!minefield) {
        return false;
    }

    if (width < 2 || height < 2 || numMines < 1) {
        return false;
    }

    const int tileCount = width * height;

    if (tileCount != minefield->tileCount) {
        Tile *newTiles = realloc(minefield->tiles, tileCount * sizeof(Tile));
        if (!newTiles) {
            return false;
        }
        minefield->tiles = newTiles;
    }

    Tile *tiles = minefield->tiles;
    memset(minefield, 0, sizeof(Minefield));

    minefield->tiles = tiles;
    minefield->tileCount = tileCount;
    minefield->width = width;
    minefield->height = height;
    minefield->numMines = mf_clamp(numMines, 1, tileCount >> 1);
    minefield->firstOpen = true;
    minefield->explosionPos = (MS_Point){-1, -1};
    minefield->state = MINESWEEPER_STATE_PLAYING;

    memset(minefield->tiles, 0, tileCount * sizeof(Tile));

    return true;
}

void minefieldDestroy(const Minefield *minefield) {
    if (minefield != NULL && minefield->tiles != NULL) {
        free(minefield->tiles);
    }
}

MinesweeperState minefieldOpenTile(Minefield *minefield, const int xPos, const int yPos) {
    assert(minefield != NULL && "Minefield cannot be null");
    assert(xPos >= 0 && xPos < minefield->width && "X position is out of range");
    assert(yPos >= 0 && yPos < minefield->height && "Y position is out of range");
    if (minefield->state != MINESWEEPER_STATE_PLAYING) return minefield->state;

    if (minefield->firstOpen) {
        placeMines(minefield, xPos, yPos);
        countAdjacentMines(minefield);
        minefield->firstOpen = false;
    }

    Tile *tile = &minefield->tiles[yPos * minefield->width + xPos];
    if (tile->isFlagged || tile->isOpen) return minefield->state;

    if (tile->isMine) {
        tile->isOpen = true;
        tile->isQuestionMarked = false;
        minefield->explosionPos.x = xPos;
        minefield->explosionPos.y = yPos;
        openAllMines(minefield);
        minefield->state = MINESWEEPER_STATE_LOST;
    } else {
        openNearbyTiles(minefield, xPos, yPos);
        checkWinCondition(minefield);
    }

    return minefield->state;
}

void minefieldToggleFlag(const Minefield *minefield, const int xPos, const int yPos) {
    assert(minefield != NULL && "Minefield cannot be null");
    assert(xPos >= 0 && xPos < minefield->width && "X position is out of range");
    assert(yPos >= 0 && yPos < minefield->height && "Y position is out of range");

    if (minefield->state != MINESWEEPER_STATE_PLAYING)
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

TileType minefieldGetTileType(const Minefield *minefield, const int xPos, const int yPos) {
    assert(minefield != NULL && "Minefield cannot be null");
    assert(xPos >= 0 && xPos < minefield->width && "X position is out of range");
    assert(yPos >= 0 && yPos < minefield->height && "Y position is out of range");

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
        if (minefield->state == MINESWEEPER_STATE_LOST && !tile->isMine) {
            return TILE_TYPE_FALSE_MINE;
        }
        return TILE_TYPE_FLAGGED;
    }
    if (tile->isQuestionMarked) {
        return TILE_TYPE_CLOSED_QUESTION_MARK;
    }

    return TILE_TYPE_CLOSED;
}

bool minefieldWithinField(const Minefield *minefield, const int xPos, const int yPos) {
    assert(minefield != NULL && "Minefield cannot be null");

    return xPos >= 0 && xPos < minefield->width && yPos >= 0 && yPos < minefield->height;
}

const Tile *minefieldGetTile(const Minefield *minefield, const int xPos, const int yPos) {
    assert(minefield != NULL && "Minefield cannot be null");
    assert(xPos >= 0 && xPos < minefield->width && "X position is out of range");
    assert(yPos >= 0 && yPos < minefield->height && "Y position is out of range");

    return &minefield->tiles[yPos * minefield->width + xPos];
}

const Tile *minefieldGetTileFromIndex(const Minefield *minefield, const int index) {
    assert(minefield != NULL && "Minefield cannot be null");
    assert(index >= 0 && index < minefield->tileCount);
    return &minefield->tiles[index];
}

bool minefieldIsGameOver(const Minefield *minefield) {
    assert(minefield != NULL && "Minefield cannot be null");
    return minefield->state == MINESWEEPER_STATE_LOST;
}

bool minefieldIsWin(const Minefield *minefield) {
    assert(minefield != NULL && "Minefield cannot be null");
    return minefield->state == MINESWEEPER_STATE_WON;
}

MS_Point minefieldGetExplosionPoint(const Minefield *minefield) {
    assert(minefield != NULL && "Minefield cannot be null");
    return minefield->explosionPos;
}
