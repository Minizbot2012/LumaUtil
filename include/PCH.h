#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#define _UNICODE
// clang-format off
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <REX/REX.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <windows.h>
#include <string>
#include <format>
#include <rfl/json.hpp>
// clang-format on
namespace logger = SKSE::log;
using namespace REX;
using namespace std::literals;
#define DLLEXPORT __declspec(dllexport)
#include "Hooking.h"
