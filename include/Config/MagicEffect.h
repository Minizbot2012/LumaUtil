#pragma once
#include "Config/Common.h"
#include <optional>
namespace MPL::DynaForm::MagicEffect {
    struct EffectSettingData {
        using Patch = RE::EffectSetting::EffectSettingData;
        std::optional<LiteForm> light;
        static EffectSettingData From(Patch* itm) {
            return {
                .light = LiteForm::FromForm(itm->light)
            };
        }
        void Apply(Patch* itm) {
            if(this->light) itm->light = this->light->Get<RE::TESObjectLIGH>();
        }
    };
    struct EffectSetting {
        using Patch = RE::EffectSetting;
        static constexpr std::string_view Name = "MagicEffect";
        std::optional<EffectSettingData> data;
        static EffectSetting From(Patch* itm) {
            return {
                .data = EffectSettingData::From(&itm->data)
            };
        }
        void Apply(Patch* itm) {
            if(this->data) this->data->Apply(&itm->data);
        }
    };
}
