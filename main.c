#ifdef WIN32
#define LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <SDL3/SDL.h>

#include "minefield.h"
#include "settings.h"

#define SDL_MAIN_USE_CALLBACKS
#include <stdio.h>
#include <SDL3/SDL_main.h>

const int TILE_SIZE = 24;
const float TILE_TEX_SIZE = 16.f;

const char *kTitle = "Minesweeper";
const char *kTitleGameOver = "Minesweeper - Game Over";
const char *kTitleGameWon = "Minesweeper - You Won!";

int startingWidth = 20;
int startingHeight = 20;
int startingMines = 30;

typedef struct AppContext {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *tileTexture;
    SDL_Surface *iconSurface;

    Minefield minefield;
    SDL_Point mousePos;
    SDL_FRect *tileSrcRects;
    SDL_FRect *tileDstRects;
} AppContext;

SDL_FRect tileTexRect(const Tile *tile, const bool explosionPoint, const bool isGameOver) {
    SDL_FRect tileRect = {
        .x = 1,
        .y = 2,
        .w = TILE_TEX_SIZE,
        .h = TILE_TEX_SIZE
    };

    if (tile->isOpen) {
        if (tile->isMine) {
            tileRect.x = explosionPoint ? 3 : 2;
            tileRect.y = 3;
        } else if (tile->adjacentMines > 0) {
            const int numberIndex = tile->adjacentMines - 1;
            tileRect.x = SDL_floorf((float) (numberIndex % 4));
            tileRect.y = SDL_floorf((float) (numberIndex) / 4);
        } else if (tile->isQuestionMarked) {
            tileRect.x = 0;
            tileRect.y = 3;
        } else {
            tileRect.x = 0;
            tileRect.y = 2;
        }
    } else {
        if (tile->isFlagged) {
            tileRect.x = 2;
            tileRect.y = 2;

            if (isGameOver && !tile->isMine) {
                tileRect.x = 3;
            }
        } else if (tile->isQuestionMarked) {
            tileRect.x = 1;
            tileRect.y = 3;
        }
    }

    tileRect.x *= TILE_TEX_SIZE;
    tileRect.y *= TILE_TEX_SIZE;

    return tileRect;
}

void updateRenderRects(AppContext *context) {
    const SDL_Point explosionRect = context->minefield.explosionPos;
    const bool isGameOver = context->minefield.isGameOver;
    for (int i = 0; i < startingWidth * startingHeight; ++i) {
        const int x = i % startingWidth;
        const int y = i / startingWidth;

        const SDL_FRect src = tileTexRect(&context->minefield.tiles[i], explosionRect.x == x && explosionRect.y == y,
                                          isGameOver);
        context->tileSrcRects[i] = src;

        context->tileDstRects[i].x = (float) (x * TILE_SIZE);
        context->tileDstRects[i].y = (float) (y * TILE_SIZE);
        context->tileDstRects[i].w = (float) TILE_SIZE;
        context->tileDstRects[i].h = (float) TILE_SIZE;
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    const Settings settings = settingsLoad("settings.ini", (Settings){
                                               .width = 20,
                                               .height = 20,
                                               .mines = 30
                                           });

    startingWidth = settings.width;
    startingHeight = settings.height;
    startingMines = settings.mines;

    AppContext *context = (AppContext *) malloc(sizeof(AppContext));
    if (!context) {
        SDL_Log("malloc failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    context->tileSrcRects = (SDL_FRect *) calloc(startingWidth * startingHeight, sizeof(SDL_FRect));
    if (!context->tileSrcRects) {
        SDL_Log("malloc failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    context->tileDstRects = (SDL_FRect *) calloc(startingWidth * startingHeight, sizeof(SDL_FRect));
    if (!context->tileDstRects) {
        SDL_Log("malloc failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    (*appstate) = context;

    if (!SDL_CreateWindowAndRenderer(kTitle, startingWidth * TILE_SIZE, startingHeight * TILE_SIZE, 0, &context->window,
                                     &context->renderer)) {
        SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderVSync(context->renderer, 1);

    context->iconSurface = SDL_LoadPNG("assets/icon.png");
    if (!context->iconSurface) {
        SDL_Log("SDL_LoadPNG failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetWindowIcon(context->window, context->iconSurface);

    SDL_Surface *surface = SDL_LoadPNG("assets/tiles.png");
    if (!surface) {
        SDL_Log("SDL_LoadPNG failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    context->tileTexture = SDL_CreateTextureFromSurface(context->renderer, surface);
    if (!context->tileTexture) {
        SDL_Log("SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_DestroySurface(surface);

    if (!minefieldCreate(&context->minefield, startingWidth, startingHeight, startingMines)) {
        SDL_Log("minefield_create failed");
        return SDL_APP_FAILURE;
    }
    updateRenderRects(context);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    const AppContext *context = (AppContext *) appstate;

    SDL_SetRenderDrawColor(context->renderer, 0, 0, 0, 255);
    SDL_RenderClear(context->renderer);

    SDL_SetRenderDrawColor(context->renderer, 255, 255, 255, 255);
    for (int i = 0; i < startingWidth * startingHeight; ++i) {
        SDL_RenderTexture(context->renderer, context->tileTexture, &context->tileSrcRects[i],
                          &context->tileDstRects[i]);
    }

    SDL_RenderPresent(context->renderer);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    AppContext *context = appstate;

    switch (event->type) {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            return SDL_APP_SUCCESS;

        case SDL_EVENT_MOUSE_MOTION:
            context->mousePos.x = (int) SDL_floorf(event->motion.x / (float) TILE_SIZE);
            context->mousePos.y = (int) SDL_floorf(event->motion.y / (float) TILE_SIZE);
            break;

        case SDL_EVENT_KEY_DOWN:
            if (event->key.key == SDLK_SPACE) {
                if (!context->minefield.isGameOver && !context->minefield.isGameWon) {
                    return SDL_APP_CONTINUE;
                }

                minefieldDestroy(&context->minefield);
                minefieldCreate(&context->minefield, startingWidth, startingHeight, startingMines);
                SDL_SetWindowTitle(context->window, kTitle);
                updateRenderRects(context);
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event->button.button == SDL_BUTTON_LEFT) {
                minefieldOpenTile(&context->minefield, context->mousePos.x, context->mousePos.y);
                if (context->minefield.isGameOver) {
                    SDL_SetWindowTitle(context->window, kTitleGameOver);
                } else if (context->minefield.isGameWon) {
                    SDL_SetWindowTitle(context->window, kTitleGameWon);
                }
                updateRenderRects(context);
            } else if (event->button.button == SDL_BUTTON_RIGHT) {
                minefieldToggleFlag(&context->minefield, context->mousePos.x, context->mousePos.y);
                updateRenderRects(context);
            }
            break;

        default:
            break;
    }


    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    AppContext *context = (AppContext *) appstate;

    free(context->tileSrcRects);
    free(context->tileDstRects);
    minefieldDestroy(&context->minefield);
    SDL_DestroySurface(context->iconSurface);
    SDL_DestroyTexture(context->tileTexture);
    SDL_DestroyRenderer(context->renderer);
    SDL_DestroyWindow(context->window);

    free(context);

    SDL_Quit();
}
