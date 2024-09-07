cmake_minimum_required(VERSION 3.2)
project(blockamok C)

add_executable(blockamok
	"src/main.c"
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
	"src/math_custom.c"
	"src/math_custom.h"
	"src/text.c"
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

if (WIIU)
	include("${DEVKITPRO}/wut/share/wut.cmake" REQUIRED)
	add_definitions(-DWII_U)
	add_definitions(-DNINTENDO_LAYOUT)

	target_link_libraries(blockamok -L${DEVKITPRO}/portlibs/wiiu/lib -L${DEVKITPRO}/portlibs/ppc/lib -L${DEVKITPRO}/wut/lib)
	# pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET SDL2)
	# pkg_check_modules(SDL2_mixer REQUIRED IMPORTED_TARGET SDL2_mixer)
	# pkg_check_modules(SDL2_ttf REQUIRED IMPORTED_TARGET SDL2_ttf)
	# target_link_libraries(blockamok PkgConfig::SDL2 PkgConfig::SDL2_mixer PkgConfig::SDL2_ttf)
	# target_link_libraries(blockamok -lSDL2_ttf -lSDL2_mixer -lmodplug -lvorbisfile -lvorbis -logg -lSDL2 -lfreetype -lpng -lmpg123 -lbz2 -ljpeg -lz -lwut)
	PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
	PKG_SEARCH_MODULE(SDL2MIXER REQUIRED SDL2_mixer>=2.0.0)
	PKG_SEARCH_MODULE(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)
	INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS} ${SDL2MIXER_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIRS})
	TARGET_LINK_LIBRARIES(blockamok ${SDL2_LIBRARIES} ${SDL2MIXER_LIBRARIES} ${SDL2TTF_LIBRARIES})

	set(SDL2_INCLUDE_DIRS
	"${DEVKITPRO}/portlibs/wiiu/include/SDL2;\
	${DEVKITPRO}/portlibs/ppc/include;\
	${DEVKITPRO}/wut/include;\
	${DEVKITPRO}/devkitPPC/powerpc-eabi/include;"
	)
	include_directories(${SDL2_INCLUDE_DIRS})

	wut_create_rpx(blockamok)

	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/blockamok.rpx"
			DESTINATION "${CMAKE_INSTALL_PREFIX}")

elseif (VITA)
	include("${VITASDK}/share/vita.cmake" REQUIRED)
	add_definitions(-DVITA)
	add_definitions(-DPRELOAD_SOUNDS)
	
	set(VITA_APP_NAME "Blockamok")
	set(VITA_TITLEID  "BLOC52665")
	set(VITA_VERSION  "01.00")
	
	PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
	PKG_SEARCH_MODULE(SDL2MIXER REQUIRED SDL2_mixer>=2.0.0)
	PKG_SEARCH_MODULE(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)
	INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS} ${SDL2MIXER_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIRS})
	TARGET_LINK_LIBRARIES(blockamok ${SDL2_LIBRARIES} ${SDL2MIXER_LIBRARIES} ${SDL2TTF_LIBRARIES})

	# target_link_libraries(blockamok SDL2_ttf SDL2_mixer SDL2 freetype png png16 bz2 jpeg z mikmod modplug mpg123 xmp FLAC vorbisfile vorbis ogg webp pthread)
	# find_package(SDL2 REQUIRED)
	# target_link_libraries(blockamok SDL2_ttf SDL2_mixer SDL2)
	target_link_libraries(blockamok
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

	vita_create_self(blockamok.self blockamok)
	vita_create_vpk(blockamok.vpk ${VITA_TITLEID} blockamok.self
		VERSION ${VITA_VERSION}
		NAME ${VITA_APP_NAME}
		FILE release-resources/logo_icon_vita.png sce_sys/icon0.png
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