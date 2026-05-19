export PS2DEV=/usr/local/ps2dev
rm -r build_ps2
mkdir build_ps2
cd build_ps2
cmake ../ -DCMAKE_TOOLCHAIN_FILE=$PS2DEV/share/ps2dev.cmake -DPS2=ON -DLOW_SPEC_BG=ON
make