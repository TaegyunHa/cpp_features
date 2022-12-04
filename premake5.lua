-- premake5.lua
workspace "CppFeatures"
    architecture "x64"
    configurations { "Debug", "Release" }
    startproject "CppFeatures"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
project "CppFeatures"
    kind "ConsoleApp"
    language "C++"
    cppdialect "c++20"
    targetdir "bin/%{cfg.buildcfg}"

    files { "src/**.h", "src/**.cpp" }

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "nDEBUG" }
        runtime "Release"
        optimize "On"
        symbols "On"
