#pragma once
#include <Config/Common.h>
namespace MPL::DynaForm::ImageSpace
{
    struct HDR
    {
        std::optional<float> eyeAdaptSpeed;
        std::optional<float> bloomBlurRadius;
        std::optional<float> bloomThreshold;
        std::optional<float> bloomScale;
        std::optional<float> receiveBloomThreshold;
        std::optional<float> white;
        std::optional<float> sunlightScale;
        std::optional<float> skyScale;
        std::optional<float> eyeAdaptStrength;
        using Patch = RE::ImageSpaceBaseData::HDR;
        void Apply(Patch* itm)
        {
            if (this->eyeAdaptSpeed) itm->eyeAdaptSpeed = *this->eyeAdaptSpeed;
            if (this->bloomBlurRadius) itm->bloomBlurRadius = *this->bloomBlurRadius;
            if (this->bloomThreshold) itm->bloomThreshold = *this->bloomThreshold;
            if (this->bloomScale) itm->bloomScale = *this->bloomScale;
            if (this->receiveBloomThreshold) itm->receiveBloomThreshold = *this->receiveBloomThreshold;
            if (this->white) itm->white = *this->white;
            if (this->sunlightScale) itm->sunlightScale = *this->sunlightScale;
            if (this->skyScale) itm->skyScale = *this->skyScale;
            if (this->eyeAdaptStrength) itm->eyeAdaptStrength = *this->eyeAdaptStrength;
        };
        static HDR From(HDR::Patch* itm)
        {
            return {
                .eyeAdaptSpeed = itm->eyeAdaptSpeed,
                .bloomBlurRadius = itm->bloomBlurRadius,
                .bloomThreshold = itm->bloomThreshold,
                .bloomScale = itm->bloomScale,
                .receiveBloomThreshold = itm->receiveBloomThreshold,
                .white = itm->white,
                .sunlightScale = itm->sunlightScale,
                .skyScale = itm->skyScale,
                .eyeAdaptStrength = itm->eyeAdaptStrength,
            };
        }
    };

    struct Cinematic
    {
        std::optional<float> saturation;
        std::optional<float> brightness;
        std::optional<float> contrast;
        using Patch = RE::ImageSpaceBaseData::Cinematic;
        void Apply(Patch* itm)
        {
            if (this->saturation) itm->saturation = *this->saturation;
            if (this->brightness) itm->brightness = *this->brightness;
            if (this->contrast) itm->contrast = *this->contrast;
        }
        static Cinematic From(Patch* itm)
        {
            return {
                .saturation = itm->saturation,
                .brightness = itm->brightness,
                .contrast = itm->contrast,
            };
        }
    };

    struct Tint
    {
        std::optional<float> amount;
        std::optional<NiColor> color;
        using Patch = RE::ImageSpaceBaseData::Tint;
        void Apply(Patch* itm)
        {
            if (this->amount) itm->amount = *this->amount;
            if (this->color) this->color->Apply(&itm->color);
        }
        static Tint From(Patch* itm)
        {
            return {
                .amount = itm->amount,
                .color = NiColor::From(&itm->color),
            };
        }
    };

    struct DepthOfField
    {
        std::optional<float> strength;
        std::optional<float> distance;
        std::optional<float> range;
        using Patch = RE::ImageSpaceBaseData::DepthOfField;
        void Apply(Patch* itm)
        {
            if (this->strength) itm->strength = *this->strength;
            if (this->distance) itm->distance = *this->distance;
            if (this->range) itm->range = *this->range;
        }
        static DepthOfField From(Patch* itm)
        {
            return {
                .strength = itm->strength,
                .distance = itm->distance,
                .range = itm->range,
            };
        }
    };

    struct ImageSpaceBaseData
    {
        std::optional<HDR> hdr;
        std::optional<Cinematic> cinematic;
        std::optional<Tint> tint;
        std::optional<DepthOfField> depthOfField;
        using Patch = RE::ImageSpaceBaseData;
        void Apply(Patch* itm)
        {
            if (this->hdr) this->hdr->Apply(&itm->hdr);
            if (this->cinematic) this->cinematic->Apply(&itm->cinematic);
            if (this->tint) this->tint->Apply(&itm->tint);
            if (this->depthOfField) this->depthOfField->Apply(&itm->depthOfField);
        }
        static ImageSpaceBaseData From(Patch* itm)
        {
            return {
                .hdr = HDR::From(&itm->hdr),
                .cinematic = Cinematic::From(&itm->cinematic),
                .tint = Tint::From(&itm->tint),
                .depthOfField = DepthOfField::From(&itm->depthOfField),
            };
        }
    };

    struct TESImageSpace
    {
        static constexpr std::string_view Name = "ImageSpaces";
        std::optional<ImageSpaceBaseData> data;
        using Patch = RE::TESImageSpace;
        void Apply(TESImageSpace::Patch* itm)
        {
            if (this->data) this->data->Apply(&itm->data);
        }
        static TESImageSpace From(TESImageSpace::Patch* itm)
        {
            return {
                .data = ImageSpaceBaseData::From(&itm->data)
            };
        }
    };
}  // namespace MPL::Config::ImageSpace
