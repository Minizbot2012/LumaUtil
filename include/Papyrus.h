#pragma once
#include <Config.h>
#include <Externals/MMSF_API.h>
#include <WeatherPatcher.h>
namespace MPL::Papyrus
{
    inline std::string GetRegion(RE::StaticFunctionTag*, RE::TESObjectCELL* cl)
    {
        if (cl != nullptr)
        {
            auto sta = MPL::Config::StatData::GetSingleton();
            if (cl->extraList.HasType<RE::ExtraCellSkyRegion>())
            {
                auto dat = cl->extraList.GetByType<RE::ExtraCellSkyRegion>();
                if (dat->skyRegion != nullptr)
                {
#ifndef NDEBUG
                    logger::info("{:X}:{}", dat->skyRegion->GetLocalFormID(), dat->skyRegion->sourceFiles.array->front()->GetFilename());
#endif
                    return sta->MMSF->LookupEDIDForFormID(dat->skyRegion->formID);
                }
            }
            if (cl->IsExteriorCell())
            {
                auto sky = RE::Sky::GetSingleton();
                if (sky && sky->region)
                {
#ifndef NDEBUG
                    logger::info("{:X}:{}", sky->region->GetLocalFormID(), sky->region->sourceFiles.array->front()->GetFilename());
#endif
                    sta->lastRegion = sky->region;
                    return sta->MMSF->LookupEDIDForFormID(sky->region->formID);
                }
                else if (sta->lastRegion)
                {
                    return sta->MMSF->LookupEDIDForFormID(sta->lastRegion->formID);
                }
            }
        }
        return "";
    }

    inline void ChangeRegion(RE::StaticFunctionTag*, RE::TESObjectCELL* cl, std::string region)
    {
        if (cl != nullptr && !region.empty())
        {
            auto sta = MPL::Config::StatData::GetSingleton();
            if (cl->extraList.HasType<RE::ExtraCellSkyRegion>())
            {
                auto dat = cl->extraList.GetByType<RE::ExtraCellSkyRegion>();
                auto skr = sta->MMSF->LookupCachedForm(region)->As<RE::TESRegion>();
                if (skr != nullptr)
                {
                    dat->skyRegion = skr;
#ifndef NDEBUG
                    logger::info("{:X}:{}", dat->skyRegion->GetLocalFormID(), dat->skyRegion->sourceFiles.array->front()->GetFilename());
#endif
                }
                else
                {
                    logger::warn("Sky region for {} IS nullptr", region);
                }
            }
            else
            {
                auto skr = sta->MMSF->LookupCachedForm(region)->As<RE::TESRegion>();
                if (skr != nullptr)
                {
                    auto dat = RE::BSExtraData::Create<RE::ExtraCellSkyRegion>();
                    dat->skyRegion = skr;
                    logger::info("Adding sky region {:X}:{}", dat->skyRegion->GetLocalFormID(), dat->skyRegion->sourceFiles.array->front()->GetFilename());
                    cl->extraList.Add(dat);
                }
                else
                {
                    logger::warn("Sky region for {} IS nullptr", region);
                }
            }
        }
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
        //BEGIN REGION: Emittance Util VM REGISTER
        vm->RegisterFunction("SetWeatherInstant", "EmittanceUtil", SetWeatherInstant);
        //END REGION: Emittance Util VM REGISTER
        return true;
    }

    //REGION: Weather Patcher VM REGISTER
    inline float GetAmbientMultiplier(RE::StaticFunctionTag*, std::string a_pluginName)
	{
		return WeatherPatcher::GetAmbientMultiplier(a_pluginName);
	}

	inline void SetAmbientMultiplier(RE::StaticFunctionTag*, std::string a_pluginName, float a_value)
	{
		WeatherPatcher::SetAmbientMultiplier(a_pluginName, a_value);
	}

	inline float GetSunlightMultiplier(RE::StaticFunctionTag*, std::string a_pluginName)
	{
		return WeatherPatcher::GetSunlightMultiplier(a_pluginName);
	}

	inline void SetSunlightMultiplier(RE::StaticFunctionTag*, std::string a_pluginName, float a_value)
	{
		WeatherPatcher::SetSunlightMultiplier(a_pluginName, a_value);
	}

