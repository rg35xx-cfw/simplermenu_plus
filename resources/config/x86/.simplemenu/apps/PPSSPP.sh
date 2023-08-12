#!/bin/sh

export SDL_GAMECONTROLLERCONFIG=$(grep "RG35XX" "${HOME}/.config/gamecontrolerdb.txt")

SDL_VSYNC=1 SDL_RENDER_DRIVER=opengles2 PPSSPP

