#include <string.h>
#include <stdio.h>

#include "./text.h"
#include "./input.h"
#include "./draw.h"
#include "./game.h"
#include "./fonts/Mono.h"

TTF_Font *Sans_126 = NULL;
int outlineSize_126;
TTF_Font *Sans_42 = NULL;
int outlineSize_42;
TTF_Font *Sans_38 = NULL;
int outlineSize_38;
SDL_Color color_black = {0, 0, 0};
SDL_Color color_white = {255, 255, 255};
SDL_Color color_gray = {208, 208, 208};
SDL_Color color_orange = {255, 160, 0};
SDL_Color color_red = {255, 92, 92};
SDL_Color color_blue = {128, 128, 255};

Message message_titlescreen;
Message message_titlescreen_play;
Message message_titlescreen_options;
Message message_titlescreen_highscore;
Message message_score;
Message message_game_cursor;
Message message_gameover;
Message message_gameover_highscore;
Message message_paused;
Message message_paused_quit;
#define INSTRUCTIONS_LENGTH_PART1 5
Message message_array_instructions_part1[INSTRUCTIONS_LENGTH_PART1];
#define INSTRUCTIONS_LENGTH_PART2 10
Message message_array_instructions_part2[INSTRUCTIONS_LENGTH_PART2];
Sint8 CREDITS_LENGTH;
Message message_array_credits[65];

bool credits_paused = false;

///////////////////
// TEXT CREATION //
///////////////////

static inline void destroyMessage(Message *message) {
  if (message->outline_texture != NULL) {
    SDL_DestroyTexture(message->outline_texture);
    message->outline_texture = NULL;
  }
  if (message->text_texture != NULL) {
    SDL_DestroyTexture(message->text_texture);
    message->text_texture = NULL;
  }
}

static inline void destroyFont(TTF_Font *font) {
	if (font != NULL) {
		TTF_CloseFont(font);
		font = NULL;
	}
}

inline void prepareMessage(SDL_Renderer *renderer, TTF_Font *font, int outlineSize, Message *message, float sizeMult, SDL_Color textColor, SDL_Color outlineColor) {
  destroyMessage(message);
  
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
  message->text_texture = SDL_CreateTextureFromSurface(renderer, textSurface);
  SDL_FreeSurface(textSurface);
}

inline void renderMessage(SDL_Renderer *renderer, Message *message) {
  SDL_RenderCopy(renderer, message->outline_texture, NULL, &message->outline_rect);
  SDL_RenderCopy(renderer, message->text_texture, NULL, &message->text_rect);
}

static inline void renderAndDestroyMessage(SDL_Renderer *renderer, Message *message) {
  renderMessage(renderer, message);
	SDL_DestroyTexture(message->text_texture);
	SDL_DestroyTexture(message->outline_texture);
}

static void mapTextArrayToMessageArray(SDL_Renderer *renderer, char *textArray[], Message *messageArray, Sint16 numMessages) {
  for (Sint16 i = 0; i < numMessages; i++) {
		if (textArray[i][0] == '\0') {
      textArray[i] = "    ";
		}
		char flag_font = textArray[i][0];
		TTF_Font *font;
		int outlineSize;
		char flag_color = textArray[i][1];
		SDL_Color text_color;
		const char *flag_text = &textArray[i][3];
#ifdef _MSC_VER
    strncpy_s(messageArray[i].text, TEXT_LINE_SIZE, flag_text, _TRUNCATE);
#else
    strncpy(messageArray[i].text, flag_text, TEXT_LINE_SIZE - 1);
#endif
    messageArray[i].text[TEXT_LINE_SIZE - 1] = '\0';
		switch (flag_font) {
		  case 'G':
			  font = Sans_126;
			  outlineSize = outlineSize_126;
			  break;
		  case 'L':
			  font = Sans_42;
			  outlineSize = outlineSize_42;
			  break;
		  case 'M':
			  font = Sans_38;
			  outlineSize = outlineSize_38;
			  break;
		  default:
        font = Sans_38;
        outlineSize = outlineSize_38;
			  break;
		}
		switch (flag_color) {
		  case 'B':
			  text_color = color_black;
			  break;
		  case 'W':
			  text_color = color_white;
			  break;
			case 'G':
				text_color = color_gray;
				break;
		  case 'o':
        text_color = color_orange;
        break;
		  case 'r':
        text_color = color_red;
        break;
			case 'b':
				text_color = color_blue;
				break;
		  default:
        text_color = color_white;
        break;
		}
		prepareMessage(renderer, font, outlineSize, &messageArray[i], 1, text_color, color_black);
	}
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
	setMessagePosX(message, (int)(GAME_WIDTH * x - message->text_rect.w * 0.5f));
}

