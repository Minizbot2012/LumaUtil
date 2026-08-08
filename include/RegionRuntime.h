#pragma once

#include <Externals/MMSF_API.h>

namespace MPL::RegionRuntime
{
    inline RE::TESRegion* HighestPriorityWeatherRegion(RE::TESObjectCELL* a_cell)
    {
        auto* regions = a_cell ? a_cell->GetRegionList(false) : nullptr;
        RE::TESRegion* selected = nullptr;
        std::uint8_t selectedPriority = 0;
        bool foundWeatherData = false;
        if (!regions)
        {
            return nullptr;
        }

        for (auto* region : *regions)
        {
            if (!region || !region->dataList) continue;
            for (auto* data : region->dataList->regionDataList)
            {
                if (!data || data->GetType() != RE::TESRegionData::Type::kWeather) continue;
                if (!foundWeatherData || data->dataHeader.priority > selectedPriority)
                {
                    selected = region;
                    selectedPriority = data->dataHeader.priority;
                    foundWeatherData = true;
                }
            }
        }
        return selected;
    }

    inline std::string EditorID(API::MMSF::Interface* a_api, RE::TESRegion* a_region)
    {
        return a_api && a_region ? a_api->LookupEDIDForFormID(a_region->formID) : std::string{};
    }

    inline RE::TESRegion* GetRegionForm(RE::TESObjectCELL* a_cell)
    {
        if (!a_cell) return nullptr;
        if (const auto* data = a_cell->extraList.GetByType<RE::ExtraCellSkyRegion>(); data && data->skyRegion)
        {
            return data->skyRegion;
        }
        if (a_cell->IsExteriorCell())
        {
            const auto* sky = RE::Sky::GetSingleton();
            if (sky && sky->region) return sky->region;
            return HighestPriorityWeatherRegion(a_cell);
        }
        return nullptr;
    }

    inline std::string GetRegion(API::MMSF::Interface* a_api, RE::TESObjectCELL* a_cell)
    {
        if (!a_api) return {};
        if (auto* region = GetRegionForm(a_cell)) return EditorID(a_api, region);
        return {};
    }
}  // namespace MPL::RegionRuntime
