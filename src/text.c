#include "./text.h"
#include "./input.h"
#include "./draw.h"
#include "./game.h"

TTF_Font *Sans = NULL;
SDL_Color color_black = {0, 0, 0};
SDL_Color color_white = {255, 255, 255};

Message message_titlescreen;
Message message_titlescreen_pressstart;
Message message_titlescreen_pressselect;
Message message_score;
Message message_gameover;
Message message_paused;
Message message_paused_quit;
Message message_credits_1;
Message message_credits_2;
Message message_credits_3;
Message message_credits_4;

SDL_Rect score_rect;

///////////////////
// TEXT CREATION //
///////////////////

static inline void prepareMessage(SDL_Renderer *renderer, TTF_Font *font, Message *message, float sizeMult, SDL_Color textColor, SDL_Color outlineColor) {
  // Create the outline
  SDL_Surface *outlineSurface = TTF_RenderText_Solid(font, message->text, outlineColor);
  message->outline_rect.w = (int)(outlineSurface->w * sizeMult * 1.1f);
  message->outline_rect.h = (int)(outlineSurface->h * sizeMult * 1.1f);
  message->outline_texture = SDL_CreateTextureFromSurface(renderer, outlineSurface);
  SDL_FreeSurface(outlineSurface);

  // Create the main text
  SDL_Surface *textSurface = TTF_RenderText_Solid(font, message->text, textColor);
  message->text_rect.w = (int)(textSurface->w * sizeMult);
  message->text_rect.h = (int)(textSurface->h * sizeMult);
  if (message->text_texture != NULL) SDL_DestroyTexture(message->text_texture);
  message->text_texture = SDL_CreateTextureFromSurface(renderer, textSurface);
  SDL_FreeSurface(textSurface);

  // Render the main text
  SDL_RenderCopy(renderer, message->text_texture, NULL, &message->text_rect);
}

static inline void renderMessage(SDL_Renderer *renderer, Message *message) {
  //SDL_RenderCopy(renderer, message->outline_texture, NULL, &message->outline_rect);
	SDL_RenderCopy(renderer, message->text_texture, NULL, &message->text_rect);
}

///////////////////
// TEXT POSITION //
///////////////////

static void setMessagePosX(Message *message, int x) {
  message->text_rect.x = x;
  message->outline_rect.x = message->text_rect.x - (message->outline_rect.w - message->text_rect.w) / 2;
}

static void setMessagePosY(Message *message, int y) {
	message->text_rect.y = y;
	message->outline_rect.y = message->text_rect.y - (message->outline_rect.h - message->text_rect.h) / 2;
}

static void setMessagePos(Message *message, int x, int y) {
	setMessagePosX(message, x);
	setMessagePosY(message, y);
}

static void setMessagePosRelativeToScreenX(Message *message, float x) {
	setMessagePosX(message, (int)(WIDTH * x - message->text_rect.w * 0.5f));
}

static void setMessagePosRelativeToScreenY(Message *message, float y) {
	setMessagePosY(message, (int)(HEIGHT * y - message->text_rect.h * 0.5f));
}

static void setMessagePosRelativeToScreen(Message *message, float x, float y) {
	setMessagePosRelativeToScreenX(message, x);
	setMessagePosRelativeToScreenY(message, y);
}

///////////////////
// GAME-SPECIFIC //
///////////////////

void initStaticMessages(SDL_Renderer *renderer) {
  Sans = TTF_OpenFont("Mono.ttf", 42 * HEIGHT / 1000);

  // Title Screen
  sprintf(message_titlescreen.text, "Blockamok");
	prepareMessage(renderer, Sans, &message_titlescreen, 3, color_black, color_white);
  setMessagePosRelativeToScreen(&message_titlescreen, 0.5f, 0.5f);

  sprintf(message_titlescreen_pressstart.text, "Press %s to fly", btn_Start);
  prepareMessage(renderer, Sans, &message_titlescreen_pressstart, 1, color_black, color_white);
  setMessagePosRelativeToScreen(&message_titlescreen_pressstart, 0.5f, 0.75f);

  sprintf(message_titlescreen_pressselect.text, "Press %s for credits", btn_Select);
  prepareMessage(renderer, Sans, &message_titlescreen_pressselect, 1, color_black, color_white);
  setMessagePosRelativeToScreen(&message_titlescreen_pressstart, 0.5f, 0.85f);

  // Score Counter
  score_rect.y = -HEIGHT / 100;

	// Game Over Screen
  sprintf(message_gameover.text, "GAME OVER");
  prepareMessage(renderer, Sans, &message_gameover, 3, color_black, color_white);
  setMessagePosRelativeToScreen(&message_gameover, 0.5f, 0.5f);

	// Pause Screen
  sprintf(message_paused.text, "PAUSED");
  prepareMessage(renderer, Sans, &message_paused, 3, color_black, color_white);
  setMessagePosRelativeToScreen(&message_paused, 0.5f, 0.5f);

  sprintf(message_paused_quit.text, "Press %s to quit", btn_Select);
  prepareMessage(renderer, Sans, &message_paused_quit, 1, color_black, color_white);
  setMessagePosRelativeToScreen(&message_paused_quit, 0.5f, 0.65f);

  // Credits Screen
  sprintf(message_credits_1.text, "Original game by Carl Riis");
  prepareMessage(renderer, Sans, &message_credits_1, 1, color_black, color_white);
  setMessagePosRelativeToScreen(&message_credits_1, 0.5f, 0.3f);

  sprintf(message_credits_2.text, "https://github.com/carltheperson/blockamok");
  prepareMessage(renderer, Sans, &message_credits_2, 0.8f, color_black, color_white);
  setMessagePosRelativeToScreen(&message_credits_2, 0.5f, 0.4f);

  sprintf(message_credits_3.text, "v2.0 update and ports by Mode8fx");
  prepareMessage(renderer, Sans, &message_credits_3, 1, color_black, color_white);
  setMessagePosRelativeToScreen(&message_credits_3, 0.5f, 0.7f);

  sprintf(message_credits_4.text, "https://github.com/Mode8fx/blockamok");
  prepareMessage(renderer, Sans, &message_credits_4, 0.8f, color_black, color_white);
  setMessagePosRelativeToScreen(&message_credits_4, 0.5f, 0.8f);
}

inline void drawTitleScreenText(SDL_Renderer *renderer) {
	renderMessage(renderer, &message_titlescreen);
	renderMessage(renderer, &message_titlescreen_pressstart);
	renderMessage(renderer, &message_titlescreen_pressselect);
}

inline void drawCreditsText(SDL_Renderer *renderer) {
  renderMessage(renderer, &message_credits_1);
  renderMessage(renderer, &message_credits_2);
  renderMessage(renderer, &message_credits_3);
  renderMessage(renderer, &message_credits_4);
}

inline void drawScoreText(SDL_Renderer *renderer) {
  sprintf(message_score.text, "%d", (int)scoreVal);
  prepareMessage(renderer, Sans, &message_score, 1, color_black, color_white);
	setMessagePosRelativeToScreenX(&message_score, 0.5f);
	renderMessage(renderer, &message_score);
}

inline void drawGameOverText(SDL_Renderer *renderer) {
	renderMessage(renderer, &message_gameover);
}

inline void drawPausedText(SDL_Renderer *renderer) {
  renderMessage(renderer, &message_paused);
  renderMessage(renderer, &message_paused_quit);
}