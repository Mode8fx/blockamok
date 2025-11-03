#include <string.h>
#include <stdio.h>

#include "./text.h"
#include "./input.h"
#include "./draw.h"
#include "./game.h"
#include "./fonts/Mono.h"

TTF_Font *Sans_126 = NULL;
int outlineSize_126;
TTF_Font *Sans_63 = NULL;
int outlineSize_63;
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

char valStr[TEXT_LINE_SIZE];
Message message_characters_white_42[NUM_PRINTABLE_CHARS];
Message message_characters_orange_42[NUM_PRINTABLE_CHARS];
Message message_characters_gray_38[NUM_PRINTABLE_CHARS];
Message message_characters_red_38[NUM_PRINTABLE_CHARS];
Message message_characters_white_38[NUM_PRINTABLE_CHARS];
Message message_characters_blue_38[NUM_PRINTABLE_CHARS];
#define COLOR_WHITE 0
#define COLOR_ORANGE 1

Message message_titlescreen_logo_1;
Message message_titlescreen_logo_2;
Message message_game_life;
Message message_game_cursor;
Message message_gameover;
Message message_paused;
Message message_fps;
Sint8 CREDITS_LENGTH;
#define CREDITS_LENGTH_DEFAULT 50
#define CREDITS_LENGTH_COMPACT 56
char *message_array_credits_text[CREDITS_LENGTH_COMPACT];

bool compactView = false;
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
#if defined(PSP)
  SDL_Surface *outlineSurface = TTF_RenderUTF8_Blended(font, message->text, outlineColor);
#else
  SDL_Surface *outlineSurface = TTF_RenderText_Solid(font, message->text, outlineColor);
#endif
  message->outline_rect.w = (int)(outlineSurface->w * sizeMult);
  message->outline_rect.h = (int)(outlineSurface->h * sizeMult);
  message->outline_texture = SDL_CreateTextureFromSurface(renderer, outlineSurface);
  SDL_FreeSurface(outlineSurface);

  // Create the main text
  TTF_SetFontOutline(font, 0);
#if defined(PSP)
  SDL_Surface *textSurface = TTF_RenderUTF8_Blended(font, message->text, textColor);
#else
  SDL_Surface *textSurface = TTF_RenderText_Solid(font, message->text, textColor);
#endif
#if defined(WII_U)
  SDL_Surface *convertedSurface = SDL_ConvertSurfaceFormat(textSurface, SDL_PIXELFORMAT_RGBA8888, 0);
#else
  SDL_Surface *convertedSurface = SDL_ConvertSurfaceFormat(textSurface, SDL_PIXELFORMAT_RGBA5551, 0);
#endif
  SDL_FreeSurface(textSurface);
  message->text_rect.w = (int)(convertedSurface->w * sizeMult);
  message->text_rect.h = (int)(convertedSurface->h * sizeMult);
  message->text_texture = SDL_CreateTextureFromSurface(renderer, convertedSurface);
  SDL_FreeSurface(convertedSurface);
}

inline void renderMessage(SDL_Renderer *renderer, Message *message) {
  SDL_RenderCopy(renderer, message->outline_texture, NULL, &message->outline_rect);
  SDL_RenderCopy(renderer, message->text_texture, NULL, &message->text_rect);
}

///////////////////
// TEXT POSITION //
///////////////////