static inline void setMessagePosRelativeToScreenY(Message *message, float y) {
	setMessagePosY(message, (int)(GAME_HEIGHT * y - message->text_rect.h * 0.5f));
}

inline void setMessagePosRelativeToScreen(Message *message, float x, float y) {
	setMessagePosRelativeToScreenX(message, x);
	setMessagePosRelativeToScreenY(message, y);
}

static inline void setMessagePosRelativeToScreenX_LeftAlign(Message* message, float x) {
  setMessagePosX(message, (int)(GAME_WIDTH * x));
}

inline void setMessagePosRelativeToScreen_LeftAlign(Message *message, float x, float y) {
  setMessagePosRelativeToScreenX_LeftAlign(message, x);
  setMessagePosRelativeToScreenY(message, y);
}

///////////////////
// GAME-SPECIFIC //
///////////////////

static void initStaticMessages_TitleScreen() {
  snprintf(message_titlescreen.text, TEXT_LINE_SIZE, "Blockamok");
  prepareMessage(renderer, Sans_126, outlineSize_126, &message_titlescreen, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_titlescreen, 0.5f, 0.4f);

  snprintf(message_titlescreen_play.text, TEXT_LINE_SIZE, "Press %s to fly", btn_Start);
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_titlescreen_play, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_titlescreen_play, 0.5f, 0.6f);

  snprintf(message_titlescreen_options.text, TEXT_LINE_SIZE, "Press %s for options", btn_Select);
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_titlescreen_options, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_titlescreen_options, 0.5f, 0.7f);

  refreshHighScoreText(renderer);
}

static void initStaticMessages_Game() {
  setMessagePosRelativeToScreenY(&message_score, 0.01f);
  message_score.outline_rect.y -= (int)fmax(GAME_HEIGHT / 240, 3);

  snprintf(message_game_cursor.text, TEXT_LINE_SIZE, "+");
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_game_cursor, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_game_cursor, 0.5f, 0.5f);
  SDL_SetTextureAlphaMod(message_game_cursor.outline_texture, 64);
  SDL_SetTextureAlphaMod(message_game_cursor.text_texture, 64);

  snprintf(message_gameover.text, TEXT_LINE_SIZE, "GAME OVER");
  prepareMessage(renderer, Sans_126, outlineSize_126, &message_gameover, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_gameover, 0.5f, 0.5f);

  snprintf(message_gameover_highscore.text, TEXT_LINE_SIZE, "New High Score!");
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_gameover_highscore, 1, color_orange, color_black);
  setMessagePosRelativeToScreen(&message_gameover_highscore, 0.5f, 0.75f);

  snprintf(message_paused.text, TEXT_LINE_SIZE, "PAUSED");
  prepareMessage(renderer, Sans_126, outlineSize_126, &message_paused, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_paused, 0.5f, 0.5f);

  snprintf(message_paused_quit.text, TEXT_LINE_SIZE, "Press %s to quit", btn_Select);
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_paused_quit, 1, color_white, color_black);
  setMessagePosRelativeToScreen(&message_paused_quit, 0.5f, 0.65f);
}

