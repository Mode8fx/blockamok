<h1 align="center">Blockamok Remix</h1>

<p align="center"><b>A CPU-only, from-scratch 3D game written in C, where you dodge blocks<br>
Available for PC, Linux, Switch, Wii U, Wii, Gamecube, and Vita</b></p>

<p align="center">You accidentally fly your spaceship into a wormhole. To your shock, you find yourself in an alternate dimension filled with blocks. The throttle on your spaceship is broken and keeps increasing the speed. How far can you make it?</p>

<p align="center"><img alt="Gameplay" src="gameplay.gif"/></p>

## Background

Blockamok Remix is a significant update to [Blockamok](https://github.com/carltheperson/blockamok), a game originally made by Carl Riis to _"challenge [himself] to create a 3D game without any pre-made 3D engine or utilities"_.

This version adds many improvements including:
- Customizable gameplay settings
- Controller support + a new control scheme
- New visual settings
- Music and sound effects
- A full menu, title screen, instructions, etc.
- Scoring system polish
- High score saving
- Console ports
- Technical optimizations for weaker hardware

## How can I play it?

[Check the latest release here.](https://github.com/Mode8fx/blockamok/releases)

Keep in mind that due to hardware and library differences, this game runs better on some systems than others. If you only play one version, I would recommend the following order:

PC/Linux = Wii > Switch > Gamecube > Vita > Wii U

(Yes, the Wii version runs as well as Switch (or slightly better) and significantly better than Wii U. I don't know why.)

## Can I port it to other systems?

Go ahead! Just make sure you appropriately follow the MIT License.

This game is made using SDL2. Controller and keyboard input are handled through SDL_GameController and keyboard state respectively, and there aren't many system-specific defines. Depending on the system, you probably want to use the Linux Makefile as a base along with the `LINUX` define (and possibly `PC` as well).

## How to Compile

#### PC
1. Download the latest SDL2 development libraries (VC versions):
- [SDL2](https://github.com/libsdl-org/SDL/releases)
- [SDL2_ttf](https://github.com/libsdl-org/SDL_ttf/releases)
- [SDL2_mixer](https://github.com/libsdl-org/SDL_mixer/releases)
2. Extract the above ZIP files into a folder called SDL2, which should be located in the Blockamok directory.
3. From there, use the Visual Studio project file from the repo with Visual Studio 2022.
#### Switch
Install devkitPro and switch-portlibs (which includes SDL2 for Switch), then run `make -f Makefiles/Makefile_switch`.
#### Wii U
Install devkitPro, Wii U Toolkit, and SDL2 for Wii U, then build with CMake. I've included a `make_wii_u.sh` file for convenience once you have everything installed.
#### Wii
Install devkitPro and SDL2 for Wii, then run `make -f Makefiles/Makefile_wii`.
#### Gamecube
Install devkitPro and SDL2 for Gamecube, then run `make -f Makefiles/Makefile_gc`.
#### Vita
Install VitaSDK, then build with CMake. I've included a `make_vita.sh` file for convenience once you have everything installed.
#### PSP*
Install [the PSPDEV toolchain](https://pspdev.github.io/), which should also come with SDL2, SDL2_ttf, and SDL2_mixer (Linux or WSL is strongly recommended), then run `make -f Makefiles/Makefile_psp`.<br>
\* The PSP version is experimental and only runs in emulator.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
