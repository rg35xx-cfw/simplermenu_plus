#!/bin/sh

export SDL_GAMECONTROLLERCONFIG=$(grep "RG35XX" "${HOME}/.config/gamecontrolerdb.txt")

flycast "$1"

