#pragma once
#include <Config/Cell.h>
#include <Config/Common.h>
#include <Config/ImageSpace.h>
#include <Config/Lights.h>
#include <Config/ObjectRef.h>
#include <Config/Templates.h>
#include <Config/Worldspace.h>
#include <Config/MagicEffect.h>
#include <Config/Projectile.h>
#include <LumaService.h>
#include <Externals/MMSF_API.h>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>
#include <vector>

namespace MPL::Config
{
    template <typename T>
    concept Named = requires(T t) {
        { T::Name } -> std::same_as<const std::string_view&>;
    };
    template <typename T>
    concept Patch = requires(T t) {
        typename T::Patch;
        { T::From(static_cast<typename T::Patch*>(nullptr)) } -> std::same_as<T>;
        t.Apply(static_cast<typename T::Patch*>(nullptr));
    };

    class StatData : public REX::Singleton<StatData>
    {
    public:
        SKSE::RegistrationSet<const RE::TESObjectCELL*> cellLoad{ "OnCellChange"sv };
        std::unordered_map<std::filesystem::path, bool> folder_map;
        RE::TESRegion* lastRegion;
        MPL::API::MMSF::Interface* mmsfAPI = nullptr;
    };
    template <typename T>
        requires Named<T> && Patch<T>
    void LoadConfigFormID(typename T::Patch* form)
    {
        const auto* source = form ? form->GetFile(0) : nullptr;
        if (!source)
        {
            return;
        }
        auto stat = StatData::GetSingleton();
        static std::vector<std::string> valid_files = RE::TESDataHandler::GetSingleton()->files |
                                                      std::views::filter([](RE::TESFile* file)
                                                          {
            std::string sum(file->summary.c_str());
            return sum.contains("[Luma]"); }) |
                                                      std::views::transform([](RE::TESFile* file)
                                                          { return std::string(file->GetFilename()); }) |
                                                      std::ranges::to<std::vector>();
        for (const auto& local_file : valid_files)
        {
            std::filesystem::path folder_name = std::filesystem::current_path() / "Data" / "Luma" / T::Name / local_file / source->GetFilename();
            if (stat->folder_map.contains(folder_name) && !stat->folder_map[folder_name])
            {
                continue;
            }
            if (!stat->folder_map.contains(folder_name))
            {
                if (std::filesystem::exists(folder_name))
                {
                    stat->folder_map[folder_name] = true;
                }
                else
                {
                    stat->folder_map[folder_name] = false;
                    continue;
                }
            }
            auto file_name = folder_name / std::format("{:06X}.json", form->GetLocalFormID());
            if (std::filesystem::exists(file_name))
            {
                auto pch = rfl::json::load<T>(file_name.string());
                if (pch.has_value())
                {
                    pch->Apply(form);
                    if constexpr (std::same_as<T, DynaForm::Cell::TESObjectCELL>)
                    {
                        const auto validSkylight =
                            pch->skylight && pch->skylight->template Get<RE::TESRegion>() != nullptr;
                        LumaService::NotifyCellPatched(
                            form,
                            local_file.c_str(),
                            validSkylight);
                    }
                    else
                    {
                        logger::warn(
                            "Could not read Luma patch {}: {}",
                            file_name.string(),
                            pch.error().what());
                    }
                }
            }
        }
    }
}  // namespace MPL::Config