static void initStaticMessages_Instructions(bool compactView) {
  char *message_array_instructions_text_part1[] = {
  "Lo Dodge the incoming blocks!",
  malloc(TEXT_LINE_SIZE),
  malloc(TEXT_LINE_SIZE),
  malloc(TEXT_LINE_SIZE),
  malloc(TEXT_LINE_SIZE)
  };
  snprintf(message_array_instructions_text_part1[1], TEXT_LINE_SIZE, "MG Hold %s or %s to speed up.", btn_A, btn_B);
  snprintf(message_array_instructions_text_part1[2], TEXT_LINE_SIZE, "MG Press %s or %s to toggle cursor.", btn_X, btn_Y);
  snprintf(message_array_instructions_text_part1[3], TEXT_LINE_SIZE, "MG Press %s to pause.", btn_Start);
  snprintf(message_array_instructions_text_part1[4], TEXT_LINE_SIZE, "MG Press %s or %s to change music.", btn_L, btn_R);
  mapTextArrayToMessageArray(renderer, message_array_instructions_text_part1, message_array_instructions_part1, INSTRUCTIONS_LENGTH_PART1);

  if (!compactView) {
    char *message_array_instructions_text_part2[] = {
      "Lo Two different movement types!",
      "Lo Press Left/Right to toggle:",
      "Lr Type A",
      "Mr Up/Down and Left/Right movement are",
      "Mr independent, so diagonal is faster.",
      "",
      "Lb Type B",
      "Mb Speed is the same regardless of direction.",
      "Mb More analog stick-friendly.",
      "",
    };
    mapTextArrayToMessageArray(renderer, message_array_instructions_text_part2, message_array_instructions_part2, INSTRUCTIONS_LENGTH_PART2);
  }
  else {
    char *message_array_instructions_text_part2[] = {
      "Lo Two different movement types!",
      "Lo Press Left/Right to toggle:",
      "Lr Type A",
      "Mr Up/Down and Left/Right movement",
      "Mr are independent, so diagonal is",
      "Mr faster.",
      "Lb Type B",
      "Mb Speed is the same",
      "Mb regardless of direction.",
      "Mb More analog stick-friendly."
    };
    mapTextArrayToMessageArray(renderer, message_array_instructions_text_part2, message_array_instructions_part2, INSTRUCTIONS_LENGTH_PART2);
  }

  setMessagePosRelativeToScreen(&message_array_instructions_part1[0], 0.5f, 0.15f);
  setMessagePosRelativeToScreen(&message_array_instructions_part1[1], 0.5f, 0.225f);
  setMessagePosRelativeToScreen(&message_array_instructions_part1[2], 0.5f, 0.3f);
  setMessagePosRelativeToScreen(&message_array_instructions_part1[3], 0.5f, 0.375f);
  setMessagePosRelativeToScreen(&message_array_instructions_part1[4], 0.5f, 0.45f);
  setMessagePosRelativeToScreen(&message_array_instructions_part2[0], 0.5f, 0.575f);
  setMessagePosRelativeToScreen(&message_array_instructions_part2[1], 0.5f, 0.65f);
  setMessagePosRelativeToScreen(&message_array_instructions_part2[2], 0.5f, 0.725f);
  setMessagePosRelativeToScreen(&message_array_instructions_part2[3], 0.5f, 0.8f);
  setMessagePosRelativeToScreen(&message_array_instructions_part2[4], 0.5f, 0.875f);
  setMessagePosRelativeToScreen(&message_array_instructions_part2[5], 0.5f, 0.95f);
  setMessagePosRelativeToScreen(&message_array_instructions_part2[6], 0.5f, 0.725f);
  setMessagePosRelativeToScreen(&message_array_instructions_part2[7], 0.5f, 0.8f);
  setMessagePosRelativeToScreen(&message_array_instructions_part2[8], 0.5f, 0.875f);
  setMessagePosRelativeToScreen(&message_array_instructions_part2[9], 0.5f, 0.95f);
}

