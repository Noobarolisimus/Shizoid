-- Всякие функции
    function split (inputstr, sep)
        if sep == nil then
                sep = "%s"
        end
        local t={}
        for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
                table.insert(t, str)
        end
        return t
    end
-- ~Всякие функции


workspace "Shizoid"
    configurations { "Debug", "Release" }
    platforms { "Win64", "Linux"}
    
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

    -- tables.cpp
    tablescpp:write("// lua generated\n#include \"tables.h\"\n");
    
    
    -- help
    local helpf = io.open("data/help.txt", "r");
    tablescpp:write("\nchar const HELPTEXT[] = \"")
    for i in helpf:lines() do
        tablescpp:write(i);
        tablescpp:write("\\n");
    end
    tablescpp:write("\";\n") 
    -- ~help

    tablescpp:write("\n// { name, { bytecode, len A, len B, len ...} }\nstd::map<std::string, std::vector<uint8_t>> asmTable{\n");

    for line in asmt:lines() do
        local start = 1;
        local words = split(line, " ");
        if tonumber(words[1], 10) ~= nil then
            bytecode = tonumber(words[1]);
            start = 2;
        end
        tablescpp:write(string.format("    { \"%s\", { %i", words[start], bytecode));
        for id, bytelen in pairs(words) do
            if id > start then
                tablescpp:write(", "..bytelen);
            end
        end
        tablescpp:write(" } },\n");
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
        tablescpp:write(string.format("    { \"%s\", %i},\n", line, memBegin));
        memBegin = memBegin + memLen;
    end
    tablescpp:write("};");
    --tablescpp:write(string.format("};\n\nconst int REGMEMAMOUNT = %i;", memBegin));


    -- tables.h
    tablesh:write("// lua generated\n#pragma once\n#include <map>\n#include <string>\n#include <vector>\n\nextern std::map<std::string, std::vector<uint8_t>> asmTable;\nextern std::map<std::string, uint8_t> regTable;\nextern char const HELPTEXT[];\n\n// Сколько занимает регион с регистрами в Байтах\nconstexpr int REGMEMAMOUNT = "..tostring(memBegin)..";\n");
    regt:seek("set", 0);
    memBegin = 0
    for line in regt:lines() do
        local space = string.find(line, " ");
        local memLen = tonumber(string.sub(line, space));
        line = string.sub(line, 1, space - 1);
        tablesh:write(string.format("#define REG_%s (*(int32_t*)(memory + %i))\n", line, memBegin));
        memBegin = memBegin + memLen;
    end    

    asmt:close();
    tablescpp:close();
    tablesh:close()
    regt:close();
-- Конец парсинга таблиц

    -- Ибо нехуй
    defines { "_ITERATOR_DEBUG_LEVEL=0" }

    filter "configurations:Debug"
        defines { "DEBUG", "COLORTERMINAL" }
        --buildoptions { "-v" }
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE" }
        optimize "On"
    
    filter "platforms:Win64"
        defines { "_WIN64", "_WIN" }
        
    filter "platforms:Linux"
        defines { "_LINUX" }
    
