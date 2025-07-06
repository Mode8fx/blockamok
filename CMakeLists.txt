cmake_minimum_required(VERSION 3.5)
project(blockamokremix C)

add_executable(blockamokremix
	"src/main.c"
	"src/audio.c"
	"src/audio.h"
	"src/config.c"
	"src/config.h"
	"src/draw.c"
	"src/draw.h"
	"src/game.c"
	"src/game.h"
	"src/game_init.c"
	"src/game_init.h"
	"src/general.c"
	"src/general.h"
	"src/input.c"
	"src/input.h"
	"src/text.c"
	"src/text_options.c"
	"src/text.h"
	"src/audio/falling_people.c"
	"src/audio/falling_people.h"
	"src/audio/falling_up.c"
	"src/audio/falling_up.h"
	"src/audio/insanity.c"
	"src/audio/insanity.h"
	"src/audio/mano_-_darkness_in_the_night.c"
	"src/audio/mano_-_darkness_in_the_night.h"
	"src/audio/spaceranger_50k.c"
	"src/audio/spaceranger_50k.h"
	"src/audio/ding2_short.c"
	"src/audio/ding2_short.h"
	"src/audio/ding2_short_reversed.c"
	"src/audio/ding2_short_reversed.h"
	"src/audio/thunk.c"
	"src/audio/thunk.h"
	"src/audio/zoom3_short.c"
	"src/audio/zoom3_short.h"
	"src/fonts/Mono.c"
	"src/fonts/Mono.h"
)
find_package(PkgConfig REQUIRED)

if (NO_QUIT)
	add_definitions(-DNO_QUIT)
endif()

if (WIIU)
	include("${DEVKITPRO}/wut/share/wut.cmake" REQUIRED)
	add_definitions(-DWII_U)

	target_link_libraries(blockamokremix -L${DEVKITPRO}/portlibs/wiiu/lib -L${DEVKITPRO}/portlibs/ppc/lib -L${DEVKITPRO}/wut/lib)
	PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
	PKG_SEARCH_MODULE(SDL2MIXER REQUIRED SDL2_mixer>=2.0.0)
	PKG_SEARCH_MODULE(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)
	INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS} ${SDL2MIXER_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIRS})
	TARGET_LINK_LIBRARIES(blockamokremix ${SDL2_LIBRARIES} ${SDL2MIXER_LIBRARIES} ${SDL2TTF_LIBRARIES})

	set(SDL2_INCLUDE_DIRS
	"${DEVKITPRO}/portlibs/wiiu/include/SDL2;\
	${DEVKITPRO}/portlibs/ppc/include;\
	${DEVKITPRO}/wut/include;\
	${DEVKITPRO}/devkitPPC/powerpc-eabi/include;"
	)
	include_directories(${SDL2_INCLUDE_DIRS})

	wut_create_rpx(blockamokremix)

	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/blockamokremix.rpx"
			DESTINATION "${CMAKE_INSTALL_PREFIX}")

elseif (VITA)
	include("${VITASDK}/share/vita.cmake" REQUIRED)
	add_definitions(-DVITA)
	
	set(VITA_APP_NAME "Blockamok Remix")
	set(VITA_TITLEID  "BLOC52665")
	set(VITA_VERSION  "01.00")
	
	PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
	PKG_SEARCH_MODULE(SDL2MIXER REQUIRED SDL2_mixer>=2.0.0)
	PKG_SEARCH_MODULE(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)
	INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS} ${SDL2MIXER_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIRS})
	TARGET_LINK_LIBRARIES(blockamokremix ${SDL2_LIBRARIES} ${SDL2MIXER_LIBRARIES} ${SDL2TTF_LIBRARIES})

	target_link_libraries(blockamokremix
		SceDisplay_stub
		SceCtrl_stub
		SceAudio_stub
		SceSysmodule_stub
		SceGxm_stub
		SceCommonDialog_stub
		SceAppMgr_stub
		SceTouch_stub
		SceMotion_stub
		SceHid_stub
		m
	)

	include_directories("${VITASDK}/arm-vita-eabi/include;${VITASDK}/arm-vita-eabi/include/SDL2;")

	vita_create_self(blockamokremix.self blockamokremix)
	vita_create_vpk(blockamokremix.vpk ${VITA_TITLEID} blockamokremix.self
		VERSION ${VITA_VERSION}
		NAME ${VITA_APP_NAME}
		FILE release-resources/logo_icon_128.png sce_sys/icon0.png
		FILE release-resources/background_vita.png sce_sys/livearea/contents/bg.png
		FILE release-resources/logo_startup_vita.png sce_sys/livearea/contents/startup.png
		FILE release-resources/template_vita.xml sce_sys/livearea/contents/template.xml
	)
elseif (RG35XX)
	add_definitions(-DRG35XX)
	PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
	PKG_SEARCH_MODULE(SDL2MIXER REQUIRED SDL2_mixer>=2.0.0)
	PKG_SEARCH_MODULE(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)
	INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS} ${SDL2MIXER_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIRS})
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${SDL2_LIBRARIES} ${SDL2MIXER_LIBRARIES} ${SDL2TTF_LIBRARIES})

else()
	if (LINUX)
		add_definitions(-DLINUX)
	endif()
	PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
	PKG_SEARCH_MODULE(SDL2MIXER REQUIRED SDL2_mixer>=2.0.0)
	PKG_SEARCH_MODULE(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)
	INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS} ${SDL2MIXER_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIRS})
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${SDL2_LIBRARIES} ${SDL2MIXER_LIBRARIES} ${SDL2TTF_LIBRARIES})

endif()

unset(WIIU CACHE)
unset(VITA CACHE)
unset(LINUX CACHE)
unset(RG35XX CACHE)