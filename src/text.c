#include "./text.h"
#include "./draw.h"
#include "./game.h"
#include "./input.h"

TTF_Font *Sans = NULL;
SDL_Color color_black = {0, 0, 0};

Message message_titlescreen;
Message message_titlescreen_pressstart;
Message message_titlescreen_pressselect;
Message message_score;
Message message_gameover;
Message message_paused;

SDL_Rect score_rect;

const float textWidthMult = WIDTH / 1000.0f;
const float textHeightMult = HEIGHT / 1000.0f;

static inline void prepareMessage(SDL_Renderer *renderer, TTF_Font *font, Message *message, float sizeMult, SDL_Color color) {
	message->surface = TTF_RenderText_Solid(font, message->text, color);
	message->rect.w = message->surface->w * sizeMult;
	message->rect.h = message->surface->h * sizeMult;
	if (message->texture != NULL) SDL_DestroyTexture(message->texture);
	message->texture = SDL_CreateTextureFromSurface(renderer, message->surface);
  SDL_FreeSurface(message->surface);
}

static inline void renderMessage(SDL_Renderer *renderer, Message *message) {
	SDL_RenderCopy(renderer, message->texture, NULL, &message->rect);

}

void initStaticMessages(SDL_Renderer *renderer) {
  Sans = TTF_OpenFont("Mono.ttf", 42 * HEIGHT / 1000);

  // Title Screen
  sprintf(message_titlescreen.text, "Blockamok");
	prepareMessage(renderer, Sans, &message_titlescreen, 3, color_black);
  message_titlescreen.rect.x = WIDTH * 0.5f - message_titlescreen.rect.w * 0.5f;
  message_titlescreen.rect.y = HEIGHT * 0.5f - message_titlescreen.rect.h * 0.5f;

  sprintf(message_titlescreen_pressstart.text, "Press %s to fall", btn_Start);
  prepareMessage(renderer, Sans, &message_titlescreen_pressstart, 1, color_black);
  message_titlescreen_pressstart.rect.x = WIDTH * 0.5f - message_titlescreen_pressstart.rect.w * 0.5f;
  message_titlescreen_pressstart.rect.y = HEIGHT * 0.75f - message_titlescreen_pressstart.rect.h * 0.5f;

  sprintf(message_titlescreen_pressselect.text, "Press %s for credits", btn_Select);
  prepareMessage(renderer, Sans, &message_titlescreen_pressselect, 1, color_black);
  message_titlescreen_pressselect.rect.x = WIDTH * 0.5f - message_titlescreen_pressselect.rect.w * 0.5f;
  message_titlescreen_pressselect.rect.y = HEIGHT * 0.75f - message_titlescreen_pressselect.rect.h * 0.5f + message_titlescreen_pressstart.rect.h;

  // Score Counter
  score_rect.y = -HEIGHT / 100;

	// Game Over Screen
  sprintf(message_gameover.text, "GAME OVER");
  prepareMessage(renderer, Sans, &message_gameover, 3, color_black);
  message_gameover.rect.x = WIDTH / 2 - message_gameover.rect.w / 2;
  message_gameover.rect.y = HEIGHT / 2 - message_gameover.rect.h / 2;

	// Pause Screen
  sprintf(message_paused.text, "PAUSED");
  prepareMessage(renderer, Sans, &message_paused, 3, color_black);
  message_paused.rect.x = WIDTH / 2 - message_paused.rect.w / 2;
  message_paused.rect.y = HEIGHT / 2 - message_paused.rect.h / 2;
}

inline void drawTitleScreenText(SDL_Renderer *renderer) {
	renderMessage(renderer, &message_titlescreen);
	renderMessage(renderer, &message_titlescreen_pressstart);
	renderMessage(renderer, &message_titlescreen_pressselect);
}

inline void drawCreditsText(SDL_Renderer *renderer) {
}

inline void drawScoreText(SDL_Renderer *renderer) {
  sprintf(message_score.text, "%d", (int)scoreVal);
  prepareMessage(renderer, Sans, &message_score, 1, color_black);
  message_score.rect.x = (WIDTH - message_score.rect.w) / 2;
	renderMessage(renderer, &message_score);
}

inline void drawGameOverText(SDL_Renderer *renderer) {
	renderMessage(renderer, &message_gameover);
}

inline void drawPausedText(SDL_Renderer *renderer) {
  renderMessage(renderer, &message_paused);
}