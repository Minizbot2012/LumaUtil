#pragma once
#include "Config/Common.h"
#include <optional>
namespace MPL::DynaForm::MagicEffect {
    struct EffectSettingData {
        using Patch = RE::EffectSetting::EffectSettingData;
        std::optional<LiteForm> light;
        std::optional<LiteForm> associatedForm;
        static EffectSettingData From(Patch* itm) {
            return {
                .light = LiteForm::FromForm(itm->light),
                .associatedForm = LiteForm::FromForm(itm->associatedForm),
            };
        }
        void Apply(Patch* itm) {
            if(this->light) itm->light = this->light->Get<RE::TESObjectLIGH>();
            if(this->associatedForm) itm->associatedForm = this->associatedForm->Get<RE::TESForm>();
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
