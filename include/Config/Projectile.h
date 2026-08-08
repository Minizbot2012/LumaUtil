#pragma once
#include "Config/Common.h"
#include <optional>
namespace MPL::DynaForm::Projectile {
    struct BGSProjectileData {
        using Patch = RE::BGSProjectileData;
        std::optional<LiteForm> light;
        std::optional<LiteForm> muzzleFlashLight;
        static BGSProjectileData From(Patch* itm) {
            return {
                .light = LiteForm::FromForm(itm->light),
                .muzzleFlashLight = LiteForm::FromForm(itm->muzzleFlashLight)
            };
        }
        void Apply(Patch* itm) {
            if(this->light) itm->light = light->Get<RE::TESObjectLIGH>();
            if(this->muzzleFlashLight) itm->muzzleFlashLight = this->muzzleFlashLight->Get<RE::TESObjectLIGH>();
        }
    };
    struct BGSProjectile {
        using Patch = RE::BGSProjectile;
        static constexpr std::string_view Name = "Projectile";
        std::optional<BGSProjectileData> data;
        static BGSProjectile From(Patch* itm) {
            return {
                .data = BGSProjectileData::From(&itm->data)
            };
        }
        void Apply(Patch* itm) {
            if(this->data) this->data->Apply(&itm->data);
        }
    };
}
