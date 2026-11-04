#pragma once
#include <Config.h>
#include <DetailedLogging.h>
#include <Externals/MMSF_API.h>
#include <RegionRuntime.h>

namespace MPL::Papyrus
{
    inline MPL::API::MMSF::IEDIDCache* GetMMSFAPI()
    {
        auto* stat = MPL::Config::StatData::GetSingleton();
        if (!stat->mmsfAPI)
        {
            stat->mmsfAPI = MPL::API::MMSF::RequestMMSFAPI();
        }
        if(!API::MMSF::HasFeature(stat->mmsfAPI->GetVersion(), API::MMSF::MMSFAPIFeatures::kCoreService))
        {
            logger::error("Core service not available");
            stl::report_and_fail("Core service not available");
            return nullptr;
        }
        if(!API::MMSF::HasFeature(stat->mmsfAPI->GetVersion(), API::MMSF::MMSFAPIFeatures::kCaching))
        {
            logger::error("EDID Cache not available");
            stl::report_and_fail("EDID Cache not available");
            return nullptr;
        }
        if(!stat->mmsfEDID)
        {
            stat->mmsfEDID = stat->mmsfAPI ? static_cast<MPL::API::MMSF::IEDIDCache*>(stat->mmsfAPI->QueryService("EDID")) : nullptr;
        }
        return stat->mmsfEDID;
    }

    inline std::string GetRegion(RE::StaticFunctionTag*, RE::TESObjectCELL* cl)
    {
        return RegionRuntime::GetRegion(GetMMSFAPI(), cl);
    }

    inline void ChangeRegion(RE::StaticFunctionTag*, RE::TESObjectCELL* cl, std::string region)
    {
        if (!cl || region.empty())
        {
            return;
        }
        auto* api = GetMMSFAPI();
        auto* form = api ? api->LookupCachedForm(region) : nullptr;
        if (!form && api)
        {
            const auto formID = api->LookupEdid(region);
            form = formID ? RE::TESForm::LookupByID(formID) : nullptr;
        }
        auto* skyRegion = form ? form->As<RE::TESRegion>() : nullptr;
        if (!skyRegion)
        {
            logger::warn("Sky region for {} is unavailable", region);
            return;
        }

        if (auto* data = cl->extraList.GetByType<RE::ExtraCellSkyRegion>())
        {
            data->skyRegion = skyRegion;
            if (const auto* source = skyRegion->GetFile(0))
            {
                DetailedLogging::Info(
                    "{:X}:{}",
                    skyRegion->GetLocalFormID(),
                    source->GetFilename());
            }
            return;
        }
        auto* data = RE::BSExtraData::Create<RE::ExtraCellSkyRegion>();
        data->skyRegion = skyRegion;
        if (const auto* source = skyRegion->GetFile(0))
        {
            DetailedLogging::Info(
                "Adding sky region {:X}:{}",
                skyRegion->GetLocalFormID(),
                source->GetFilename());
        }
        cl->extraList.Add(data);
    }

    inline void RegisterForOnCellLoadForm(RE::StaticFunctionTag*, RE::TESForm* listener)
    {
        MPL::Config::StatData::GetSingleton()->cellLoad.Register(listener);
    }

    inline void RegisterForOnCellLoadAlias(RE::StaticFunctionTag*, RE::BGSRefAlias* listener)
    {
        MPL::Config::StatData::GetSingleton()->cellLoad.Register(listener);
    }

    inline void RegisterForOnCellLoadMgef(RE::StaticFunctionTag*, RE::ActiveEffect* listener)
    {
        MPL::Config::StatData::GetSingleton()->cellLoad.Register(listener);
    }

    //REGION: EmittanceUtil

