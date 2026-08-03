#pragma once
#include <Config/Common.h>
namespace MPL::DynaForm::Worldspace
{
    struct TESWorldSpace
    {
        static constexpr std::string_view Name = "Worldspace";
        std::optional<DynaForm::LiteForm> climate;
        std::optional<DynaForm::LiteForm> lightTemplate;
        using Patch = RE::TESWorldSpace;
        static TESWorldSpace From(Patch* itm)
        {
            return TESWorldSpace{
                .climate = itm->climate ? std::optional<LiteForm>(LiteForm{ itm->climate->formID }) : std::nullopt,
                .lightTemplate = itm->lightingTemplate ? std::optional<LiteForm>(LiteForm{ itm->lightingTemplate->formID }) : std::nullopt,
            };
        }
        void Apply(Patch* itm)
        {
            if (climate)
            {
                itm->climate = this->climate->Get<RE::TESClimate>();
            }
            if (lightTemplate)
            {
                itm->lightingTemplate = lightTemplate->Get<RE::BGSLightingTemplate>();
            }
        }
    };
}  // namespace MPL::Config::Worldspace
