#pragma once

#include <LumaAPI.h>

namespace MPL::LumaService
{
    void NotifyCellInitialized(RE::TESObjectCELL*);
    void NotifyReferenceInitialized(RE::TESObjectREFR*);
    void NotifyCellChanging(RE::TESObjectCELL*);
    void NotifyCellChanged(const RE::TESObjectCELL*);
    void NotifyCellPatched(RE::TESObjectCELL*, const char*, bool);
}  // namespace MPL::LumaService
