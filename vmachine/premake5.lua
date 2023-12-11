workspace "Shizoid"
    configurations { "Debug", "Release" }
    
project "VMachine"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetdir "bin"

    files { "**.h", "**.cpp" }
    
    -- filter "system:windows"
    --     os.execute("copy data/asmTable.txt bin/asmTable.txt")
    -- 
    -- filter "system:linux"
    --     os.execute("cp data/asmTable.txt bin/asmTable.txt")

-- Начало парсинга таблиц
    local tablescpp = io.open("src/tables.cpp", "w");
    local tablesh = io.open("src/tables.h", "w");
    local asmt = io.open("data/asmTable.txt", "r");
    local regt = io.open("data/regTable.txt", "r");
    local bytecode = 0;
    if asmt == nil then
        error("There is no \"asmTable.txt\" file.");
    end
    if regt == nil then
        error("There is no \"regtTable.txt\" file.");
    end

    -- tablescpp
    tablescpp:write("//lua generated\n#include \"tables.h\"\n\nstd::map<std::string, uint8_t> asmTable{\n");
    for line in asmt:lines() do
        local space = string.find(line, " ");
        if space ~= nil then
            bytecode = tonumber(string.sub(line, space))
            line = string.sub(line, 1, space - 1);
        end
        tablescpp:write(string.format("    {\"%s\", %i},\n", line, bytecode));
        bytecode = bytecode + 1;
    end

    tablescpp:write("};\n\nstd::map<std::string, uint8_t> regTable{\n");

    local memBegin = 0;
    for line in regt:lines() do
        local memLen;
        local space = string.find(line, " ");
        if space == nil then
            error("There is no register length: \""..line.."\".")
        end
        memLen = tonumber(string.sub(line, space));
        line = string.sub(line, 1, space - 1);
        tablescpp:write(string.format("    {\"%s\", %i},\n", line, memBegin));
        memBegin = memBegin + memLen;
    end
    tablescpp:write("};");
    --tablescpp:write(string.format("};\n\nconst int REGMEMAMOUNT = %i;", memBegin));

    -- tablesh
    tablesh:write("// lua generated\n#include <map>\n#include <string>\n\nextern std::map<std::string, uint8_t> asmTable;\nextern std::map<std::string, uint8_t> regTable;\n\n// Сколько занимает регион с регистрами в Байтах\nconstexpr int REGMEMAMOUNT = "..tostring(memBegin)..";\n");
    regt:seek("set", 0);
    memBegin = 0
    for line in regt:lines() do
        local space = string.find(line, " ");
        local memLen = tonumber(string.sub(line, space));
        line = string.sub(line, 1, space - 1);
        tablesh:write(string.format("#define REG_%s (*(int32_t*)(memory + %i))\n", line, memBegin));
        memBegin = memBegin + memLen;
    end
    
    -- help
    local helpf = io.open("data/help.txt", "r");
    tablesh:write("\nchar const HELPTEXT[] = \"")
    for i in helpf:lines() do
        tablesh:write(i);
        tablesh:write("\\n");
    end
    tablesh:write("\";\n") 
    -- ~help

    asmt:close();
    tablescpp:close();
    tablesh:close()
    regt:close();
-- Конец парсинга таблиц


    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"