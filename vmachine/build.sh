#!/bin/bash
premake5 export-compile-commands --cc=clang gmake2 
echo "Copying compile_commands.json ..."
if [ -z $1 ]
then
    cp compile_commands/debug_linux.json compile_commands.json
    make config=debug_linux
else
    cp compile_commands/debug_linux.json compile_commands.json
    make config=$1_linux
fi