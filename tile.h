//
// Created by blomq on 2026-06-16.
//

#ifndef MINESWEEPER_C_CELL_H
#define MINESWEEPER_C_CELL_H
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct Tile {
    bool isOpen: 1;
    bool isMine: 1;
    bool isFlagged: 1;
    bool isQuestionMarked: 1;
    uint8_t adjacentMines: 4;
} Tile;

typedef enum TileType {
    TILE_TYPE_NUMBER_ONE,
    TILE_TYPE_NUMBER_TWO,
    TILE_TYPE_NUMBER_THREE,
    TILE_TYPE_NUMBER_FOUR,
    TILE_TYPE_NUMBER_FIVE,
    TILE_TYPE_NUMBER_SIX,
    TILE_TYPE_NUMBER_SEVEN,
    TILE_TYPE_NUMBER_EIGHT,
    TILE_TYPE_OPEN,
    TILE_TYPE_CLOSED,
    TILE_TYPE_FLAGGED,
    TILE_TYPE_FALSE_MINE,
    TILE_TYPE_OPEN_QUESTION_MARK,
    TILE_TYPE_CLOSED_QUESTION_MARK,
    TILE_TYPE_MINE,
    TILE_TYPE_EXPLODED_MINE,
    TILE_TYPE_COUNT_SIZE
} TileType;

#endif //MINESWEEPER_C_CELL_H
