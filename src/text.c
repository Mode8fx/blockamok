#include "./text.h"
#include "./input.h"
#include "./draw.h"
#include "./game.h"
#include "./fonts/Mono.h"

TTF_Font *Sans_42 = NULL;
int outlineSize_42;
TTF_Font *Sans_38 = NULL;
int outlineSize_38;
SDL_Color color_black = {0, 0, 0};
SDL_Color color_white = {255, 255, 255};
SDL_Color color_orange = {255, 160, 0};
SDL_Color color_red = {255, 92, 92};

Message message_titlescreen;
Message message_titlescreen_play;
Message message_titlescreen_instructions;
Message message_titlescreen_credits;
Message message_titlescreen_quit;
Message message_score;
Message message_gameover;
Message message_paused;
Message message_paused_quit;
Message message_instructions_1;
Message message_instructions_2;
Message message_instructions_3;
Message message_instructions_4;
Message message_instructions_5;
Message message_instructions_6;
Message message_instructions_7a;
Message message_instructions_7b;
Message message_instructions_8a;
Message message_instructions_8b;
Message message_instructions_9a;
Message message_instructions_9b;
Message message_credits_1;
Message message_credits_2;
Message message_credits_3;
Message message_credits_4;

SDL_Rect score_rect;

///////////////////
// TEXT CREATION //
///////////////////

static inline void prepareMessage(SDL_Renderer *renderer, TTF_Font *font, int outlineSize, Message *message, float sizeMult, SDL_Color textColor, SDL_Color outlineColor) {
  // Create the outline
  TTF_SetFontOutline(font, outlineSize);
  SDL_Surface *outlineSurface = TTF_RenderText_Solid(font, message->text, outlineColor);
  message->outline_rect.w = (int)(outlineSurface->w * sizeMult);
  message->outline_rect.h = (int)(outlineSurface->h * sizeMult);
  message->outline_texture = SDL_CreateTextureFromSurface(renderer, outlineSurface);
  SDL_FreeSurface(outlineSurface);

  // Create the main text
  TTF_SetFontOutline(font, 0);
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
  SDL_RenderCopy(renderer, message->outline_texture, NULL, &message->outline_rect);
	SDL_RenderCopy(renderer, message->text_texture, NULL, &message->text_rect);
}

static inline void renderAndDestroyMessage(SDL_Renderer *renderer, Message *message) {
  renderMessage(renderer, message);
	SDL_DestroyTexture(message->text_texture);
	SDL_DestroyTexture(message->outline_texture);
}

///////////////////
// TEXT POSITION //
///////////////////

static inline void setMessagePosX(Message *message, int x) {
  message->text_rect.x = x;
  message->outline_rect.x = message->text_rect.x - (message->outline_rect.w - message->text_rect.w) / 2;
}

static inline void setMessagePosY(Message *message, int y) {
	message->text_rect.y = y;
	message->outline_rect.y = message->text_rect.y - (message->outline_rect.h - message->text_rect.h) / 2;
}

static inline void setMessagePos(Message *message, int x, int y) {
	setMessagePosX(message, x);
	setMessagePosY(message, y);
}

static inline void setMessagePosRelativeToScreenX(Message *message, float x) {
	setMessagePosX(message, (int)(WIDTH * x - message->text_rect.w * 0.5f));
}

static inline void setMessagePosRelativeToScreenY(Message *message, float y) {
	setMessagePosY(message, (int)(HEIGHT * y - message->text_rect.h * 0.5f));
}

static inline void setMessagePosRelativeToScreen(Message *message, float x, float y) {
	setMessagePosRelativeToScreenX(message, x);
	setMessagePosRelativeToScreenY(message, y);
}

///////////////////
// GAME-SPECIFIC //
///////////////////

