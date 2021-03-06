
local getcxxflags = premake.gcc.getcxxflags;
function premake.gcc.getcxxflags(cfg)
    local cxxflags = { Cxx0x = "-std=c++0x" }
    local r = getcxxflags(cfg);
    local r2 = table.translate(cfg.flags, cxxflags);
    for _,v in ipairs(r2) do table.insert(r, v) end
    return r;
end
table.insert(premake.fields.flags.allowed, "Cxx0x");

solution "Circa"
    configurations { "Debug", "Release" }
    language "C++"
    flags { "Symbols", "Cxx0x", "NoRTTI", "NoExceptions" }
    targetdir "build"
    objdir "build/obj"
    includedirs { "include", "src", "3rdparty" }

    configuration "Release"
        flags { "OptimizeSpeed" }

    configuration "Debug"
        defines { "DEBUG" }

    project "library"
        kind "StaticLib"

        targetname "circa"
        location "src"
        files {
            "src/*.cpp",
            "src/ext/read_tar.cpp",
            "src/ext/perlin.cpp",
            "src/generated/stdlib_script_text.cpp",
            "3rdparty/tinymt/tinymt64.cc"
            }

        configuration "Debug"
            targetname "circa_d"

    project "command_line"
        kind "ConsoleApp"
        targetname "circa"
        location "src"
        defines { "CIRCA_USE_LINENOISE" }
        files {
            "src/command_line/command_line.cpp",
            "src/command_line/command_line_main.cpp",
            "3rdparty/linenoise/linenoise.c",
        }
        links {"library","dl"}

        configuration "Debug"
            targetname "circa_d"

    --[[
    project "unit_tests"
        kind "ConsoleApp"
        targetname "circa_test"
        location "src"
        files {"src/unit_tests/*.cpp"}
        links {"static_lib","dl"}

        configuration "Release"
            targetname "circa_test_r"
            ]]--
