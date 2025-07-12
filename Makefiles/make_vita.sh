export VITASDK=/usr/local/vitasdk
rm -r build_vita
mkdir build_vita
cd build_vita
cmake ../ -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake -DVITA=ON -DFORCE_DRAW_BG=ON -DFORCE_DRAW_OVERLAY=ON
make