#pragma once
#include <Config.h>
#include <DetailedLogging.h>
#include <Externals/MMSF_API.h>
#include <RegionRuntime.h>

namespace MPL::Papyrus
{
    inline MPL::API::MMSF::Interface* GetMMSFAPI()
    {
        auto* stat = MPL::Config::StatData::GetSingleton();
        if (!stat->mmsfAPI)
        {
            stat->mmsfAPI = MPL::API::MMSF::RequestMMSFAPI();
        }
        return stat->mmsfAPI;
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
            const auto formID = api->LookupFormIDForEDID(region);
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

    inline bool Bind(RE::BSScript::IVirtualMachine* vm)
    {
        vm->RegisterFunction("GetRegion", "CLUtil", GetRegion);
        vm->RegisterFunction("ChangeRegion", "CLUtil", ChangeRegion);
        vm->RegisterFunction("RegisterForCellloadForm", "CLUtil", RegisterForOnCellLoadForm);
        vm->RegisterFunction("RegisterForCellloadRef", "CLUtil", RegisterForOnCellLoadAlias);
        vm->RegisterFunction("RegisterForCellloadMgef", "CLUtil", RegisterForOnCellLoadMgef);
        return true;
    }
}  // namespace MPL::Papyrus
