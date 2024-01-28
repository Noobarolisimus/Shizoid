premake5 export-compile-commands --cc=clang gmake2 
echo "Copying compile_commands.json ..."
copy compile_commands\debug_win64.json compile_commands.json