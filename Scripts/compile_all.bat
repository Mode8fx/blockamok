@echo off

set "CURR_DIR=%CD%"

:: Manual sleep amounts (because `start /wait` doesn't work...)
set SLEEP_COMPILE=25
set SLEEP_COMPILE_LONG=35
set SLEEP_COMPILE_SHORT=5
set SLEEP_CLEAN=2

:: Path: repo (the folder that contains /src)
set REPO=D:/GitHub/blockamok
set REPO_DKP=/d/GitHub/blockamok
set REPO_WSL=/mnt%REPO_DKP%
set REPO_MSYS=%REPO_DKP%
set OUTPUT_DIR=%REPO%/dist/new_version/BlockamokRemix
set RELEASE_RESOURCES=%REPO%/release-resources
set MAKEFILE_DEFAULT=%REPO%/Makefile
set MAKEFILE_DEFAULT=%MAKEFILE_DEFAULT:/=\%

:: Path: devkitPro
set DEVKITPRO=C:/devkitPro/msys2/mingw64.exe

:: Path: msys2
set MSYS=C:/msys64/mingw64.exe
set MSYS_x86=C:/msys64/mingw32.exe

:: Path: Makefile directory
set MAKEFILES_DKP=%REPO_DKP%/Makefiles
set MAKEFILES_MSYS=%MAKEFILES_DKP%
set MAKEFILES_WSL=/mnt%MAKEFILES_DKP%

:: Path: 3DS CIA Utilities
set BANNERTOOL="D:/Utilities/3DS CIA Creation/bannertool"
set MAKEROM="D:/Utilities/3DS CIA Creation/makerom-v0.18.3-win_x86_64/makerom.exe"
set VERSION_3DS=272



:: Makefile: Windows x64
set MAKEFILE_MSYS_WINDOWS=%MAKEFILES_MSYS%/Makefile_pc
set OUTPUT_WINDOWS=%OUTPUT_DIR%-windows-x64/BlockamokRemix.exe

:: Makefile: Windows x86
set MAKEFILE_MSYS_WINDOWS_X86=%MAKEFILES_MSYS%/Makefile_pc_x86
set OUTPUT_WINDOWS_x86=%OUTPUT_DIR%-windows-x86/BlockamokRemix.exe

:: Makefile: Linux
set MAKEFILE_WSL_LINUX=%MAKEFILES_WSL%/Makefile_linux
set OUTPUT_LINUX=%OUTPUT_DIR%-linux/BlockamokRemix

:: Makefile: Gamecube
set MAKEFILE_DKP_GC=%MAKEFILES_DKP%/Makefile_gc
set OUTPUT_GC=%OUTPUT_DIR%-gamecube/BlockamokRemix/boot.dol

:: Makefile: Wii
set MAKEFILE_DKP_WII=%MAKEFILES_DKP%/Makefile_wii
set OUTPUT_WII=%OUTPUT_DIR%-wii/apps/BlockamokRemix/boot.dol

:: Makefile: Wii U
set MAKEFILE_DKP_WII_U=%MAKEFILES_DKP%/make_wii_u.sh
set OUTPUT_WII_U=%OUTPUT_DIR%-wiiu/wiiu/apps/BlockamokRemix/BlockamokRemix.rpx
set OUTPUT_WII_U_WUHB=%OUTPUT_DIR%-wiiu/wiiu/apps/BlockamokRemix/BlockamokRemix.wuhb

:: Makefile: Switch
set MAKEFILE_DKP_SWITCH=%MAKEFILES_DKP%/Makefile_switch
set OUTPUT_SWITCH=%OUTPUT_DIR%-switch/switch/BlockamokRemix/BlockamokRemix.nro

:: Makefile: 3DS
set MAKEFILE_DKP_3DS=%MAKEFILES_DKP%/Makefile_3ds
set OUTPUT_3DS=%OUTPUT_DIR%-3dsx/3ds/BlockamokRemix/BlockamokRemix.3dsx
set OUTPUT_3DS_CIA=%OUTPUT_DIR%-3ds.cia

:: Makefile: PSP
set MAKEFILE_WSL_PSP=%MAKEFILES_WSL%/Makefile_psp
set OUTPUT_PSP=%OUTPUT_DIR%-psp/PSP/GAME/BlockamokRemix/EBOOT.PBP

:: Makefile: Vita
set MAKEFILE_WSL_VITA=%MAKEFILES_WSL%/make_vita.sh
set OUTPUT_VITA=%OUTPUT_DIR%-vita.vpk

:: Makefile: Xbox (original)
set MAKEFILE_WSL_XBOX=%MAKEFILES_WSL%/Makefile_xbox

:: Makefile: FunKey
set MAKEFILE_WSL_FUNKEY=%MAKEFILES_WSL%/Makefile_funkey
set OUTPUT_FUNKEY=%OUTPUT_DIR%-funkey

:: Makefile: Android
set OUTPUT_ANDROID=%OUTPUT_DIR%-android.apk
set OUTPUT_ANDROID_IDSIG=%OUTPUT_ANDROID%.idsig
set OUTPUT_ANDROID_IDSIG=%OUTPUT_ANDROID_IDSIG:/=\%



:: Running compilation commands...
call :compile_windows_x64
call :compile_windows_x86
call :compile_linux
call :compile_gc
call :compile_wii
call :compile_wii_u
call :compile_switch
call :compile_vita
rem call :compile_funkey
rem call :compile_psp
call :compile_3ds
call :compile_android

echo Done.
goto :eof



:compile_windows_x64
echo Windows x64: Compiling with MSYS2 MINGW64...
start /wait "" %MSYS% /usr/bin/bash -lc "cd %REPO_MSYS%; make -f %MAKEFILE_MSYS_WINDOWS%"
sleep %SLEEP_COMPILE%
echo Windows x64: Compressing with UPX...
upx --best --lzma %REPO%/BlockamokRemix.exe
echo Windows x64: Moving compiled exe to %OUTPUT_WINDOWS%...
mv %REPO%/BlockamokRemix.exe %OUTPUT_WINDOWS%
echo Windows x64: Cleaning up...
start /wait "" %MSYS% /usr/bin/bash -lc "cd %REPO_MSYS%; make clean -f %MAKEFILE_MSYS_WINDOWS%"
sleep %SLEEP_CLEAN%
echo.
goto :eof


:compile_windows_x86
echo Windows x86: Compiling with MSYS2 MINGW32...
start /wait "" %MSYS_x86% /usr/bin/bash -lc "cd %REPO_MSYS%; make -f %MAKEFILE_MSYS_WINDOWS_X86%"
sleep %SLEEP_COMPILE%
echo Windows x86: Compressing with UPX...
upx --best --lzma %REPO%/BlockamokRemix.exe
echo Windows x86: Moving compiled exe to %OUTPUT_WINDOWS_X86%...
mv %REPO%/BlockamokRemix.exe %OUTPUT_WINDOWS_X86%
echo Windows x86: Cleaning up...
start /wait "" %MSYS_x86% /usr/bin/bash -lc "cd %REPO_MSYS%; make clean -f %MAKEFILE_MSYS_WINDOWS_X86%"
sleep %SLEEP_CLEAN%
echo.
goto :eof

:compile_linux
echo Linux: Compiling with WSL...
wsl -e sh -c "cd %REPO_WSL% && make -f %MAKEFILE_WSL_LINUX% > /dev/null 2>&1"
echo Linux: Moving compiled binary to %OUTPUT_LINUX%...
mv %REPO%/blockamokremix %OUTPUT_LINUX%
echo Linux: Cleaning up...
wsl -e sh -c "cd %REPO_WSL% && make clean -f %MAKEFILE_WSL_LINUX% > /dev/null 2>&1"
echo.
goto :eof

:compile_gc
echo Gamecube: Compiling with devkitPro...
cp %MAKEFILE_DKP_GC% %MAKEFILE_DEFAULT%
start /wait "" %DEVKITPRO% /usr/bin/bash -lc "cd %REPO_DKP%; make"
sleep %SLEEP_COMPILE%
echo Gamecube: Moving compiled dol to %OUTPUT_GC%...
mv %REPO%/boot.dol %OUTPUT_GC%
echo Gamecube: Cleaning up...
start /wait "" %DEVKITPRO% /usr/bin/bash -lc "cd %REPO_DKP%; make clean"
sleep %SLEEP_CLEAN%
del /q "%MAKEFILE_DEFAULT%"
echo.
goto :eof

:compile_wii
echo Wii: Compiling with devkitPro...
cp %MAKEFILE_DKP_WII% %MAKEFILE_DEFAULT%
start /wait "" %DEVKITPRO% /usr/bin/bash -lc "cd %REPO_DKP%; make"
sleep %SLEEP_COMPILE%
echo Wii: Moving compiled dol to %OUTPUT_WII%...
mv %REPO%/boot.dol %OUTPUT_WII%
echo Wii: Cleaning up...
start /wait "" %DEVKITPRO% /usr/bin/bash -lc "cd %REPO_DKP%; make clean"
sleep %SLEEP_CLEAN%
del /q "%MAKEFILE_DEFAULT%"
echo.
goto :eof

:compile_wii_u
echo Wii U: Compiling with devkitPro...
start /wait "" %DEVKITPRO% /usr/bin/bash -lc "cd %REPO_DKP%; sh %MAKEFILE_DKP_WII_U%"
sleep %SLEEP_COMPILE_LONG%
sleep 10
echo Wii U: Moving compiled rpx to %OUTPUT_WII_U%...
mv %REPO%/build_wii_u/BlockamokRemix.rpx %OUTPUT_WII_U%
echo Wii U WHUB: Creating WUHB in %OUTPUT_WII_U_WUHB%...
start /wait "" %DEVKITPRO% /usr/bin/bash -lc "wuhbtool %REPO%/build_wii_u_whub/BlockamokRemix.rpx %OUTPUT_WII_U_WUHB% --name=\"Blockamok Remix\" --short-name=\"Blockamok Remix\" --author=Mode8fx --icon=%RELEASE_RESOURCES%/logo_icon_128.png --tv-image=%RELEASE_RESOURCES%/splash_screen_wiiu_tv.png --drc-image=%RELEASE_RESOURCES%/splash_screen_wiiu_gamepad.png"
sleep %SLEEP_COMPILE_SHORT%
echo Wii U: Cleaning up...
rm -r %REPO%/build_wii_u
sleep %SLEEP_CLEAN%
rm -r %REPO%/build_wii_u_whub
sleep %SLEEP_CLEAN%
echo.
goto :eof

:compile_switch
echo Switch: Compiling with devkitPro...
cp %MAKEFILE_DKP_SWITCH% %MAKEFILE_DEFAULT%
start /wait "" %DEVKITPRO% /usr/bin/bash -lc "cd %REPO_DKP%; make"
sleep %SLEEP_COMPILE_LONG%
echo Switch: Moving compiled nro to %OUTPUT_SWITCH%...
mv %REPO%/blockamok.nro %OUTPUT_SWITCH%
echo Switch: Cleaning up...
start /wait "" %DEVKITPRO% /usr/bin/bash -lc "cd %REPO_DKP%; make clean"
sleep %SLEEP_CLEAN%
del /q "%MAKEFILE_DEFAULT%"
echo.
goto :eof

:compile_3ds
echo 3DS: Compiling with devkitPro...
cp %MAKEFILE_DKP_3DS% %MAKEFILE_DEFAULT%
start /wait "" %DEVKITPRO% /usr/bin/bash -lc "cd %REPO_DKP%; make"
sleep %SLEEP_COMPILE%
echo 3DS: Moving compiled 3dsx to %OUTPUT_3DS%...
mv %REPO%/blockamok.3dsx %OUTPUT_3DS%
echo 3DS: Creating CIA in %OUTPUT_3DS_CIA%...
%MAKEROM% -f cia -o %OUTPUT_3DS_CIA% -elf %REPO%/blockamok.elf -icon %RELEASE_RESOURCES%/icon_3ds.smdh -banner %RELEASE_RESOURCES%/banner_3ds.bnr -ver %VERSION_3DS% -rsf %RELEASE_RESOURCES%/app_3ds.rsf
echo 3DS: Cleaning up...
start /wait "" %DEVKITPRO% /usr/bin/bash -lc "cd %REPO_DKP%; make clean"
sleep %SLEEP_CLEAN%
del /q "%MAKEFILE_DEFAULT%"
echo.
goto :eof

:compile_vita
echo Vita: Compiling with WSL...
wsl -e sh -c "cd %REPO_WSL% && sh %MAKEFILE_WSL_VITA% > /dev/null 2>&1"
echo Vita: Moving compiled vpk to %OUTPUT_VITA%...
mv %REPO%/build_vita/blockamokremix.vpk %OUTPUT_VITA%
echo Vita: Cleaning up...
rm -r %REPO%/build_vita
echo.
goto :eof

:compile_funkey
echo FunKey: Compiling with WSL...
wsl -e sh -c "cd %REPO_WSL% && make -f %MAKEFILE_WSL_FUNKEY% > /dev/null 2>&1"
echo FunKey: Moving compiled binary to %OUTPUT_FUNKEY%...
mv %REPO%/blockamokremix %OUTPUT_FUNKEY%
echo FunKey: Cleaning up...
rm -r %REPO%/build_funkey
echo Did not attempt to make OPK. Use OpenPackageCreator for that.
echo.
goto :eof

:compile_psp
:: TODO: This doesn't work for some reason
echo PSP: Compiling with WSL...
wsl -e sh -c "cd %REPO_WSL% && make -f %MAKEFILE_WSL_PSP% > /dev/null 2>&1"
echo PSP: Moving compiled binary to %OUTPUT_PSP%...
mv %REPO%/blockamokremix %OUTPUT_PSP%
echo PSP: Cleaning up...
wsl -e sh -c "cd %REPO_WSL% && make clean -f %MAKEFILE_WSL_PSP% > /dev/null 2>&1"
echo.
goto :eof

:compile_xbox
:: (this doesn't work since ./activate breaks out of script)
rem echo Xbox: Compiling with WSL...
rem wsl -e sh -c "cd ~/nxdk/bin && ./activate && cd %REPO_WSL% && make -f %MAKEFILE_WSL_XBOX% > /dev/null 2>&1"
rem echo Xbox: Moving compiled binary to %OUTPUT_XBOX%...
rem mv %REPO%/build_xbox/blockamokremix %OUTPUT_XBOX%
rem echo.
goto :eof

:compile_android
echo Android: Compiling with Android Studio...
cd "%REPO%/Android/android-project"
rem call ndk-build clean NDK_PROJECT_PATH=app NDK_MODULE_PATH=app/jni
call ndk-build NDK_PROJECT_PATH=app NDK_MODULE_PATH=app/jni
call gradlew assembleRelease
echo Android: Signing compiled apk and moving it to %OUTPUT_ANDROID%...
echo Type keystore password and press Enter.
call apksigner sign --ks my-release-key.keystore --out %OUTPUT_ANDROID% ./app/build/outputs/apk/release/app-release-unsigned.apk
rem call adb install -r app-release-signed.apk
sleep 2
echo Android: Deleting unneeded idsig file...
del /q "%OUTPUT_ANDROID_IDSIG%"
cd /d "%CURR_DIR%"
echo.
goto :eof