static void initStaticMessages_Credits(bool compactView) {
  if (!compactView) {
    char *message_array_credits_text[] = {
      "Lo BLOCKAMOK v2.0",
      "",
      "Mr Carl Riis",
      "Mr Original game",
      "MW https://github.com/carltheperson/blockamok",
      "",
      "Mb Mode8fx",
      "Mb v2.0 update and ports",
      "MW https://github.com/Mode8fx/blockamok",
      "",
      "Lo MUSIC",
      "",
      "MG Raina ft. Coaxcable",
      "MW \"Spaceranger 50k\"",
      "",
      "MG Cobburn and Monty",
      "MW \"Falling Up\"",
      "",
      "MG Diomatic",
      "MW \"Falling People\"",
      "",
      "MG mano and ske",
      "MW \"Darkness in da Night\"",
      "",
      "MG Diáblo",
      "MW \"Dance 2 Insanity\"",
      "",
      "MG All music obtained from modarchive.org",
      "",
      "MG \"Spaceranger 50k\" and \"Falling People\"",
      "MG edited by Mode8fx for looping purposes",
      "",
      "Lo SOUND EFFECTS",
      "",
      "MG claudeb",
      "MW https://opengameart.org",
      "MW /content/buzz-grid-sounds",
      "",
      "Lo LIBRARIES",
      "",
      "MW SDL2",
      "MW SDL2_mixer",
      "MW SDL2_ttf",
      "",
      "Lo THANKS FOR PLAYING!",
      "",
      "MG Blockamok is available on a wide variety",
      "MG of homebrew-enabled systems, old and new.",
      "MG Play it everywhere!",
      "MW https://github.com/Mode8fx/blockamok",
      "",
      "",
      "MG One last thing: to reset your high score,",
      "MG go back to the title screen and press",
      "Mr Up B Down B Left B Right B"
    };
    CREDITS_LENGTH = 55;
    mapTextArrayToMessageArray(renderer, message_array_credits_text, message_array_credits, CREDITS_LENGTH);
  }
  else {
    char *message_array_credits_text[] = {
      "Lo BLOCKAMOK v2.0",
      "",
      "Mr Carl Riis",
      "Mr Original game",
      "MW https://github.com",
      "MW /carltheperson/blockamok",
      "",
      "Mb Mode8fx",
      "Mb v2.0 update and ports",
      "MW https://github.com",
      "MW /Mode8fx/blockamok",
      "",
      "Lo MUSIC",
      "",
      "MG Raina ft. Coaxcable",
      "MW \"Spaceranger 50k\"",
      "",
      "MG Cobburn and Monty",
      "MW \"Falling Up\"",
      "",
      "MG Diomatic",
      "MW \"Falling People\"",
      "",
      "MG mano and ske",
      "MW \"Darkness in da Night\"",
      "",
      "MG Diáblo",
      "MW \"Dance 2 Insanity\"",
      "",
      "MG All music obtained from",
      "MG modarchive.org",
      "",
      "MG \"Spaceranger 50k\" and",
      "MG \"Falling People\" edited by",
      "MG Mode8fx for looping purposes",
      "",
      "Lo SOUND EFFECTS",
      "",
      "MG claudeb",
      "MW https://opengameart.org",
      "MW /content/buzz-grid-sounds",
      "",
      "Lo LIBRARIES",
      "",
      "MW SDL2",
      "MW SDL2_mixer",
      "MW SDL2_ttf",
      "",
      "Lo THANKS FOR PLAYING!",
      "",
      "MG Blockamok is available on a wide",
      "MG variety of homebrew-enabled",
      "MG systems, old and new.",
      "MG Play it everywhere!",
      "MW https://github.com",
      "MW /Mode8fx/blockamok",
      "",
      "",
      "MG One last thing: to reset your",
      "MG high score, go back to the",
      "MG title screen and press",
      "Mr Up B Down B Left B Right B"
    };
    CREDITS_LENGTH = 62;
    mapTextArrayToMessageArray(renderer, message_array_credits_text, message_array_credits, CREDITS_LENGTH);
  }
}

void initStaticMessages(SDL_Renderer *renderer) {
  cleanUpText();
  bool compactView = GAME_HEIGHT <= 289;

  SDL_RWops *rw = SDL_RWFromMem(Mono_ttf, Mono_ttf_len);

  int textSize_126 = (int)fmax(126 * GAME_HEIGHT / 1000, 36);
  outlineSize_126 = (int)fmax(textSize_126 / 10, 3);
  Sans_126 = TTF_OpenFontRW(rw, 0, textSize_126);

  SDL_RWseek(rw, 0, RW_SEEK_SET);
  int textSize_42 = (int)fmax(42 * GAME_HEIGHT / 1000, 13);
  outlineSize_42 = (int)fmax(textSize_42 / 10, 3);
  Sans_42 = TTF_OpenFontRW(rw, 0, textSize_42);

  SDL_RWseek(rw, 0, RW_SEEK_SET);
  int textSize_38 = (int)fmax(38 * GAME_HEIGHT / 1000, 11);
  outlineSize_38 = (int)fmax(textSize_38 / 10, 3);
  if (compactView) {
    outlineSize_38 = 2;
  }
  Sans_38 = TTF_OpenFontRW(rw, 1, textSize_38);

  initStaticMessages_TitleScreen();
  initStaticMessages_Game();
  initStaticMessages_Options(renderer);
  initStaticMessages_Instructions(compactView);
  initStaticMessages_Credits(compactView);

  destroyFont(Sans_126); // no longer needed
}

inline void drawTitleScreenText(SDL_Renderer *renderer, bool drawSecondaryText) {
	renderMessage(renderer, &message_titlescreen);
  if (drawSecondaryText) {
    renderMessage(renderer, &message_titlescreen_play);
    renderMessage(renderer, &message_titlescreen_options);
    renderMessage(renderer, &message_titlescreen_highscore);
  }
}

