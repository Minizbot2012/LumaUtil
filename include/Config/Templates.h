#pragma once
#include <Config/Common.h>
#include <cstdint>
namespace MPL::DynaForm::Template
{

    struct MinMaxColor
    {
        std::optional<Color> max;
        std::optional<Color> min;
        using Patch = RE::BGSDirectionalAmbientLightingColors::Directional::MaxMin<RE::Color>;
        static MinMaxColor From(Patch* itm)
        {
            return {
                .max = Color::From(&itm->max),
                .min = Color::From(&itm->min),
            };
        }
        void Apply(Patch* itm)
        {
            if (this->max) this->max->Apply(&itm->max);
            if (this->min) this->min->Apply(&itm->min);
        }
    };

    struct Directional
    {
        std::optional<MinMaxColor> x;
        std::optional<MinMaxColor> y;
        std::optional<MinMaxColor> z;
        using Patch = RE::BGSDirectionalAmbientLightingColors::Directional;
        static Directional From(Patch* itm)
        {
            Directional cpy{
                .x = MinMaxColor::From(&itm->x),
                .y = MinMaxColor::From(&itm->y),
                .z = MinMaxColor::From(&itm->z)
            };
            return cpy;
        }
        void Apply(Patch* itm)
        {
            if (this->x) this->x->Apply(&itm->x);
            if (this->y) this->y->Apply(&itm->y);
            if (this->z) this->z->Apply(&itm->z);
        }
    };

