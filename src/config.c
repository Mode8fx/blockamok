#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <windows.h>
#elif defined(LINUX) || defined(GAMECUBE) || defined(THREEDS)
#include <sys/stat.h>
#endif
#if defined(WII_U)
#include "./general.h"
#endif

#include "./config.h"
#include "./draw.h"
#include "./game.h"
#include "./audio.h"
#include "./text.h"

int WINDOW_WIDTH;
int WINDOW_HEIGHT;
#define DEFAULT_SCREEN_HEIGHT (screenHeight * 0.5)
#define DEFAULT_SCREEN_WIDTH (screenWidth * 0.5)

char rootDir[256];

char saveFile[256];
char configFile[256];

#if !defined(PATH_MAX) && defined(LINUX)
#define PATH_MAX 4096
#elif !defined(PATH_MAX)
#define PATH_MAX 260
#endif

// Returns a malloc'd string with the directory path. Caller must free() it.
char* getExeDirectory(void) {
  char buffer[PATH_MAX];
#if defined(_WIN32)
  DWORD len = GetModuleFileNameA(NULL, buffer, sizeof(buffer));
  if (len == 0 || len >= sizeof(buffer)) {
    return NULL;
  }
  buffer[len] = '\0'; // Ensure null-termination
#elif defined(LINUX)
  ssize_t count = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
  if (count == -1 || count >= sizeof(buffer)) {
    return NULL;
  }
  buffer[count] = '\0';
#else
  return NULL;
#endif

  // Find the last directory separator
  char *lastSlash = strrchr(buffer, '/');
#if defined(_WIN32)
  char *lastBackslash = strrchr(buffer, '\\');
  if (!lastSlash || (lastBackslash && lastBackslash > lastSlash)) {
    lastSlash = lastBackslash;
  }
#endif

  if (lastSlash) {
    size_t dirLen = (size_t)(lastSlash - buffer + 1);
    char *dir = (char*)malloc(dirLen + 1);
    if (!dir) return NULL;
    memcpy(dir, buffer, dirLen);
    dir[dirLen] = '\0';
    return dir;
  } else {
    // No directory separator found, return empty string
    char *dir = (char*)malloc(1);
    if (dir) dir[0] = '\0';
    return dir;
  }
}

#if defined(GAMECUBE)
#include <gccore.h>
#include <dirent.h>
#include <fat.h>
#include <sdcard/gcsd.h>

static bool directoryExists(const char *path) {
  struct stat info;
  if (stat(path, &info) != 0) {
    // Path does not exist or cannot be accessed
    return false;
  }
  return (info.st_mode & S_IFDIR) != 0; // Check if it's a directory
}

#define DEV_GCSDA 1
#define DEV_GCSDB 2
#define DEV_GCSDC 3

static bool gc_initFAT(int device) {
	switch (device) {
	case DEV_GCSDA:
		__io_gcsda.startup();
		if (!__io_gcsda.isInserted()) {
			return false;
		}
		if (!fatMountSimple("sda", &__io_gcsda)) {
			return false;
		}
		break;
	case DEV_GCSDB:
		__io_gcsdb.startup();
		if (!__io_gcsdb.isInserted()) {
			return false;
		}
		if (!fatMountSimple("sdb", &__io_gcsdb)) {
			return false;
		}
		break;
	case DEV_GCSDC:
		__io_gcsd2.startup();
		if (!__io_gcsd2.isInserted()) {
			return false;
		}
		if (!fatMountSimple("sdc", &__io_gcsd2)) {
			return false;
		}
		break;
	default:
		return false;
		break;
	}

	return true;
}
#endif

