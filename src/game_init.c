#include "./game_init.h"
#include "./draw.h"

Uint32 startingTick;

void fadeInFromBlack(SDL_Renderer *renderer) {
  Uint32 elapsedTime = SDL_GetTicks() - startingTick;
  if (elapsedTime > INIT_FADE_LENGTH) {
    return;
  }
  Uint8 alpha = 255 - (255 * elapsedTime / INIT_FADE_LENGTH);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
  SDL_Rect rect = { 0, 0, GAME_WIDTH, GAME_HEIGHT };
  SDL_RenderFillRect(renderer, &rect);
}