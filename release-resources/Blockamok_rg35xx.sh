export PORTS_FOLDER=$(realpath $(dirname $0))
export GAMEDIR="${PORTS_FOLDER}/BlockamokRemix"
export HOME="${GAMEDIR}"
export SDL_ASSERT=always_ignore
export SDL_GAMECONTROLLERCONFIG=$(grep "Deeplay" "${GAMEDIR}/gamecontrollerdb.txt")

chmod +x $GAMEDIR/blockamokremix

cd $GAMEDIR
./blockamokremix