void initStaticMessages(SDL_Renderer *renderer) {
  // Initialize TTF_Fonts
  SDL_RWops *rw = SDL_RWFromMem(Mono_ttf, Mono_ttf_len);

  int textSize_42 = 42 * HEIGHT / 1000;
  outlineSize_42 = textSize_42 / 10;
  Sans_42 = TTF_OpenFontRW(rw, 0, textSize_42);

  SDL_RWseek(rw, 0, RW_SEEK_SET);
  int textSize_38 = 38 * HEIGHT / 1000;
  outlineSize_38 = textSize_38 / 10;
  Sans_38 = TTF_OpenFontRW(rw, 1, textSize_38);

  // Title Screen
  sprintf(message_titlescreen.text, "Blockamok");
	prepareMessage(renderer, Sans_42, outlineSize_42, &message_titlescreen, 3, color_white, color_black);
  setMessagePosRelativeToScreen(&message_titlescreen, 0.5f, 0.4f);

  sprintf(message_titlescreen_play.text, "Press %s to fly", btn_Start);
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_titlescreen_play, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_titlescreen_play, 0.5f, 0.6f);

  sprintf(message_titlescreen_instructions.text, "Press %s for instructions", btn_X);
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_titlescreen_instructions, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_titlescreen_instructions, 0.5f, 0.7f);

  sprintf(message_titlescreen_credits.text, "Press %s for credits", btn_Y);
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_titlescreen_credits, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_titlescreen_credits, 0.5f, 0.8f);

  sprintf(message_titlescreen_quit.text, "Press %s to quit", btn_Select);
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_titlescreen_quit, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_titlescreen_quit, 0.5f, 0.9f);

  // Score Counter
  score_rect.y = -HEIGHT / 100;

	// Game Over Screen
  sprintf(message_gameover.text, "GAME OVER");
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_gameover, 3, color_white, color_black);
  setMessagePosRelativeToScreen(&message_gameover, 0.5f, 0.5f);

	// Pause Screen
  sprintf(message_paused.text, "PAUSED");
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_paused, 3, color_white, color_black);
  setMessagePosRelativeToScreen(&message_paused, 0.5f, 0.5f);

  sprintf(message_paused_quit.text, "Press %s to quit", btn_Select);
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_paused_quit, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_paused_quit, 0.5f, 0.65f);

  // Instructions Screen
  sprintf(message_instructions_1.text, "Dodge the incoming blocks!");
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_instructions_1, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_instructions_1, 0.5f, 0.15f);

  sprintf(message_instructions_2.text, "Hold %s or %s to speed up.", btn_A, btn_B);
  prepareMessage(renderer, Sans_38, outlineSize_38, &message_instructions_2, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_instructions_2, 0.5f, 0.25f);

  sprintf(message_instructions_3.text, "Press %s to pause.", btn_Start);
  prepareMessage(renderer, Sans_38, outlineSize_38, &message_instructions_3, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_instructions_3, 0.5f, 0.325f);

  sprintf(message_instructions_4.text, "Press %s or %s to change music.", btn_L, btn_R);
  prepareMessage(renderer, Sans_38, outlineSize_38, &message_instructions_4, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_instructions_4, 0.5f, 0.4f);

  sprintf(message_instructions_5.text, "Two different movement types!");
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_instructions_5, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_instructions_5, 0.5f, 0.55f);

  sprintf(message_instructions_6.text, "Press Left/Right to toggle:");
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_instructions_6, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_instructions_6, 0.5f, 0.625f);

  sprintf(message_instructions_7a.text, "Type A");
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_instructions_7a, 1, color_orange, color_black);
  setMessagePosRelativeToScreen(&message_instructions_7a, 0.5f, 0.7f);

  sprintf(message_instructions_8a.text, "Up/Down and Left/Right movement are");
  prepareMessage(renderer, Sans_38, outlineSize_38, &message_instructions_8a, 1, color_orange, color_black);
  setMessagePosRelativeToScreen(&message_instructions_8a, 0.5f, 0.775f);

  sprintf(message_instructions_9a.text, "independent, so diagonal is faster.");
  prepareMessage(renderer, Sans_38, outlineSize_38, &message_instructions_9a, 1, color_orange, color_black);
  setMessagePosRelativeToScreen(&message_instructions_9a, 0.5f, 0.85f);

  sprintf(message_instructions_7b.text, "Type B");
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_instructions_7b, 1, color_red, color_black);
  setMessagePosRelativeToScreen(&message_instructions_7b, 0.5f, 0.7f);

  sprintf(message_instructions_8b.text, "Speed is the same regardless of direction.");
  prepareMessage(renderer, Sans_38, outlineSize_38, &message_instructions_8b, 1, color_red, color_black);
  setMessagePosRelativeToScreen(&message_instructions_8b, 0.5f, 0.775f);

  sprintf(message_instructions_9b.text, "More analog stick-friendly.");
  prepareMessage(renderer, Sans_38, outlineSize_38, &message_instructions_9b, 1, color_red, color_black);
  setMessagePosRelativeToScreen(&message_instructions_9b, 0.5f, 0.85f);

  // Credits Screen
  sprintf(message_credits_1.text, "Original game by Carl Riis");
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_credits_1, 1, color_orange, color_black);
  setMessagePosRelativeToScreen(&message_credits_1, 0.5f, 0.3f);

  sprintf(message_credits_2.text, "https://github.com/carltheperson/blockamok");
  prepareMessage(renderer, Sans_38, outlineSize_38, &message_credits_2, 1, color_orange, color_black);
  setMessagePosRelativeToScreen(&message_credits_2, 0.5f, 0.4f);

  sprintf(message_credits_3.text, "v2.0 update and ports by Mode8fx");
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_credits_3, 1, color_red, color_black);
  setMessagePosRelativeToScreen(&message_credits_3, 0.5f, 0.7f);

  sprintf(message_credits_4.text, "https://github.com/Mode8fx/blockamok");
  prepareMessage(renderer, Sans_38, outlineSize_38, &message_credits_4, 1, color_red, color_black);
  setMessagePosRelativeToScreen(&message_credits_4, 0.5f, 0.8f);
}

