#!/bin/sh
watchexec -w $1 -- "lua sc.lua $1 > ../scene.json"
