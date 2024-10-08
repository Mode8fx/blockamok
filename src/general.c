#include <stdio.h>

#include "./general.h"

void systemSpecificOpen() {
#if defined(WII_U)
	/* Set SD Card Mount Path */
	WHBMountSdCard();
	WHBGetSdCardMountPath();
	char sdPathStr[256];
	const char *sdPathStart = WHBGetSdCardMountPath();
	snprintf(sdPathStr, sizeof(sdPathStr), "%s/wiiu/apps/BlockamokRemix", sdPathStart);
	const char *sdPath = sdPathStr;
	chdir(sdPath);
#elif defined(VITA)
	/* Disable rear touch pad */
	SDL_setenv("VITA_DISABLE_TOUCH_BACK", "1", 1);
#elif defined(SWITCH)
	/* Set SD Card mount path */
	chdir("/switch/BlockamokRemix");
#elif defined(WII) || defined(GAMECUBE)
	/* Initialize SD Card */
	fatInitDefault();
	/* Initialize Controller */
	PAD_Init();
#elif defined(XBOX)
	XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
#endif
}

void systemSpecificClose() {
#if defined(WII_U)
	WHBUnmountSdCard();
	WHBProcInit();
	WHBProcShutdown();
#elif defined(VITA)
	sceKernelExitProcess(0);
#elif defined(WII) || defined(GAMECUBE)
	exit(EXIT_SUCCESS);
#elif defined(PSP)
	sceKernelExitGame();
#endif
}