inline void drawInstructionsText(SDL_Renderer *renderer) {
  renderMessage(renderer, &message_array_instructions_part1[0]);
  renderMessage(renderer, &message_array_instructions_part1[1]);
  renderMessage(renderer, &message_array_instructions_part1[2]);
  renderMessage(renderer, &message_array_instructions_part1[3]);
  renderMessage(renderer, &message_array_instructions_part1[4]);
  renderMessage(renderer, &message_array_instructions_part2[0]);
  renderMessage(renderer, &message_array_instructions_part2[1]);
  if (!isAnalog) {
    renderMessage(renderer, &message_array_instructions_part2[2]);
    renderMessage(renderer, &message_array_instructions_part2[3]);
    renderMessage(renderer, &message_array_instructions_part2[4]);
    renderMessage(renderer, &message_array_instructions_part2[5]);
  } else {
    renderMessage(renderer, &message_array_instructions_part2[6]);
    renderMessage(renderer, &message_array_instructions_part2[7]);
    renderMessage(renderer, &message_array_instructions_part2[8]);
    renderMessage(renderer, &message_array_instructions_part2[9]);
  }
}

inline void drawCreditsText(SDL_Renderer *renderer, Uint64 now) {
  if (keyPressed(INPUT_A)) {
		credits_paused = !credits_paused;
  }
  if (keyHeld(INPUT_UP)) {
    credits_startTime += (Uint64)(36000 * deltaTime);
  } else if (keyHeld(INPUT_DOWN)) {
    credits_startTime -= (Uint64)(12000 * deltaTime);
	} else if (credits_paused) {
    credits_startTime += (Uint64)(12000 * deltaTime);
  }

  Uint64 timer = now - credits_startTime;
  float offset_timer = 0.15f * timer / 1000; // scroll speed
  if (CREDITS_LENGTH > 60) { // compact view
    offset_timer *= 1.1f;
  }
  for (int i = 0; i < CREDITS_LENGTH; i++) {
		float offset_index = 0.06f * i; // spacing between lines
    float startPosY = 1.0f + offset_index - offset_timer;
    if (startPosY < -0.1f) {
      if (i == CREDITS_LENGTH - 1 && startPosY < -0.5f) {
				credits_startTime = now; // loop credits
      }
      continue;
		} else if (startPosY > 1.2f) {
			break;
		}
		setMessagePosRelativeToScreen(&message_array_credits[i], 0.5f, startPosY);
		renderMessage(renderer, &message_array_credits[i]);
  }
}

inline void drawScoreText(SDL_Renderer *renderer) {
  snprintf(message_score.text, TEXT_LINE_SIZE, "%d", (int)scoreVal);
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_score, 1, color_white, color_black);
	setMessagePosRelativeToScreenX(&message_score, 0.5f);
  renderMessage(renderer, &message_score);
}

inline void drawCursor(SDL_Renderer *renderer) {
  if (showCursor) {
    renderMessage(renderer, &message_game_cursor);
  }
}

inline void drawGameOverText(SDL_Renderer *renderer) {
  renderMessage(renderer, &message_gameover);
  if (newHighScore) {
    renderMessage(renderer, &message_gameover_highscore);
  }
}

inline void drawPausedText(SDL_Renderer *renderer) {
  renderMessage(renderer, &message_paused);
  renderMessage(renderer, &message_paused_quit);
}

inline void refreshHighScoreText(SDL_Renderer *renderer) {
  snprintf(message_titlescreen_highscore.text, TEXT_LINE_SIZE, "High Score: %d", highScoreVal);
  prepareMessage(renderer, Sans_42, outlineSize_42, &message_titlescreen_highscore, 1, color_orange, color_black);
  setMessagePosRelativeToScreen(&message_titlescreen_highscore, 0.5f, 0.9f);
}

void cleanUpText() {
  destroyMessage(&message_titlescreen);
  destroyMessage(&message_titlescreen_play);
  destroyMessage(&message_titlescreen_options);
  destroyMessage(&message_titlescreen_highscore);
  destroyMessage(&message_score);
  destroyMessage(&message_game_cursor);
  destroyMessage(&message_gameover);
  destroyMessage(&message_gameover_highscore);
  destroyMessage(&message_paused);
  destroyMessage(&message_paused_quit);

  for (int i = 0; i < INSTRUCTIONS_LENGTH_PART1; i++) {
    destroyMessage(&message_array_instructions_part1[i]);
  }
  for (int i = 0; i < INSTRUCTIONS_LENGTH_PART2; i++) {
    destroyMessage(&message_array_instructions_part2[i]);
  }
  for (int i = 0; i < CREDITS_LENGTH; i++) {
    destroyMessage(&message_array_credits[i]);
  }

  destroyFont(Sans_42);
  destroyFont(Sans_38);
}