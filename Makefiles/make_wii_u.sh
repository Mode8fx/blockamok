export DEVKITPRO=/opt/devkitpro

rm -r build_wii_u
mkdir build_wii_u
cd build_wii_u
cmake ../ -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/wut/share/wut.toolchain.cmake -DWIIU=ON -DFORCE_DRAW_BG=ON -DFORCE_DRAW_OVERLAY=ON
make

cd ../
rm -r build_wii_u_whub
mkdir build_wii_u_whub
cd build_wii_u_whub
cmake ../ -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/wut/share/wut.toolchain.cmake -DWIIU=ON -DFORCE_DRAW_BG=ON -DFORCE_DRAW_OVERLAY=ON -DNO_QUIT=ON
make