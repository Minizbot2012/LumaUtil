add_rules("mode.debug", "mode.release")
--[[
local xwin_dir = os.getenv("WINSYSROOT")
if xwin_dir then
    set_toolchains("clang-cl[llvm]")
    add_cxxflags("/winsysroot " .. xwin_dir, { force = true })
    add_ldflags("/winsysroot " .. xwin_dir, { force = true })
end
]]
includes("extern/CommonLibSSE")
add_requires("reflect-cpp")

target("LumaUtil")
set_kind("shared")
set_symbols("debug")
set_pcxxheader("include/PCH.h")
set_languages("c++23")
set_encodings("utf-8")
add_files("src/**.cpp")
if is_plat("windows") and is_config("toolchain", "msvc") then
    add_cxflags("/permissive-", "/Zc:__cplusplus", "/Zc:preprocessor")
end
add_includedirs("include")
set_configdir("$(builddir)/include")
add_includedirs("$(builddir)/include")
set_configvar("PROJECT_NAME", "XEMIUtil")
set_configvar("PROJECT_VERSION_MAJOR", 1)
set_configvar("PROJECT_VERSION_MINOR", 5)
set_configvar("PROJECT_VERSION_PATCH", 0)
add_configfiles("res/Plugin.h.in", { filename = "Plugin.h" })
add_deps("commonlibsse-ng")
add_packages("reflect-cpp")
