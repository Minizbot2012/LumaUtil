#include <Config/Common.h>
#include <RE/T/TESObjectLIGH.h>
namespace MPL::DynaForm::Explosion {
    struct BGSExplosionData {
        using Patch = RE::BGSExplosionData;
        std::optional<LiteForm> light;
        static BGSExplosionData From(Patch* itm) {
            return {
                .light = LiteForm::FromForm(itm->light),
            };
        }
        void Apply(Patch* itm) {
            if(this->light) itm->light = this->light->Get<RE::TESObjectLIGH>();
        }
    };
    struct BGSExplosion {
        using Patch = RE::BGSExplosion;
        static constexpr std::string_view Name = "Explosion";
        std::optional<BGSExplosionData> data;
        static BGSExplosion From(Patch* itm) {
            return {
                .data = BGSExplosionData::From(&itm->data),
            };
        }
        void Apply(Patch* itm) {
            if(this->data) this->data->Apply(&itm->data);
        }
    };
}
