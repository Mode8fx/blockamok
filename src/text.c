#include <string.h>

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
SDL_Color color_gray = {192, 192, 192};
SDL_Color color_orange = {255, 160, 0};
SDL_Color color_red = {255, 92, 92};
SDL_Color color_blue = {128, 128, 255};

Message message_titlescreen;
Message message_titlescreen_play;
Message message_titlescreen_instructions;
Message message_titlescreen_credits;
Message message_titlescreen_quit;
Message message_score;
Message message_gameover;
Message message_paused;
Message message_paused_quit;
#define INSTRUCTIONS_LENGTH 12
Message message_array_instructions[INSTRUCTIONS_LENGTH];
#define CREDITS_LENGTH 48
Message message_array_credits[CREDITS_LENGTH];

SDL_Rect score_rect;

bool credits_paused = false;

///////////////////
// TEXT CREATION //
///////////////////

static inline void prepareMessage(SDL_Renderer *renderer, TTF_Font *font, int outlineSize, Message *message, float sizeMult, SDL_Color textColor, SDL_Color outlineColor) {
  // Create the outline
  TTF_SetFontOutline(font, outlineSize);
  SDL_Surface *outlineSurface = TTF_RenderText_Solid(font, message->text, outlineColor);
  message->outline_rect.w = (int)(outlineSurface->w * sizeMult);
  message->outline_rect.h = (int)(outlineSurface->h * sizeMult);
  if (message->outline_texture != NULL) SDL_DestroyTexture(message->outline_texture);
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

static void mapTextArrayToMessageArray(SDL_Renderer *renderer, const char *textArray[], Message *messageArray, Sint16 numMessages) {
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
    strncpy_s(messageArray[i].text, sizeof(messageArray[i].text), flag_text, _TRUNCATE);
    messageArray[i].text[sizeof(messageArray[i].text) - 1] = '\0';
		switch (flag_font) {
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

  int textSize_42 = 42 * GAME_HEIGHT / 1000;
  outlineSize_42 = (int)fmax(textSize_42 / 10, 3);
  Sans_42 = TTF_OpenFontRW(rw, 0, textSize_42);

  SDL_RWseek(rw, 0, RW_SEEK_SET);
  int textSize_38 = 38 * GAME_HEIGHT / 1000;
  outlineSize_38 = (int)fmax(textSize_38 / 10, 3);
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
  score_rect.y = -SCREEN_HEIGHT / 100;

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
  const char *message_array_instructions_text[] = {
		"Lo Dodge the incoming blocks!",
		malloc(TEXT_LINE_SIZE),
    malloc(TEXT_LINE_SIZE),
    malloc(TEXT_LINE_SIZE),
		"LW Two different movement types!",
		"LW Press Left/Right to toggle:",
		"Lr Type A",
		"Mr Up/Down and Left/Right movement are",
		"Mr independent, so diagonal is faster.",
		"Lb Type B",
		"Mb Speed is the same regardless of direction.",
		"Mb More analog stick-friendly."
	};
  sprintf(message_array_instructions_text[1], "MW Hold %s or %s to speed up.", btn_A, btn_B);
  sprintf(message_array_instructions_text[2], "MW Press %s to pause.", btn_Start);
  sprintf(message_array_instructions_text[3], "MW Press %s or %s to change music.", btn_L, btn_R);
  mapTextArrayToMessageArray(renderer, message_array_instructions_text, message_array_instructions, INSTRUCTIONS_LENGTH);

  setMessagePosRelativeToScreen(&message_array_instructions[0], 0.5f, 0.15f);
  setMessagePosRelativeToScreen(&message_array_instructions[1], 0.5f, 0.25f);
  setMessagePosRelativeToScreen(&message_array_instructions[2], 0.5f, 0.325f);
  setMessagePosRelativeToScreen(&message_array_instructions[3], 0.5f, 0.4f);
  setMessagePosRelativeToScreen(&message_array_instructions[4], 0.5f, 0.55f);
  setMessagePosRelativeToScreen(&message_array_instructions[5], 0.5f, 0.625f);
  setMessagePosRelativeToScreen(&message_array_instructions[6], 0.5f, 0.7f);
  setMessagePosRelativeToScreen(&message_array_instructions[7], 0.5f, 0.775f);
  setMessagePosRelativeToScreen(&message_array_instructions[8], 0.5f, 0.85f);
  setMessagePosRelativeToScreen(&message_array_instructions[9], 0.5f, 0.7f);
  setMessagePosRelativeToScreen(&message_array_instructions[10], 0.5f, 0.775f);
  setMessagePosRelativeToScreen(&message_array_instructions[11], 0.5f, 0.85f);

  // Credits Screen
  const char *message_array_credits_text[] = {
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
    "MW ???",
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
    "MW https://github.com/Mode8fx/blockamok"
  };
  mapTextArrayToMessageArray(renderer, message_array_credits_text, message_array_credits, CREDITS_LENGTH);
}

inline void drawTitleScreenText(SDL_Renderer *renderer, bool drawSecondaryText) {
	renderMessage(renderer, &message_titlescreen);
  if (drawSecondaryText) {
    renderMessage(renderer, &message_titlescreen_play);
    renderMessage(renderer, &message_titlescreen_instructions);
    renderMessage(renderer, &message_titlescreen_credits);
    renderMessage(renderer, &message_titlescreen_quit);
  }
}

inline void drawInstructionsText(SDL_Renderer *renderer) {
  renderMessage(renderer, &message_array_instructions[0]);
	renderMessage(renderer, &message_array_instructions[1]);
	renderMessage(renderer, &message_array_instructions[2]);
	renderMessage(renderer, &message_array_instructions[3]);
	renderMessage(renderer, &message_array_instructions[4]);
  renderMessage(renderer, &message_array_instructions[5]);
  if (!isAnalog) {
		renderMessage(renderer, &message_array_instructions[6]);
		renderMessage(renderer, &message_array_instructions[7]);
		renderMessage(renderer, &message_array_instructions[8]);
	} else {
    renderMessage(renderer, &message_array_instructions[9]);
    renderMessage(renderer, &message_array_instructions[10]);
    renderMessage(renderer, &message_array_instructions[11]);
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

  int numMessages = sizeof(message_array_credits) / sizeof(message_array_credits[0]);
  Uint64 timer = now - credits_startTime;
  float offset_timer = 0.15f * timer / 1000; // scroll speed
  for (int i = 0; i < numMessages; i++) {
		float offset_index = 0.06f * i; // spacing between lines
    float startPosY = 1.0f + offset_index - offset_timer;
    if (startPosY < -0.1f) {
      if (i == numMessages - 1 && startPosY < -0.5f) {
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