inline void drawTitleScreenText(SDL_Renderer *renderer) {
	renderMessage(renderer, &message_titlescreen);
	renderMessage(renderer, &message_titlescreen_play);
	renderMessage(renderer, &message_titlescreen_instructions);
	renderMessage(renderer, &message_titlescreen_credits);
	renderMessage(renderer, &message_titlescreen_quit);
}

inline void drawInstructionsText(SDL_Renderer *renderer) {
  renderMessage(renderer, &message_instructions_1);
	renderMessage(renderer, &message_instructions_2);
	renderMessage(renderer, &message_instructions_3);
	renderMessage(renderer, &message_instructions_4);
	renderMessage(renderer, &message_instructions_5);
  renderMessage(renderer, &message_instructions_6);
  if (!isAnalog) {
		renderMessage(renderer, &message_instructions_7a);
		renderMessage(renderer, &message_instructions_8a);
		renderMessage(renderer, &message_instructions_9a);
	} else {
    renderMessage(renderer, &message_instructions_7b);
    renderMessage(renderer, &message_instructions_8b);
    renderMessage(renderer, &message_instructions_9b);
  }
}

inline void drawCreditsText(SDL_Renderer *renderer) {
  renderMessage(renderer, &message_credits_1);
  renderMessage(renderer, &message_credits_2);
  renderMessage(renderer, &message_credits_3);
  renderMessage(renderer, &message_credits_4);
}

inline void drawScoreText(SDL_Renderer *renderer) {
  sprintf(message_score.text, "%d", (int)scoreVal);
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_score, 1, color_white, color_black);
	setMessagePosRelativeToScreenX(&message_score, 0.5f);
  renderAndDestroyMessage(renderer, &message_score);
}

inline void drawGameOverText(SDL_Renderer *renderer) {
	renderMessage(renderer, &message_gameover);
}

inline void drawPausedText(SDL_Renderer *renderer) {
  renderMessage(renderer, &message_paused);
  renderMessage(renderer, &message_paused_quit);
}