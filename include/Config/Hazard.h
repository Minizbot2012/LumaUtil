#include "Config/Common.h"
#include <RE/B/BGSHazard.h>
#include <RE/H/Hazard.h>
#include <RE/T/TESObjectLIGH.h>
namespace MPL::DynaForm::Hazard {
    struct HazardData {
        using Patch = RE::BGSHazardData;
        LiteForm light;
        void Apply(Patch* itm) {
            itm->light = light.Get<RE::TESObjectLIGH>();
        }
        static HazardData From(Patch* itm) {
            return HazardData {
                .light = LiteForm::FromForm(itm->light)
            };
        }
    };
    struct Hazard {
        static constexpr std::string_view Name = "Hazard";
        using Patch = RE::BGSHazard;
        std::optional<HazardData> data;
        void Apply(Patch* itm) {
            if (data) {
                data->Apply(&itm->data);
            }
        }
        static Hazard From(Patch* itm) {
            return Hazard {
                .data = HazardData::From(&itm->data)
            };
        }

    };
}