    // Re-runs the engine's per-region weather update, exactly like a natural
    // Weather.SetActive / weather transition, and then pushes the result into
    // the reference's loaded 3D.
    //
    // Each emittance-driven reference stores its source in ExtraEmittanceSource.
    // When that source is a Region (e.g. a custom "all weathers" emittance
    // region), the engine keeps a cached emittanceColor on the region that it
    // refreshes during a weather transition tick via TESRegion::SetCurrentWeather.
    // Forced/instant weather changes skip that tick, leaving emittances stale.
    // Calling SetCurrentWeather(currentWeather) here reproduces the engine's own
    // update; UpdateRefLight() then re-reads it into the ref's 3D.
    //
    // Emittance sources that are NOT regions (e.g. a linked light) are left
    // untouched — they do not follow the weather.
    static bool RefreshEmittanceRef(RE::TESObjectREFR* a_ref, RE::Sky* a_sky,
        std::unordered_set<RE::TESRegion*>& a_updatedRegions)
    {
        if (!a_ref || !a_ref->Is3DLoaded() || !a_sky || !a_sky->currentWeather)
        {
            return false;
        }
        auto* extra = a_ref->extraList.GetByType<RE::ExtraEmittanceSource>();
        if (!extra || !extra->source || !extra->source->Is(RE::FormType::Region))
        {
            return false;
        }
        auto* region = static_cast<RE::TESRegion*>(extra->source);
        // Run the engine's weather update once per region, then refresh the 3D.
        if (a_updatedRegions.insert(region).second)
        {
            region->SetCurrentWeather(a_sky->currentWeather);
        }
        a_ref->UpdateRefLight();
        return true;
    }

    // Fan the current weather's emittance out to every loaded reference,
    // replicating the per-ref update that a natural weather transition
    // performs across the whole loaded area (not just a single cell).
    inline std::uint32_t RefreshLoadedEmittances(RE::Sky* a_sky)
    {
        auto* tes = RE::TES::GetSingleton();
        if (!tes || !a_sky || !a_sky->currentWeather)
        {
            return 0;
        }
        std::unordered_set<RE::TESRegion*> updatedRegions;
        std::uint32_t refreshed = 0;
        tes->ForEachReference([&](RE::TESObjectREFR* a_ref)
            {
            if (RefreshEmittanceRef(a_ref, a_sky, updatedRegions)) {
                ++refreshed;
            }
            return RE::BSContainer::ForEachResult::kContinue; });
        return refreshed;
    }

    // "Instant SetActive": applies the weather instantly like ForceActive
    // (Sky::ForceWeather), then performs the emittance fan-out that ForceActive
    // skips but SetActive does over its transition. The net effect is a weather
    // change that is instant AND leaves no stale emittances behind.
    inline void SetWeatherInstant(RE::StaticFunctionTag*, RE::TESWeather* a_weather, bool a_override)
    {
        auto* sky = RE::Sky::GetSingleton();
        if (!sky || !a_weather)
        {
            return;
        }
        sky->ForceWeather(a_weather, a_override);
        auto refreshed = RefreshLoadedEmittances(sky);
        logger::info("SetWeatherInstant: weather {:08X} override={} emittancesRefreshed={}",
            a_weather->GetFormID(), a_override, refreshed);
    }

    //END REGION: EMITTANCEUTIL


    inline bool Bind(RE::BSScript::IVirtualMachine* vm)
    {
        vm->RegisterFunction("GetRegion", "CLUtil", GetRegion);
        vm->RegisterFunction("ChangeRegion", "CLUtil", ChangeRegion);
        vm->RegisterFunction("RegisterForCellloadForm", "CLUtil", RegisterForOnCellLoadForm);
        vm->RegisterFunction("RegisterForCellloadRef", "CLUtil", RegisterForOnCellLoadAlias);
        vm->RegisterFunction("RegisterForCellloadMgef", "CLUtil", RegisterForOnCellLoadMgef);

        vm->RegisterFunction("SetWeatherInstant", "EmittanceUtil", SetWeatherInstant);
        return true;
    }
}  // namespace MPL::Papyrus
