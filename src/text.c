#include "./text.h"
#include "./draw.h"
#include "./game.h"

TTF_Font *Sans = NULL;
SDL_Color TEXT_COLOR = {0, 0, 0};
SDL_Surface *message_titlescreen_surface;
SDL_Texture *message_titlescreen_texture;
SDL_Surface *message_gameover_surface;
SDL_Texture *message_gameover_texture;
SDL_Surface *message_paused_surface;
SDL_Texture *message_paused_texture;

SDL_Rect scoreRect;
SDL_Rect gameOverRect;
SDL_Rect titleScreenRect;
SDL_Rect pausedRect;

char score[10];

const float textHeightMult = HEIGHT / 1000.0f;

void initStaticMessages(SDL_Renderer *renderer) {
  Sans = TTF_OpenFont("Mono.ttf", 42 * HEIGHT / 1000);

  message_titlescreen_surface = TTF_RenderText_Solid(Sans, "Blockamok", TEXT_COLOR);
  titleScreenRect.w = message_titlescreen_surface->w * 3;
  titleScreenRect.h = message_titlescreen_surface->h * 3;
  message_titlescreen_texture = SDL_CreateTextureFromSurface(renderer, message_titlescreen_surface);
  titleScreenRect.x = WIDTH / 2 - titleScreenRect.w / 2;
  titleScreenRect.y = HEIGHT / 2 - titleScreenRect.h / 2;
  SDL_FreeSurface(message_titlescreen_surface);

  scoreRect.w = 72 * textHeightMult;
  scoreRect.h = 50 * textHeightMult;
  scoreRect.x = 0;
  scoreRect.y = -textHeightMult * 10;

  message_gameover_surface = TTF_RenderText_Solid(Sans, "GAME OVER", TEXT_COLOR);
  gameOverRect.w = message_gameover_surface->w * 3;
  gameOverRect.h = message_gameover_surface->h * 3;
  message_gameover_texture = SDL_CreateTextureFromSurface(renderer, message_gameover_surface);
  gameOverRect.x = WIDTH / 2 - gameOverRect.w / 2;
  gameOverRect.y = HEIGHT / 2 - gameOverRect.h / 2;
  SDL_FreeSurface(message_gameover_surface);

  message_paused_surface = TTF_RenderText_Solid(Sans, "PAUSED", TEXT_COLOR);
  pausedRect.w = message_paused_surface->w * 3;
  pausedRect.h = message_paused_surface->h * 3;
  message_paused_texture = SDL_CreateTextureFromSurface(renderer, message_paused_surface);
  pausedRect.x = WIDTH / 2 - pausedRect.w / 2;
  pausedRect.y = HEIGHT / 2 - pausedRect.h / 2;
  SDL_FreeSurface(message_paused_surface);
}

inline void drawTitleScreenText(SDL_Renderer *renderer) {
  SDL_RenderCopy(renderer, message_titlescreen_texture, NULL, &titleScreenRect);
}

inline void drawScoreText(SDL_Renderer *renderer) {
  sprintf(score, "%d", (int)scoreVal);
  SDL_Surface *message_score_surface = TTF_RenderText_Solid(Sans, score, TEXT_COLOR);

  // Adjust the position of the scoreRect to center the text
  scoreRect.x = (WIDTH - message_score_surface->w) / 2;
  scoreRect.w = message_score_surface->w;
  scoreRect.h = message_score_surface->h;

  SDL_Texture *message_score_texture = SDL_CreateTextureFromSurface(renderer, message_score_surface);
  SDL_RenderCopy(renderer, message_score_texture, NULL, &scoreRect);
  SDL_FreeSurface(message_score_surface);
  SDL_DestroyTexture(message_score_texture);
}

inline void drawGameOverText(SDL_Renderer *renderer) {
  SDL_RenderCopy(renderer, message_gameover_texture, NULL, &gameOverRect);
}

inline void drawPausedText(SDL_Renderer *renderer) {
  SDL_RenderCopy(renderer, message_paused_texture, NULL, &pausedRect);
}