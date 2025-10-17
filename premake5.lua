-- Variables for directories
local targetDir = "bin/%{cfg.buildcfg}/%{cfg.platform}/"
local objDir    = "obj/%{cfg.buildcfg}/%{cfg.platform}/"

workspace "DAG"
    architecture "x64"
    configurations { "Debug", "Release" }
    startproject "DAG"

project "DAG"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir(targetDir)
    objdir(objDir)

    -- Project include directories
    includedirs {
        "%{wks.location}/include",   -- your own headers
        -- add more here if needed, e.g. external libraries:
        -- "%{wks.location}/thirdparty/somelib/include"
    }

    files {
        "src/**.cpp",
        "include/**.h"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        symbols "On"
        runtime "Debug"

    filter "configurations:Release"
        optimize "On"
        runtime "Release"