    struct DirectionalAmbientLightingColor
    {
        std::optional<Directional> directional;
        std::optional<Color> specular;
        std::optional<float> scale;
        using Patch = RE::BGSDirectionalAmbientLightingColors;
        void Apply(Patch* itm)
        {
            if (this->directional) this->directional->Apply(&itm->directional);
            if (this->specular) this->specular->Apply(&itm->specular);
            if (this->scale) itm->fresnelPower = *this->scale;
        };
        static DirectionalAmbientLightingColor From(Patch* itm)
        {
            return {
                .directional = Directional::From(&itm->directional),
                .specular = Color::From(&itm->specular),
                .scale = itm->fresnelPower,
            };
        }
    };
    struct Inherit
    {
        std::optional<bool> AmbientColor;
        std::optional<bool> DirectionalColor;
        std::optional<bool> FogColor;
        std::optional<bool> FogNear;
        std::optional<bool> FogFar;
        std::optional<bool> DirectionalRotation;
        std::optional<bool> DirectionalFade;
        std::optional<bool> ClipDistance;
        std::optional<bool> FogPower;
        std::optional<bool> FogMax;
        std::optional<bool> LightFadeDistance;
        using Patch = REX::EnumSet<RE::INTERIOR_DATA::Inherit, uint32_t>;
        void Apply(Patch* itm)
        {
            if (this->AmbientColor) itm->set(*this->AmbientColor, RE::INTERIOR_DATA::Inherit::kAmbientColor);
            if (this->DirectionalColor) itm->set(*this->DirectionalColor, RE::INTERIOR_DATA::Inherit::kDirectionalColor);
            if (this->FogColor) itm->set(*this->FogColor, RE::INTERIOR_DATA::Inherit::kFogColor);
            if (this->FogNear) itm->set(*this->FogNear, RE::INTERIOR_DATA::Inherit::kFogNear);
            if (this->FogFar) itm->set(*this->FogFar, RE::INTERIOR_DATA::Inherit::kFogFar);
            if (this->DirectionalRotation) itm->set(*this->DirectionalRotation, RE::INTERIOR_DATA::Inherit::kDirectionalRotation);
            if (this->DirectionalFade) itm->set(*this->DirectionalFade, RE::INTERIOR_DATA::Inherit::kDirectionalFade);
            if (this->ClipDistance) itm->set(*this->ClipDistance, RE::INTERIOR_DATA::Inherit::kClipDistance);
            if (this->FogPower) itm->set(*this->FogPower, RE::INTERIOR_DATA::Inherit::kFogPower);
            if (this->FogMax) itm->set(*this->FogMax, RE::INTERIOR_DATA::Inherit::kFogMax);
            if (this->LightFadeDistance) itm->set(*this->LightFadeDistance, RE::INTERIOR_DATA::Inherit::kLightFadeDistances);
        }
        static Inherit From(Patch* itm)
        {
            return {
                .AmbientColor = (*itm & RE::INTERIOR_DATA::Inherit::kAmbientColor).underlying() != 0,
                .DirectionalColor = (*itm & RE::INTERIOR_DATA::Inherit::kDirectionalColor).underlying() != 0,
                .FogColor = (*itm & RE::INTERIOR_DATA::Inherit::kFogColor).underlying() != 0,
                .FogNear = (*itm & RE::INTERIOR_DATA::Inherit::kFogNear).underlying() != 0,
                .FogFar = (*itm & RE::INTERIOR_DATA::Inherit::kFogFar).underlying() != 0,
                .DirectionalRotation = (*itm & RE::INTERIOR_DATA::Inherit::kDirectionalRotation).underlying() != 0,
                .DirectionalFade = (*itm & RE::INTERIOR_DATA::Inherit::kDirectionalFade).underlying() != 0,
                .ClipDistance = (*itm & RE::INTERIOR_DATA::Inherit::kClipDistance).underlying() != 0,
                .FogPower = (*itm & RE::INTERIOR_DATA::Inherit::kFogPower).underlying() != 0,
                .FogMax = (*itm & RE::INTERIOR_DATA::Inherit::kFogMax).underlying() != 0,
                .LightFadeDistance = (*itm & RE::INTERIOR_DATA::Inherit::kLightFadeDistances).underlying() != 0,
            };
        }
    };
    struct INTERIOR_DATA
    {
        std::optional<Color> ambient;
        std::optional<Color> directional;
        std::optional<Color> fogColorNear;
        std::optional<Color> fogColorFar;
        std::optional<float> fogNear;
        std::optional<float> fogFar;
        std::optional<float> fogMax;
        std::optional<float> fogPower;
        std::optional<float> directionalFade;
        std::optional<float> clipDist;
        std::optional<float> lightFadeStart;
        std::optional<float> lightFadeEnd;
        std::optional<uint32_t> directionalXY;
        std::optional<uint32_t> directionalZ;
        std::optional<Inherit> inherit;
        std::optional<DirectionalAmbientLightingColor> directionalAmbientLightingColors;
        using Patch = RE::INTERIOR_DATA;
        void Apply(Patch* itm)
        {
            if (this->ambient) this->ambient->Apply(&itm->ambient);
            if (this->directional) this->directional->Apply(&itm->directional);
            if (this->directionalAmbientLightingColors) this->directionalAmbientLightingColors->Apply(&itm->directionalAmbientLightingColors);
            if (this->fogColorNear) this->fogColorNear->Apply(&itm->fogColorNear);
            if (this->fogColorFar) this->fogColorFar->Apply(&itm->fogColorFar);
            if (this->fogNear) itm->fogNear = *this->fogNear;
            if (this->fogFar) itm->fogFar = *this->fogFar;
            if (this->fogMax) itm->fogClamp = *this->fogMax;
            if (this->directionalFade) itm->directionalFade = *this->directionalFade;
            if (this->clipDist) itm->clipDist = *this->clipDist;
            if (this->fogPower) itm->fogPower = *this->fogPower;
            if (this->directionalXY) itm->directionalXY = *this->directionalXY;
            if (this->directionalZ) itm->directionalZ = *this->directionalZ;
            if (this->lightFadeStart) itm->lightFadeStart = *this->lightFadeStart == 0.0f ? 4096.0f : *this->lightFadeStart;
            if (this->lightFadeEnd) itm->lightFadeEnd = *this->lightFadeEnd == 0.0f ? 30720.0f : *this->lightFadeEnd;
            if (this->inherit) this->inherit->Apply(&itm->lightingTemplateInheritanceFlags);
        }
        static INTERIOR_DATA From(Patch* itm)
        {
            return {
                .ambient = Color::From(&itm->ambient),
                .directional = Color::From(&itm->directional),
                .fogColorNear = Color::From(&itm->fogColorNear),
                .fogColorFar = Color::From(&itm->fogColorFar),
                .fogNear = itm->fogNear,
                .fogFar = itm->fogFar,
                .fogMax = itm->fogClamp,
                .fogPower = itm->fogPower,
                .directionalFade = itm->directionalFade,
                .clipDist = itm->clipDist,
                .lightFadeStart = itm->lightFadeStart,
                .lightFadeEnd = itm->lightFadeEnd,
                .directionalXY = itm->directionalXY,
                .directionalZ = itm->directionalZ,
                .inherit = Inherit::From(&itm->lightingTemplateInheritanceFlags),
                .directionalAmbientLightingColors = DirectionalAmbientLightingColor::From(&itm->directionalAmbientLightingColors),
            };
        }
    };

    struct BGSLightingTemplate
    {
        static constexpr std::string_view Name = "LightingTemplates";
        std::optional<INTERIOR_DATA> data;
        std::optional<DirectionalAmbientLightingColor> directionalAmbientLightingColors;
        using Patch = RE::BGSLightingTemplate;
        void Apply(Patch* itm)
        {
            if (this->data) this->data->Apply(&itm->data);
            if (this->directionalAmbientLightingColors) this->directionalAmbientLightingColors->Apply(&itm->directionalAmbientLightingColors);
        };
        static BGSLightingTemplate From(Patch* itm)
        {
            return {
                .data = INTERIOR_DATA::From(&itm->data),
                .directionalAmbientLightingColors = DirectionalAmbientLightingColor::From(&itm->directionalAmbientLightingColors),
            };
        }
    };
}  // namespace MPL::Config::Template