static inline void setMessagePosX(Message *message, int x) {
  message->text_rect.x = x;
  message->text_rect.x += gameOffsetX;
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

static inline void setMessagePosRelativeToGameX(Message *message, float x) {
	setMessagePosX(message, (int)(GAME_WIDTH * x - message->text_rect.w * 0.5f));
}

inline void setMessagePosRelativeToScreenY(Message *message, float y) {
	setMessagePosY(message, (int)(GAME_HEIGHT * y - message->text_rect.h * 0.5f));
}

inline void setMessagePosRelativeToGame(Message *message, float x, float y) {
	setMessagePosRelativeToGameX(message, x);
	setMessagePosRelativeToScreenY(message, y);
}

static inline void setMessagePosRelativeToGameX_LeftAlign(Message *message, float x) {
  setMessagePosX(message, (int)(GAME_WIDTH * x));
}

void setMessagePosRelativeToGame_LeftAlign(Message *message, float x, float y) {
  setMessagePosRelativeToGameX_LeftAlign(message, x);
  setMessagePosRelativeToScreenY(message, y);
}

static inline void setMessagePosRelativeToGameX_RightAlign(Message *message, float x) {
  setMessagePosX(message, (int)(GAME_WIDTH * x - message->text_rect.w));
}

static inline void setMessagePosRelativeToGame_RightAlign(Message *message, float x, float y) {
  setMessagePosRelativeToGameX_RightAlign(message, x);
  setMessagePosRelativeToScreenY(message, y);
}

static inline void drawTextFromChars(SDL_Renderer *renderer, float relX, float relY) {
  Message *message_characters;

  // Initialize string if empty
  if (valStr[0] == '\0') {
    snprintf(valStr, 5, "    ");
  }
  Uint8 numChars = (Uint8)strlen(valStr) - 4;
  // First two characters set font size and color
  if (strncmp(valStr, "LW", 2) == 0) {
    message_characters = message_characters_white_42;
  } else if (strncmp(valStr, "Lo", 2) == 0) {
    message_characters = message_characters_orange_42;
  } else if (strncmp(valStr, "MG", 2) == 0) {
    message_characters = message_characters_gray_38;
  } else if (strncmp(valStr, "Mr", 2) == 0) {
    message_characters = message_characters_red_38;
  } else if (strncmp(valStr, "MW", 2) == 0) {
    message_characters = message_characters_white_38;
  } else if (strncmp(valStr, "Mb", 2) == 0) {
    message_characters = message_characters_blue_38;
  } else {
    message_characters = message_characters_white_42;
  }
  // Third character sets alignment
  int currX;
  switch (valStr[2]) {
  case 'L':
    currX = (int)(relX * GAME_WIDTH + gameOffsetX);
    break;
  case 'C':
    currX = (int)(relX * GAME_WIDTH + gameOffsetX - (numChars * message_characters[0].text_rect.w) / 2);
    break;
  default: // 'R'
    currX = (int)(relX * GAME_WIDTH + gameOffsetX - numChars * message_characters[0].text_rect.w);
    break;
  }

  // The fourth character is a delimiter (it does nothing), while the actual string starts at the fifth character
  // Render all outlines, then all text
  const char *flag_text = &valStr[4];
  for (int i = 0; i < numChars; i++) {
    Uint8 chr = flag_text[i] - FIRST_PRINTABLE_CHAR;
    message_characters[chr].text_rect.x = currX + i * message_characters[chr].text_rect.w;
    message_characters[chr].outline_rect.x = message_characters[chr].text_rect.x - (message_characters[chr].outline_rect.w - message_characters[chr].text_rect.w) / 2;
    setMessagePosRelativeToScreenY(&message_characters[chr], relY);
    SDL_RenderCopy(renderer, message_characters[chr].outline_texture, NULL, &message_characters[chr].outline_rect);
  }
  for (int i = 0; i < numChars; i++) {
    Uint8 chr = flag_text[i] - FIRST_PRINTABLE_CHAR;
    message_characters[chr].text_rect.x = currX + i * message_characters[chr].text_rect.w;
    SDL_RenderCopy(renderer, message_characters[chr].text_texture, NULL, &message_characters[chr].text_rect);
  }
}

///////////////////
// GAME-SPECIFIC //
///////////////////

static void initStaticMessages_Characters() {
  for (Uint8 i = 0; i < NUM_PRINTABLE_CHARS; i++) {
    Uint8 character = i + FIRST_PRINTABLE_CHAR;
    snprintf(message_characters_white_42[i].text, 2, "%c", character);
    prepareMessage(renderer, Sans_42, outlineSize_42, &message_characters_white_42[i], 1, color_white, color_black);
    snprintf(message_characters_orange_42[i].text, 2, "%c", character);
    prepareMessage(renderer, Sans_42, outlineSize_42, &message_characters_orange_42[i], 1, color_orange, color_black);
    snprintf(message_characters_gray_38[i].text, 2, "%c", character);
    prepareMessage(renderer, Sans_38, outlineSize_38, &message_characters_gray_38[i], 1, color_gray, color_black);
    snprintf(message_characters_red_38[i].text, 2, "%c", character);
    prepareMessage(renderer, Sans_38, outlineSize_38, &message_characters_red_38[i], 1, color_red, color_black);
    snprintf(message_characters_white_38[i].text, 2, "%c", character);
    prepareMessage(renderer, Sans_38, outlineSize_38, &message_characters_white_38[i], 1, color_white, color_black);
    snprintf(message_characters_blue_38[i].text, 2, "%c", character);
    prepareMessage(renderer, Sans_38, outlineSize_38, &message_characters_blue_38[i], 1, color_blue, color_black);
  }
}

static void initStaticMessages_TitleScreen() {
  snprintf(message_titlescreen_logo_1.text, MAX_MESSAGE_SIZE, "Blockamok");
  prepareMessage(renderer, Sans_126, outlineSize_126, &message_titlescreen_logo_1, 1, color_white, color_black);
  setMessagePosRelativeToGame(&message_titlescreen_logo_1, 0.5f, 0.4f);

  snprintf(message_titlescreen_logo_2.text, MAX_MESSAGE_SIZE, "Remix");
  prepareMessage(renderer, Sans_63, outlineSize_63, &message_titlescreen_logo_2, 1, color_black, color_white);
  setMessagePosRelativeToGame(&message_titlescreen_logo_2, 0.5f, 0.525f);
}

static void initStaticMessages_Game() {
  snprintf(message_game_cursor.text, MAX_MESSAGE_SIZE, "+");
  prepareMessage(renderer, Sans_63, outlineSize_63, &message_game_cursor, 1, color_white, color_black);
  setMessagePosRelativeToGame(&message_game_cursor, 0.5f, 0.5f);
  SDL_SetTextureAlphaMod(message_game_cursor.outline_texture, 64);
  SDL_SetTextureAlphaMod(message_game_cursor.text_texture, 64);

  snprintf(message_game_life.text, MAX_MESSAGE_SIZE, ".");
  prepareMessage(renderer, Sans_126, outlineSize_126, &message_game_life, 1, color_red, color_blue);
  setMessagePosRelativeToScreenY(&message_game_life, -0.01f);

  snprintf(message_gameover.text, MAX_MESSAGE_SIZE, "GAME OVER");
  prepareMessage(renderer, Sans_126, outlineSize_126, &message_gameover, 1, color_white, color_black);
  setMessagePosRelativeToGame(&message_gameover, 0.5f, 0.5f);

  snprintf(message_paused.text, MAX_MESSAGE_SIZE, "PAUSED");
  prepareMessage(renderer, Sans_126, outlineSize_126, &message_paused, 1, color_white, color_black);
  setMessagePosRelativeToGame(&message_paused, 0.5f, 0.5f);
}

static void initStaticMessages_Credits() {
  if (!compactView) {
    message_array_credits_text[0] = "LoC BLOCKAMOK REMIX v1.2";
    message_array_credits_text[1] = "";
    message_array_credits_text[2] = "MrC Carl Riis";
    message_array_credits_text[3] = "MrC Original game";
    message_array_credits_text[4] = "MWC https://github.com/carltheperson/blockamok";
    message_array_credits_text[5] = "";
    message_array_credits_text[6] = "MbC Mode8fx";
    message_array_credits_text[7] = "MbC \"Remix\" edition and ports";
    message_array_credits_text[8] = "MWC https://github.com/Mode8fx/blockamok";
    message_array_credits_text[9] = "";
    message_array_credits_text[10] = "LoC MUSIC";
    message_array_credits_text[11] = "";
    message_array_credits_text[12] = "MGC Raina ft. Coaxcable";
    message_array_credits_text[13] = "MWC \"Spaceranger 50k\"";
    message_array_credits_text[14] = "";
    message_array_credits_text[15] = "MGC Cobburn and Monty";
    message_array_credits_text[16] = "MWC \"Falling Up\"";
    message_array_credits_text[17] = "";
    message_array_credits_text[18] = "MGC Diomatic";
    message_array_credits_text[19] = "MWC \"Falling People\"";
    message_array_credits_text[20] = "";
    message_array_credits_text[21] = "MGC mano and ske";
    message_array_credits_text[22] = "MWC \"Darkness in da Night\"";
    message_array_credits_text[23] = "";
    message_array_credits_text[24] = "MGC Diablo";
    message_array_credits_text[25] = "MWC \"Dance 2 Insanity\"";
    message_array_credits_text[26] = "";
    message_array_credits_text[27] = "MGC All music obtained from modarchive.org";
    message_array_credits_text[28] = "";
    message_array_credits_text[29] = "MGC \"Spaceranger 50k\" and \"Falling People\"";
    message_array_credits_text[30] = "MGC edited by Mode8fx for looping purposes";
    message_array_credits_text[31] = "";
    message_array_credits_text[32] = "LoC SOUND EFFECTS";
    message_array_credits_text[33] = "";
    message_array_credits_text[34] = "MGC claudeb";
    message_array_credits_text[35] = "MWC https://opengameart.org";
    message_array_credits_text[36] = "MWC /content/buzz-grid-sounds";
    message_array_credits_text[37] = "";
    message_array_credits_text[38] = "LoC LIBRARIES";
    message_array_credits_text[39] = "";
    message_array_credits_text[40] = "MWC SDL2";
    message_array_credits_text[41] = "MWC SDL2_mixer";
    message_array_credits_text[42] = "MWC SDL2_ttf";
    message_array_credits_text[43] = "";
    message_array_credits_text[44] = "LoC THANKS FOR PLAYING!";
    message_array_credits_text[45] = "";
    message_array_credits_text[46] = "MGC Blockamok Remix is available on a wide";
    message_array_credits_text[47] = "MGC variety of homebrew-enabled systems,";
    message_array_credits_text[48] = "MGC old and new. Play it everywhere!";
    message_array_credits_text[49] = "MWC https://github.com/Mode8fx/blockamok";
    CREDITS_LENGTH = CREDITS_LENGTH_DEFAULT;
  }
  else {
    message_array_credits_text[0] = "LoC BLOCKAMOK REMIX";
    message_array_credits_text[1] = "";
    message_array_credits_text[2] = "MrC Carl Riis";
    message_array_credits_text[3] = "MrC Original game";
    message_array_credits_text[4] = "MWC https://github.com";
    message_array_credits_text[5] = "MWC /carltheperson/blockamok";
    message_array_credits_text[6] = "";
    message_array_credits_text[7] = "MbC Mode8fx";
    message_array_credits_text[8] = "MbC \"Remix\" update and ports";
    message_array_credits_text[9] = "MWC https://github.com";
    message_array_credits_text[10] = "MWC /Mode8fx/blockamok";
    message_array_credits_text[11] = "";
    message_array_credits_text[12] = "LoC MUSIC";
    message_array_credits_text[13] = "";
    message_array_credits_text[14] = "MGC Raina ft. Coaxcable";
    message_array_credits_text[15] = "MWC \"Spaceranger 50k\"";
    message_array_credits_text[16] = "";
    message_array_credits_text[17] = "MGC Cobburn and Monty";
    message_array_credits_text[18] = "MWC \"Falling Up\"";
    message_array_credits_text[19] = "";
    message_array_credits_text[20] = "MGC Diomatic";
    message_array_credits_text[21] = "MWC \"Falling People\"";
    message_array_credits_text[22] = "";
    message_array_credits_text[23] = "MGC mano and ske";
    message_array_credits_text[24] = "MWC \"Darkness in da Night\"";
    message_array_credits_text[25] = "";
    message_array_credits_text[26] = "MGC Diablo";
    message_array_credits_text[27] = "MWC \"Dance 2 Insanity\"";
    message_array_credits_text[28] = "";
    message_array_credits_text[29] = "MGC All music obtained from";
    message_array_credits_text[30] = "MGC modarchive.org";
    message_array_credits_text[31] = "";
    message_array_credits_text[32] = "MGC \"Spaceranger 50k\" and";
    message_array_credits_text[33] = "MGC \"Falling People\" edited by";
    message_array_credits_text[34] = "MGC Mode8fx for looping purposes";
    message_array_credits_text[35] = "";
    message_array_credits_text[36] = "LoC SOUND EFFECTS";
    message_array_credits_text[37] = "";
    message_array_credits_text[38] = "MGC claudeb";
    message_array_credits_text[39] = "MWC https://opengameart.org";
    message_array_credits_text[40] = "MWC /content/buzz-grid-sounds";
    message_array_credits_text[41] = "";
    message_array_credits_text[42] = "LoC LIBRARIES";
    message_array_credits_text[43] = "";
    message_array_credits_text[44] = "MWC SDL2";
    message_array_credits_text[45] = "MWC SDL2_mixer";
    message_array_credits_text[46] = "MWC SDL2_ttf";
    message_array_credits_text[47] = "";
    message_array_credits_text[48] = "LoC THANKS FOR PLAYING!";
    message_array_credits_text[49] = "";
    message_array_credits_text[50] = "MGC Blockamok Remix is available on a";
    message_array_credits_text[51] = "MGC wide variety of homebrew-enabled";
    message_array_credits_text[52] = "MGC systems, old and new.";
    message_array_credits_text[53] = "MGC Play it everywhere!";
    message_array_credits_text[54] = "MWC https://github.com";
    message_array_credits_text[55] = "MWC /Mode8fx/blockamok";
    CREDITS_LENGTH = CREDITS_LENGTH_COMPACT;
  }
}

#if defined(PSP)
#define MIN_SIZE_38 12
#else
#define MIN_SIZE_38 10
#endif

void initStaticMessages(SDL_Renderer *renderer) {
  cleanUpText();
  compactView = GAME_HEIGHT <= 289;

  SDL_RWops *rw = SDL_RWFromMem(Mono_ttf, Mono_ttf_len);

  int textSize_126 = (int)fmax(126 * GAME_HEIGHT / 1000, 36);
  outlineSize_126 = (int)fmax(textSize_126 / 10, 3);
  Sans_126 = TTF_OpenFontRW(rw, 0, textSize_126);

  int textSize_63 = (int)fmax(63 * GAME_HEIGHT / 1000, 18);
  outlineSize_63 = (int)fmax(textSize_63 / 10, 3);
  Sans_63 = TTF_OpenFontRW(rw, 0, textSize_63);

  SDL_RWseek(rw, 0, RW_SEEK_SET);
  int textSize_42 = (int)fmax(42 * GAME_HEIGHT / 1000, 12);
  outlineSize_42 = (int)fmax(textSize_42 / 10, 3);
  Sans_42 = TTF_OpenFontRW(rw, 0, textSize_42);

  SDL_RWseek(rw, 0, RW_SEEK_SET);
  int textSize_38 = (int)fmax(38 * GAME_HEIGHT / 1000, MIN_SIZE_38);
  outlineSize_38 = (int)fmax(textSize_38 / 10, 3);
  if (compactView) {
    outlineSize_38 = 2;
  }
  Sans_38 = TTF_OpenFontRW(rw, 1, textSize_38);

  initStaticMessages_Characters();
  initStaticMessages_TitleScreen();
  initStaticMessages_Game();
  initStaticMessages_Options(renderer);
  initStaticMessages_Credits();

  destroyFont(Sans_126); // no longer needed
  destroyFont(Sans_63); // no longer needed
}

void drawTitleScreenText(SDL_Renderer *renderer, bool drawSecondaryText) {
  renderMessage(renderer, &message_titlescreen_logo_1);
  renderMessage(renderer, &message_titlescreen_logo_2);
  if (drawSecondaryText) {
    snprintf(valStr, TEXT_LINE_SIZE, "LWC Press %s to fly", btn_Start);
    drawTextFromChars(renderer, 0.5f, 0.65f);
    snprintf(valStr, TEXT_LINE_SIZE, "LWC Press %s for options", btn_Select);
    drawTextFromChars(renderer, 0.5f, 0.75f);
    snprintf(valStr, TEXT_LINE_SIZE, "LoC High Score: %d", highScoreVal);
    drawTextFromChars(renderer, 0.5f, 0.9f);
  }
}

void drawGameText(SDL_Renderer *renderer) {
  snprintf(valStr, TEXT_LINE_SIZE, "LWC %d", (int)scoreVal);
  drawTextFromChars(renderer, 0.5f, 0.03f);
  if (debugMode) {
    usedDebugMode = true;
    snprintf(valStr, TEXT_LINE_SIZE, "LWC %.1f %d", cubeBoundsBase, cubeAmount);
    drawTextFromChars(renderer, 0.5f, 0.90f);
  }

  Uint32 invinceTimer = now - invinceStart;
  if (invinceTimer > INVINCE_TIME || invinceTimer / INVINCE_BLINK_TIME % 2 == 1 || gameStart == invinceStart) {
    for (int i = 0; i < numLives; i++) {
      setMessagePosRelativeToGameX_LeftAlign(&message_game_life, 0.8f + 0.06f * i);
      renderMessage(renderer, &message_game_life);
    }
  }

  if (OPTION_SPEEDOMETER) {
    float printedSpeed = playerSpeed * (speedingUp ? SPEED_UP_MULT : 1);
    if (printedSpeed < (speedingUp ? TRUE_MAX_SPEED_INT : MAX_SPEED_INT)) {
      snprintf(valStr, TEXT_LINE_SIZE, "LWR %d MPH", (int)printedSpeed);
      drawTextFromChars(renderer, 0.95f, 0.95f);
    } else {
      snprintf(valStr, TEXT_LINE_SIZE, "LoR %d MPH", (int)printedSpeed);
      drawTextFromChars(renderer, 0.95f, 0.95f);
    }
  }
}

void drawInstructionsText(SDL_Renderer *renderer) {
  snprintf(valStr, TEXT_LINE_SIZE, "LoC Dodge the incoming blocks!");
  drawTextFromChars(renderer, 0.5f, 0.15f);
  snprintf(valStr, TEXT_LINE_SIZE, "MGC Hold %s or %s to speed up.", btn_A, btn_B);
  drawTextFromChars(renderer, 0.5f, 0.225f);
#if defined(PSP)
  snprintf(valStr, TEXT_LINE_SIZE, "MGC %s or %s to toggle cursor.", btn_X, btn_Y);
#else
  snprintf(valStr, TEXT_LINE_SIZE, "MGC Press %s or %s to toggle cursor.", btn_X, btn_Y);
#endif
  drawTextFromChars(renderer, 0.5f, 0.3f);
  snprintf(valStr, TEXT_LINE_SIZE, "MGC Press %s to pause.", btn_Start);
  drawTextFromChars(renderer, 0.5f, 0.375f);
  snprintf(valStr, TEXT_LINE_SIZE, "MGC Press %s or %s to change music.", btn_L, btn_R);
  drawTextFromChars(renderer, 0.5f, 0.45f);
  snprintf(valStr, TEXT_LINE_SIZE, "LoC Check the Options menu");
  drawTextFromChars(renderer, 0.5f, 0.6f);
  snprintf(valStr, TEXT_LINE_SIZE, "LoC to customize your game!");
  drawTextFromChars(renderer, 0.5f, 0.675f);
}

void drawCreditsText(SDL_Renderer *renderer, Uint32 now) {
  if (keyPressed(INPUT_A)) {
		credits_paused = !credits_paused;
  }
  if (keyHeld(INPUT_UP)) {
    credits_startTime += (3 * deltaTime);
  } else if (keyHeld(INPUT_DOWN)) {
    credits_startTime -= deltaTime;
	} else if (credits_paused) {
    credits_startTime += deltaTime;
  }

  Uint32 timer = now - credits_startTime;
  float offset_timer = 0.15f * timer / 1000; // scroll speed
  if (compactView) {
    offset_timer *= 1.1f;
  }
  for (int i = 0; i < CREDITS_LENGTH; i++) {
		float offset_index = 0.06f * i; // spacing between lines
    float startPosY = 1.03f + offset_index - offset_timer;
    if (startPosY < -0.1f) {
      if (i == CREDITS_LENGTH - 1 && startPosY < -0.5f) {
				credits_startTime = now; // loop credits
      }
      continue;
		} else if (startPosY > 1.2f) {
			break;
		}
    snprintf(valStr, TEXT_LINE_SIZE, "%s", message_array_credits_text[i]);
    drawTextFromChars(renderer, 0.5f, startPosY);
  }
}

void drawResetHighScoreText(SDL_Renderer *renderer) {
  snprintf(valStr, TEXT_LINE_SIZE, "LWC Are you sure you want to");
  drawTextFromChars(renderer, 0.5f, 0.35f);
  snprintf(valStr, TEXT_LINE_SIZE, "LWC reset your high score?");
  drawTextFromChars(renderer, 0.5f, 0.425f);
  snprintf(valStr, TEXT_LINE_SIZE, "LWC If so, press");
  drawTextFromChars(renderer, 0.5f, 0.575f);
  if (!compactView) {
    snprintf(valStr, TEXT_LINE_SIZE, "MrC Up Down Left Right Up Down Left Right");
    drawTextFromChars(renderer, 0.5f, 0.65f);
  } else {
    snprintf(valStr, TEXT_LINE_SIZE, "MrC Up Down Left Right");
    drawTextFromChars(renderer, 0.5f, 0.65f);
    //snprintf(valStr, TEXT_LINE_SIZE, "Mr Up Down Left Right");
    drawTextFromChars(renderer, 0.5f, 0.725f);
  }
}

void drawQuitText(SDL_Renderer *renderer) {
#if defined(SWITCH) || defined(WII_U)
  snprintf(valStr, TEXT_LINE_SIZE, "LWC Quit to homebrew menu?");
  drawTextFromChars(renderer, 0.5f, 0.45f);
#elif defined(WII)
  snprintf(valStr, TEXT_LINE_SIZE, "LWC Quit to Homebrew Channel?");
  drawTextFromChars(renderer, 0.5f, 0.45f);
#else
  snprintf(valStr, TEXT_LINE_SIZE, "LWC Are you sure you want to quit?");
  drawTextFromChars(renderer, 0.5f, 0.45f);
#endif
  snprintf(valStr, TEXT_LINE_SIZE, "MrC Press %s to quit", btn_A);
  drawTextFromChars(renderer, 0.5f, 0.55f);
}

void drawCursor(SDL_Renderer *renderer) {
  if (showCursor) {
    renderMessage(renderer, &message_game_cursor);
  }
}

void drawGameOverText(SDL_Renderer *renderer) {
  renderMessage(renderer, &message_gameover);
  if (newHighScore) {
    if (usedDebugMode) {
      snprintf(valStr, TEXT_LINE_SIZE, "LoC Now try it without debug mode!");
      drawTextFromChars(renderer, 0.5f, 0.75f);
    } else {
      snprintf(valStr, TEXT_LINE_SIZE, "LoC New High Score!");
      drawTextFromChars(renderer, 0.5f, 0.75f);
    }
  }
}

void drawPausedText(SDL_Renderer *renderer) {
  renderMessage(renderer, &message_paused);
  snprintf(valStr, TEXT_LINE_SIZE, "LWC Press %s to quit", btn_Select);
  drawTextFromChars(renderer, 0.5f, 0.65f);
}

void cleanUpText() {
  for (int i = 0; i < NUM_PRINTABLE_CHARS; i++) {
    destroyMessage(&message_characters_white_42[i]);
    destroyMessage(&message_characters_orange_42[i]);
    destroyMessage(&message_characters_gray_38[i]);
    destroyMessage(&message_characters_red_38[i]);
    destroyMessage(&message_characters_white_38[i]);
    destroyMessage(&message_characters_blue_38[i]);
  }
  destroyMessage(&message_titlescreen_logo_1);
  destroyMessage(&message_titlescreen_logo_2);
  destroyMessage(&message_game_life);
  destroyMessage(&message_game_cursor);
  destroyMessage(&message_gameover);
  destroyMessage(&message_paused);

  destroyFont(Sans_42);
  destroyFont(Sans_38);
}

Uint64 fps_lastTime = 0;
double fps_freq;
Uint8 fps_frameCount = 0;
Uint8 fps_frameCountLastSecond = 0;

void printFPS() {
  fps_frameCount++;
  Uint64 currentTime = SDL_GetPerformanceCounter();
  if (fps_lastTime == 0) {
    fps_lastTime = currentTime;
    fps_freq = (double)SDL_GetPerformanceFrequency();
  }

  double elapsed = (double)(currentTime - fps_lastTime) / fps_freq;
  if (elapsed >= 1.0) {
    fps_frameCountLastSecond = fps_frameCount;
    fps_frameCount = 0;
    fps_lastTime = currentTime;
  }

  int hundreds = (int)(fps_frameCountLastSecond) / 100;
  int tens = ((int)(fps_frameCountLastSecond) / 10) % 10;
  int ones = (int)(fps_frameCountLastSecond) % 10;
  snprintf(valStr, TEXT_LINE_SIZE, "LWC %d%d%d", hundreds, tens, ones);
  drawTextFromChars(renderer, 0.5f, 0.95f);
}
