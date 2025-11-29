
<h1 align="center">Blockamok Remix</h1>

<p align="center"><b>A CPU-only, from-scratch 3D game written in C, where you dodge blocks<br>
Available for PC, Linux, Android, Switch, Wii U, Wii, GameCube, 3DS, PSP, and Vita</b></p>

<p align="center">You accidentally fly your spaceship into a wormhole. To your shock, you find yourself in an alternate dimension filled with blocks. The throttle on your spaceship is broken and keeps increasing the speed. How far can you make it?</p>

<p align="center"><img alt="Gameplay" src="gameplay.gif"/></p>

## Background

Blockamok Remix is a significant update to [Blockamok](https://github.com/carltheperson/blockamok), a game originally made by Carl Riis to _"challenge [himself] to create a 3D game without any pre-made 3D engine or utilities"_.

This version adds many improvements including:
- Widescreen
- Customizable gameplay settings
- Controller support + a new control scheme
- New visual settings
- Music and sound effects
- A full menu, title screen, instructions, etc.
- Scoring system polish
- High score saving
- Console ports
- Performance improvements for weaker hardware [(see performance metrics comparing Remix v1.0 to v1.1 here)](https://github.com/Mode8fx/blockamok/blob/main/Console%20Performance%20Metrics%20v1.0%20to%20v1.1.png)

## How can I play it?

[Check the latest release here.](https://github.com/Mode8fx/blockamok/releases)

Keep in mind that due to hardware and library differences, this game runs better on some systems than others, and it's not necessarily determined by the power of the system. For example, the GameCube and Wii versions both run better than Wii U.

## Can I port it to other systems?

Go ahead! Just make sure you appropriately follow the MIT License.

This game is made using SDL2. Controller and keyboard input are handled through SDL_GameController and keyboard state respectively, and there aren't many system-specific defines. Depending on the system, you probably want to use the Linux Makefile as a base along with the `LINUX` define (and possibly `PC` as well).

The following additional defines exist and may be needed:
- `FORCE_DRAW_OVERLAY`: This makes the game redraw the overlay (colored side bars) on every frame. Enable this if the overlay has a visual issue such as flickering, or if the thin black bars on the inside edge of the overlay do not render.
- `FORCE_DRAW_BG`: This makes the game redraw the transparent background triangles on every frame instead of drawing them once and saving+rendering as a texture. Enabling this may increase or decrease performance depending on the device.
- `LOW_SPEC_BG` - Enable this to replace the background with a flat color. This looks the worst visually, so only use it if the other background options result in poor performance. Do not use `FORCE_DRAW_BG` and `LOW_SPEC_BG` together.

## How to Compile

#### PC (Visual Studio)
1. Download the latest SDL2 development libraries (VC versions):
- [SDL2](https://github.com/libsdl-org/SDL/releases)
- [SDL2_ttf](https://github.com/libsdl-org/SDL_ttf/releases)
- [SDL2_mixer](https://github.com/libsdl-org/SDL_mixer/releases)
2. Extract the above ZIP files into a folder called SDL2, which should be located in the Blockamok directory.
#### PC (MSYS2)
Install MSYS2 and the latest SDL2 development libraries (along with mixer and TTF), open MSYS2 MINGW64 (64-bit) or MSYS2 MINGW32 (32-bit), and run `make -f Makefiles/Makefile_pc` (or `make -f Makefiles/Makefile_pc_x86` for 32-bit).
3. From there, use the Visual Studio project file from the repo with Visual Studio 2022.
#### Switch
Install devkitPro and switch-portlibs (which includes SDL2 for Switch), then run `make -f Makefiles/Makefile_switch`.
#### Wii U
Install devkitPro, Wii U Toolkit, and SDL2 for Wii U, then build with CMake. I've included a `make_wii_u.sh` file for convenience once you have everything installed.
#### Wii
Install devkitPro and SDL2 for Wii, then run `make -f Makefiles/Makefile_wii`.
#### GameCube
Install devkitPro and SDL2 for GameCube, then run `make -f Makefiles/Makefile_gc`.
#### 3DS
Install devkitPro and SDL2 for 3DS, then run `make -f Makefiles/Makefile_3ds`.
#### Vita
Install VitaSDK, then build with CMake. I've included a `make_vita.sh` file for convenience once you have everything installed.
#### PSP
Install [the PSPDEV toolchain](https://pspdev.github.io/), which should also come with SDL2, SDL2_ttf, and SDL2_mixer (Linux or WSL is strongly recommended), then run `make -f Makefiles/Makefile_psp`.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
