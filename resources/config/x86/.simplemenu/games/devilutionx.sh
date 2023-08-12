#!/bin/sh

export HOME=/userdata/system

cd /userdata/roms/devilutionx

#export SDL_GAMECONTROLLERCONFIG=$(grep "RG35XX" "${HOME}/.config/gamecontrolerdb.txt")

SDL_GAMECONTROLLERCONFIG='19000000010000000100000000010000,RG35XX Gamepad,a:b0,b:b1,x:b2,y:b3,leftshoulder:b4,rightshoulder:b5,lefttrigger:b6,righttrigger:b7,guide:b8,start:b9,back:b10,dpup:b13,dpleft:b15,dpright:b16,dpdown:b14,volumedown:b11,volumeup:b12,platform:Linux' devilutionx

