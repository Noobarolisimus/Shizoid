-- Всякие функции
function split (inputstr, sep)
    if sep == nil then
            sep = "%s"
    end
    local t={}
    for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
            table.insert(t, str);
    end
    return t
end

function removeComment(line)
    local lineEnd = line:find(";");
    if lineEnd == nil then
        lineEnd = line:len();
    else
        lineEnd = lineEnd - 1;
    end
    return line:sub(1, lineEnd);
end

-- Возвращает либо "windows", либо "unix"
function getOs()
    local sep = package.config:sub(1,1);
    if sep == "\\" then
        return "windows";
    end
    return "unix";
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

    do
        local file = io.open("data/tablesLastModifiedTime.txt", "r");
        if file == nil then
            os.execute("echo q > data/tablesLastModifiedTime.txt");
        end
        file = io.open("data/tablesLastModifiedTime.txt", "r");
        
        local old = file:read("a");
        file:close();

        if getOs() == "windows" then
            os.execute("forfiles /P .\\data /M *Table.txt /C \"cmd /c echo @file @fdate @ftime\" > data/tablesLastModifiedTime.txt");
        else
            os.execute("> data/tablesLastModifiedTime.txt;for i in $(find ./data -name '*Table.txt');do date -r $i +%d%m%y%H%M%S >> data/tablesLastModifiedTime.txt;done")
        end
        file = io.open("data/tablesLastModifiedTime.txt", "r");

        local new = file:read("a");
        if old == new then
            goto tableParsingEnd;
        end
    end

    
-- Начало парсинга таблиц
    do
        local tablescpp = io.open("src/tables.cpp", "w");
        local tablesh = io.open("src/tables.h", "w");
        local asmt = io.open("data/asmTable.txt", "r");
        local regt = io.open("data/regTable.txt", "r");
        local bytecode = 0;
        if asmt == nil then
            error("There is no \"asmTable.txt\" file.");
        end
        if regt == nil then
            error("There is no \"regTable.txt\" file.");
        end

        -- tables.cpp
        tablescpp:write("// lua generated\n#include \"tables.h\"\n");
        
        
        -- help
        local helpf = io.open("data/helpTable.txt", "r");
        tablescpp:write("\nchar const HELPTEXT[] = \"")
        for i in helpf:lines() do
            tablescpp:write(i);
            tablescpp:write("\\n");
        end
        tablescpp:write("\";\n") 
        -- ~help

        tablescpp:write("\n// { name, { bytecode, len A, len B, len ...} }\nstd::map<std::string, std::vector<uint8_t>> asmTable{\n");

        for line in asmt:lines() do
            line = removeComment(line);
            if line == "" then
                goto tablescpp_continue1;
            end

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
            ::tablescpp_continue1::
        end

        tablescpp:write("};\n\nstd::map<std::string, uint8_t> regTable{\n");

        local memBegin = 0;
        for line in regt:lines() do
            line = removeComment(line);
            if line == "" then
                goto tablescpp_continue2;
            end
            local memLen;
            local space = string.find(line, " ");
            if space == nil then
                error("There is no register length: \""..line.."\".")
            end
            memLen = tonumber(string.sub(line, space));
            line = string.sub(line, 1, space - 1);
            tablescpp:write(string.format("    { \"%s\", %i},\n", line, memBegin));
            memBegin = memBegin + memLen;
            ::tablescpp_continue2::
        end
        tablescpp:write("};");
        --tablescpp:write(string.format("};\n\nconst int REGMEMAMOUNT = %i;", memBegin));


        -- tables.h
        tablesh:write("// lua generated\n#pragma once\n#include <map>\n#include <string>\n#include <vector>\n\nextern std::map<std::string, std::vector<uint8_t>> asmTable;\nextern std::map<std::string, uint8_t> regTable;\nextern char const HELPTEXT[];\n\n// Сколько занимает регион с регистрами в Байтах\nconstexpr int REGMEMAMOUNT = "..tostring(memBegin)..";\n");
        regt:seek("set", 0);
        memBegin = 0
        for line in regt:lines() do
            line = removeComment(line);
            if line == "" then
                goto tablesh_continue1;
            end
            local space = string.find(line, " ");
            local memLen = tonumber(string.sub(line, space));
            line = string.sub(line, 1, space - 1);
            tablesh:write(string.format("#define REG_%s (*(int%i_t*)(memory + %i))\n", line, memLen * 8, memBegin));
            memBegin = memBegin + memLen;
            ::tablesh_continue1::
        end    

        asmt:close();
        tablescpp:close();
        tablesh:close()
        regt:close();
    end
-- Конец парсинга таблиц
    ::tableParsingEnd::

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
        buildoptions { "-DDIRDELIM=\\".."'\\\\\\".."'" }
        
    filter "platforms:Linux"
        defines { "_LINUX" }
        buildoptions { "-DDIRDELIM=\\".."'/\\".."'" }