void initFilePaths() {
#if defined(VITA)
  snprintf(rootDir, sizeof(rootDir), "ux0:data/BlockamokRemix/");
#elif defined(WII_U)
  WHBMountSdCard();
  WHBGetSdCardMountPath();
  const char *sdPathStart = WHBGetSdCardMountPath();
  snprintf(rootDir, sizeof(rootDir), "%s/wiiu/apps/BlockamokRemix/", sdPathStart);
#elif defined(WII)
  snprintf(rootDir, sizeof(rootDir), "sd:/apps/BlockamokRemix/");
#elif defined(GAMECUBE)
  for (int i = 1; i < 4; i++) {
    if (gc_initFAT(i)) {
      break;
    }
  }
  const char *devices[] = { "sda", "sdb", "sdc" };
  for (int i = 0; i < 3; i++) {
    snprintf(rootDir, sizeof(rootDir), "%s:/BlockamokRemix/", devices[i]);
    if (directoryExists(rootDir)) {
      break;
    }
  }
#elif defined(THREEDS)
  snprintf(rootDir, sizeof(rootDir), "sdmc:/3ds/BlockamokRemix/");
#elif defined(_WIN32)
  char *exeDir = getExeDirectory();
  if (exeDir) {
    snprintf(rootDir, sizeof(rootDir), "%s", exeDir);
    free(exeDir);
  } else {
    snprintf(rootDir, sizeof(rootDir), "./");
  }
#elif defined(LINUX)
  char *exeDir = getExeDirectory();
  if (exeDir) {
    snprintf(rootDir, sizeof(rootDir), "%s", exeDir);
    free(exeDir);
  } else {
    snprintf(rootDir, sizeof(rootDir), "./");
  }
  //const char *home = getenv("HOME");
  //if (home) {
  //  char localPath[PATH_MAX];
  //  snprintf(localPath, sizeof(localPath), "%s/.local", home);
  //  mkdir(localPath, 0755);
  //  snprintf(localPath, sizeof(localPath), "%s/.local/share", home);
  //  mkdir(localPath, 0755);
  //  snprintf(localPath, sizeof(localPath), "%s/.local/share/.BlockamokRemix", home);
  //  mkdir(localPath, 0755);
  //  snprintf(rootDir, sizeof(rootDir), "%s/.local/share/.BlockamokRemix/", home);
  //} else {
  //  // Fallback if HOME is not set
  //  snprintf(rootDir, sizeof(rootDir), "./.BlockamokRemix/");
  //  mkdir(rootDir, 0755);
  //}
#elif defined(ANDROID)
	snprintf(rootDir, sizeof(rootDir), "%s", SDL_GetPrefPath("mode8fx", "blockamokremix"));
#endif
  snprintf(saveFile, sizeof(saveFile), "%s%s", rootDir, "save.bin");
  snprintf(configFile, sizeof(configFile), "%s%s", rootDir, "config.ini");
}

void writeSaveData() {
#if defined(LINUX) || defined(VITA) || defined(THREEDS)
  mkdir(rootDir, 0777);
#endif
  FILE *file = fopen(saveFile, "wb");
  if (file != NULL) {
    fwrite(&highScoreVal, sizeof(highScoreVal), 1, file);
    fwrite(&OPTION_CUBE_FREQUENCY, sizeof(OPTION_CUBE_FREQUENCY), 1, file);
    fwrite(&OPTION_CUBE_SIZE, sizeof(OPTION_CUBE_SIZE), 1, file);
    fwrite(&OPTION_LIVES, sizeof(OPTION_LIVES), 1, file);
    fwrite(&OPTION_CONTROL_TYPE, sizeof(OPTION_CONTROL_TYPE), 1, file);
    fwrite(&OPTION_BACKGROUND_COLOR, sizeof(OPTION_BACKGROUND_COLOR), 1, file);
    fwrite(&OPTION_CUBE_COLOR, sizeof(OPTION_CUBE_COLOR), 1, file);
    fwrite(&OPTION_OVERLAY_COLOR, sizeof(OPTION_OVERLAY_COLOR), 1, file);
    fwrite(&OPTION_SPEEDOMETER, sizeof(OPTION_SPEEDOMETER), 1, file);
    fwrite(&OPTION_FULLSCREEN, sizeof(OPTION_FULLSCREEN), 1, file);
    fwrite(&OPTION_MUSIC, sizeof(OPTION_MUSIC), 1, file);
    fwrite(&OPTION_MUSIC_VOLUME, sizeof(OPTION_MUSIC_VOLUME), 1, file);
    fwrite(&OPTION_SFX_VOLUME, sizeof(OPTION_SFX_VOLUME), 1, file);
    fwrite(&OPTION_SIMPLE_CUBES, sizeof(OPTION_SIMPLE_CUBES), 1, file);
    fwrite(&OPTION_FRAME_RATE, sizeof(OPTION_FRAME_RATE), 1, file);
    fwrite(&OPTION_SPAWN_AREA, sizeof(OPTION_SPAWN_AREA), 1, file);

    Uint8 numBytesUsed = sizeof(highScoreVal)
      + sizeof(OPTION_CUBE_FREQUENCY) + sizeof(OPTION_CUBE_SIZE) + sizeof(OPTION_LIVES) + sizeof(OPTION_CONTROL_TYPE)
      + sizeof(OPTION_BACKGROUND_COLOR) + sizeof(OPTION_CUBE_COLOR) + sizeof(OPTION_OVERLAY_COLOR) + sizeof(OPTION_SPEEDOMETER) + sizeof(OPTION_FULLSCREEN)
      + sizeof(OPTION_MUSIC) + sizeof(OPTION_MUSIC_VOLUME) + sizeof(OPTION_SFX_VOLUME) + sizeof(OPTION_SIMPLE_CUBES) + sizeof(OPTION_FRAME_RATE)
      + sizeof(OPTION_SPAWN_AREA);
    Uint8 emptyBytesSize = 255 - numBytesUsed; // In case I want to add more to the save data in a future update
    if (emptyBytesSize > 0) {
      char *emptyBytes = (char *)calloc(emptyBytesSize, sizeof(char));
      fwrite(emptyBytes, emptyBytesSize, 1, file);
      free(emptyBytes); // Free the allocated memory
    }
    fclose(file);
  }
}