	inline float GetAmbientCompression(RE::StaticFunctionTag*, std::string a_pluginName)
	{
		return WeatherPatcher::GetAmbientCompression(a_pluginName);
	}

	inline void SetAmbientCompression(RE::StaticFunctionTag*, std::string a_pluginName, float a_value)
	{
		WeatherPatcher::SetAmbientCompression(a_pluginName, a_value);
	}

	inline float GetSunlightCompression(RE::StaticFunctionTag*, std::string a_pluginName)
	{
		return WeatherPatcher::GetSunlightCompression(a_pluginName);
	}

	inline void SetSunlightCompression(RE::StaticFunctionTag*, std::string a_pluginName, float a_value)
	{
		WeatherPatcher::SetSunlightCompression(a_pluginName, a_value);
	}

	inline float GetMoonlightSaturationMultiplier(RE::StaticFunctionTag*, std::string a_pluginName)
	{
		return WeatherPatcher::GetMoonlightSaturationMultiplier(a_pluginName);
	}

	inline void SetMoonlightSaturationMultiplier(RE::StaticFunctionTag*, std::string a_pluginName, float a_value)
	{
		WeatherPatcher::SetMoonlightSaturationMultiplier(a_pluginName, a_value);
	}

	inline void ApplySettings(RE::StaticFunctionTag*, std::string a_pluginName)
	{
		WeatherPatcher::ApplySettings(a_pluginName);
	}

	inline void SaveSettings(RE::StaticFunctionTag*, std::string a_pluginName)
	{
		WeatherPatcher::SaveSettings(a_pluginName);
	}

	inline bool ResetSettingsToDefault(RE::StaticFunctionTag*, std::string a_pluginName)
	{
		return WeatherPatcher::ResetSettingsToDefault(a_pluginName);
	}

	inline std::vector<std::string> GetPresets(RE::StaticFunctionTag*, std::string a_pluginName, int a_type)
	{
		return WeatherPatcher::GetPresets(a_pluginName, WeatherPatcher::IntToPresetType(a_type));
	}

	inline void LoadPreset(RE::StaticFunctionTag*, std::string a_pluginName, int a_type, std::string a_presetName)
	{
		WeatherPatcher::LoadPreset(a_pluginName, WeatherPatcher::IntToPresetType(a_type), a_presetName);
	}

	inline bool BindWeatherPatcher(RE::BSScript::IVirtualMachine* a_vm)
	{
		a_vm->RegisterFunction("GetAmbientMultiplier", "WeatherUtil", GetAmbientMultiplier);
		a_vm->RegisterFunction("SetAmbientMultiplier", "WeatherUtil", SetAmbientMultiplier);
		a_vm->RegisterFunction("GetSunlightMultiplier", "WeatherUtil", GetSunlightMultiplier);
		a_vm->RegisterFunction("SetSunlightMultiplier", "WeatherUtil", SetSunlightMultiplier);
		a_vm->RegisterFunction("GetAmbientCompression", "WeatherUtil", GetAmbientCompression);
		a_vm->RegisterFunction("SetAmbientCompression", "WeatherUtil", SetAmbientCompression);
		a_vm->RegisterFunction("GetSunlightCompression", "WeatherUtil", GetSunlightCompression);
		a_vm->RegisterFunction("SetSunlightCompression", "WeatherUtil", SetSunlightCompression);
		a_vm->RegisterFunction("GetMoonlightSaturationMultiplier", "WeatherUtil", GetMoonlightSaturationMultiplier);
		a_vm->RegisterFunction("SetMoonlightSaturationMultiplier", "WeatherUtil", SetMoonlightSaturationMultiplier);
		a_vm->RegisterFunction("ApplySettings", "WeatherUtil", ApplySettings);
		a_vm->RegisterFunction("SaveSettings", "WeatherUtil", SaveSettings);
		a_vm->RegisterFunction("ResetSettingsToDefault", "WeatherUtil", ResetSettingsToDefault);
		a_vm->RegisterFunction("GetPresets", "WeatherUtil", GetPresets);
		a_vm->RegisterFunction("LoadPreset", "WeatherUtil", LoadPreset);
		return true;
	}
	// END REGION: Weather Patcher VM REGISTER
}  // namespace MPL::Papyrus
