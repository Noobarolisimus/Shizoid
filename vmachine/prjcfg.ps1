premake5 --cc=clang gmake2
premake5 export-compile-commands
echo "Copying compile_commands.json ..."
copy compile_commands\debug_win64.json compile_commands.json