void readSaveData(bool skipVisualSettings) {
  FILE *file = fopen(saveFile, "rb");
  if (file != NULL) {
    fread(&highScoreVal, sizeof(highScoreVal), 1, file);
    fread(&OPTION_CUBE_FREQUENCY, sizeof(OPTION_CUBE_FREQUENCY), 1, file);
    fread(&OPTION_CUBE_SIZE, sizeof(OPTION_CUBE_SIZE), 1, file);
    fread(&OPTION_LIVES, sizeof(OPTION_LIVES), 1, file);
    fread(&OPTION_CONTROL_TYPE, sizeof(OPTION_CONTROL_TYPE), 1, file);
    if (skipVisualSettings) {
      fseek(file, sizeof(OPTION_BACKGROUND_COLOR) + sizeof(OPTION_CUBE_COLOR) +
        sizeof(OPTION_OVERLAY_COLOR) + sizeof(OPTION_SPEEDOMETER) +
        sizeof(OPTION_FULLSCREEN), SEEK_CUR);
    } else {
      fread(&OPTION_BACKGROUND_COLOR, sizeof(OPTION_BACKGROUND_COLOR), 1, file);
      fread(&OPTION_CUBE_COLOR, sizeof(OPTION_CUBE_COLOR), 1, file);
      fread(&OPTION_OVERLAY_COLOR, sizeof(OPTION_OVERLAY_COLOR), 1, file);
      fread(&OPTION_SPEEDOMETER, sizeof(OPTION_SPEEDOMETER), 1, file);
      fread(&OPTION_FULLSCREEN, sizeof(OPTION_FULLSCREEN), 1, file);
    }
    fread(&OPTION_MUSIC, sizeof(OPTION_MUSIC), 1, file);
    fread(&OPTION_MUSIC_VOLUME, sizeof(OPTION_MUSIC_VOLUME), 1, file);
    fread(&OPTION_SFX_VOLUME, sizeof(OPTION_SFX_VOLUME), 1, file);
    if (!skipVisualSettings) {
      fseek(file, sizeof(OPTION_SIMPLE_CUBES) + sizeof(OPTION_FRAME_RATE), SEEK_CUR);
    } else {
      fread(&OPTION_SIMPLE_CUBES, sizeof(OPTION_SIMPLE_CUBES), 1, file);
      fread(&OPTION_FRAME_RATE, sizeof(OPTION_FRAME_RATE), 1, file);
    }
    fread(&OPTION_SPAWN_AREA, sizeof(OPTION_SPAWN_AREA), 1, file);
    fclose(file);
  } else {
    forceIndexReset = true;
    writeSaveData();
    forceIndexReset = false;
  }
}

static void writeDefaultConfig(int screenWidth, int screenHeight) {
  FILE *file = fopen(configFile, "w");
  fprintf(file, "# Width and height must be between 240 and your screen's dimensions\n");
  fprintf(file, "WINDOW_HEIGHT=%d\n", (int)DEFAULT_SCREEN_HEIGHT);
  fprintf(file, "WINDOW_WIDTH=%d\n", (int)DEFAULT_SCREEN_WIDTH);
  fclose(file);
}

void loadConfig(int screenWidth, int screenHeight) {
#if defined(ANDROID)
	WINDOW_WIDTH = (int)fmax(screenWidth, screenHeight);
	WINDOW_HEIGHT = (int)fmin(screenWidth, screenHeight);
#elif defined(WII_U)
  WINDOW_WIDTH = 960;
  WINDOW_HEIGHT = 540;
#elif defined(LINUX) || !defined(PC)
	WINDOW_WIDTH = screenWidth;
	WINDOW_HEIGHT = screenHeight;
#else
  FILE *file = fopen(configFile, "r");
  if (file == NULL) {
    writeDefaultConfig(screenWidth, screenHeight);
    WINDOW_HEIGHT = (int)DEFAULT_SCREEN_HEIGHT;
    WINDOW_WIDTH = (int)DEFAULT_SCREEN_WIDTH;
    return;
  }

  char line[256];
  int width = 0, height = 0;
  bool validConfig = false;

  while (fgets(line, sizeof(line), file)) {
    if (strncmp(line, "WINDOW_WIDTH=", 13) == 0) {
  width = atoi(line + 13);
} else if (strncmp(line, "WINDOW_HEIGHT=", 14) == 0) {
      height = atoi(line + 14);
    }
    // Support legacy WINDOW_SIZE for backwards compatibility
 else if (strncmp(line, "WINDOW_SIZE=", 12) == 0) {
      int size = atoi(line + 12);
      if (width == 0) width = size;
    if (height == 0) height = size;
    }
  }

  fclose(file);

  if (width >= MIN_WINDOW_SIZE && width <= screenWidth &&
    height >= MIN_WINDOW_SIZE && height <= screenHeight) {
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    validConfig = true;
  }

  if (!validConfig) {
    // Invalid config, create a new one with default values
    writeDefaultConfig(screenWidth, screenHeight);
    WINDOW_HEIGHT = (int)DEFAULT_SCREEN_HEIGHT;
    WINDOW_WIDTH = (int)DEFAULT_SCREEN_WIDTH;
  }
#endif
}