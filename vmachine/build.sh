#!/bin/bash

mode=debug

if [ $# -gt 1 ]
then
    mode="$1"
fi

if [ -f "~/.premake/export-compile-commands.lua" ]
then
    premake5 export-compile-commands --cc=clang gmake2 
    echo "Copying compile_commands.json ..."
    cp compile_commands/debug_linux.json compile_commands.json
else
    premake5 --cc=clang gmake2
fi

make "config=${mode}_linux"
