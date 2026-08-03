#pragma once
#include <Config/Common.h>
#include <cstdint>
#include <optional>
namespace MPL::DynaForm::Lights
{
    struct TES_LIGHT_FLAGS
    {
        std::optional<bool> Dynamic;
        std::optional<bool> CanCarry;
        std::optional<bool> Negative;
        std::optional<bool> Flicker;
        std::optional<bool> DeepCopy;
        std::optional<bool> OffByDefault;
        std::optional<bool> FlickerSlow;
        std::optional<bool> Pulse;
        std::optional<bool> PulseSlow;
        std::optional<bool> Spotlight;
        std::optional<bool> SpotShadow;
        std::optional<bool> HemiShadow;
        std::optional<bool> OmniShadow;
        std::optional<bool> PortalStrict;
        std::optional<bool> InverseSquare;
        std::optional<bool> Linear;
        using Patch = REX::EnumSet<RE::TES_LIGHT_FLAGS, std::uint32_t>;
        void Apply(Patch* itm)
        {
            if (Dynamic) itm->set(*Dynamic, RE::TES_LIGHT_FLAGS::kDynamic);
            if (CanCarry) itm->set(*CanCarry, RE::TES_LIGHT_FLAGS::kCanCarry);
            if (Negative) itm->set(*Negative, RE::TES_LIGHT_FLAGS::kNegative);
            if (Flicker) itm->set(*Flicker, RE::TES_LIGHT_FLAGS::kFlicker);
            if (DeepCopy) itm->set(*DeepCopy, RE::TES_LIGHT_FLAGS::kDeepCopy);
            if (OffByDefault) itm->set(*OffByDefault, RE::TES_LIGHT_FLAGS::kOffByDefault);
            if (FlickerSlow) itm->set(*FlickerSlow, RE::TES_LIGHT_FLAGS::kFlickerSlow);
            if (Pulse) itm->set(*Pulse, RE::TES_LIGHT_FLAGS::kPulse);
            if (PulseSlow) itm->set(*PulseSlow, RE::TES_LIGHT_FLAGS::kPulseSlow);
            if (Spotlight) itm->set(*Spotlight, RE::TES_LIGHT_FLAGS::kSpotlight);
            if (SpotShadow) itm->set(*SpotShadow, RE::TES_LIGHT_FLAGS::kSpotShadow);
            if (HemiShadow) itm->set(*HemiShadow, RE::TES_LIGHT_FLAGS::kHemiShadow);
            if (OmniShadow) itm->set(*OmniShadow, RE::TES_LIGHT_FLAGS::kOmniShadow);
            if (PortalStrict) itm->set(*PortalStrict, RE::TES_LIGHT_FLAGS::kPortalStrict);
            if (InverseSquare) itm->set(*InverseSquare, static_cast<RE::TES_LIGHT_FLAGS>(1 << 14));
            if (Linear) itm->set(*Linear, static_cast<RE::TES_LIGHT_FLAGS>(1 << 15));
        }
        static TES_LIGHT_FLAGS From(Patch* itm)
        {
            auto underlying = itm->underlying();
            return {
                .Dynamic = ((underlying & 1 << 0) != 0),
                .CanCarry = ((underlying & 1 << 1) != 0),
                .Negative = ((underlying & 1 << 2) != 0),
                .Flicker = ((underlying & 1 << 3) != 0),
                .DeepCopy = ((underlying & 1 << 4) != 0),
                .OffByDefault = ((underlying & 1 << 5) != 0),
                .FlickerSlow = ((underlying & 1 << 6) != 0),
                .Pulse = ((underlying & 1 << 7) != 0),
                .PulseSlow = ((underlying & 1 << 8) != 0),
                .Spotlight = ((underlying & 1 << 9) != 0),
                .SpotShadow = ((underlying & 1 << 10) != 0),
                .HemiShadow = ((underlying & 1 << 11) != 0),
                .OmniShadow = ((underlying & 1 << 12) != 0),
                .PortalStrict = ((underlying & 1 << 13) != 0),
                .InverseSquare = ((underlying & 1 << 14) != 0),
                .Linear = ((underlying & 1 << 15) != 0)
            };
        }
    };
    struct OBJ_LIGH
    {
        std::optional<std::int32_t> time;
        std::optional<std::uint32_t> radius;
        std::optional<DynaForm::Color> color;
        std::optional<TES_LIGHT_FLAGS> flags;
        std::optional<float> fallofExponent;
        std::optional<float> fov;
        std::optional<float> nearDistance;
        std::optional<float> flickerPeriodRecip;
        std::optional<float> flickerIntensityAmplitude;
        std::optional<float> flickerMovementAmplitude;
        using Patch = RE::OBJ_LIGH;
        void Apply(Patch* itm)
        {
            if (time) itm->time = *time;
            if (radius) itm->radius = *radius;
            if (color) this->color->Apply(&itm->color);
            if (flags) this->flags->Apply(&itm->flags);
            if (fallofExponent) itm->fallofExponent = *fallofExponent;
            if (fov) itm->fov = *fov;
            if (nearDistance) itm->nearDistance = *nearDistance;
            if (flickerPeriodRecip) itm->flickerPeriodRecip = *flickerPeriodRecip * 100;
            if (flickerIntensityAmplitude) itm->flickerIntensityAmplitude = *flickerIntensityAmplitude;
            if (flickerMovementAmplitude) itm->flickerMovementAmplitude = *flickerMovementAmplitude;
        }
        static OBJ_LIGH From(Patch* itm)
        {
            return {
                .time = itm->time,
                .radius = itm->radius,
                .color = DynaForm::Color::From(&itm->color),
                .flags = TES_LIGHT_FLAGS::From(&itm->flags),
                .fallofExponent = itm->fallofExponent,
                .fov = itm->fov,
                .nearDistance = itm->nearDistance,
                .flickerPeriodRecip = itm->flickerPeriodRecip,
                .flickerIntensityAmplitude = itm->flickerIntensityAmplitude,
                .flickerMovementAmplitude = itm->flickerMovementAmplitude
            };
        }
    };
    struct TESObjectLIGH
    {
        static constexpr std::string_view Name = "Light";
        std::optional<OBJ_LIGH> data;
        std::optional<float> fade;
        std::optional<DynaForm::NiColor> emittanceColor;
        using Patch = RE::TESObjectLIGH;
        void Apply(Patch* itm)
        {
            if (data) data->Apply(&itm->data);
            if (fade) itm->fade = *fade;
            if (emittanceColor) emittanceColor->Apply(&itm->emittanceColor);
        }
        static TESObjectLIGH From(Patch* itm)
        {
            return {
                .data = OBJ_LIGH::From(&itm->data),
                .fade = itm->fade,
                .emittanceColor = NiColor::From(&itm->emittanceColor)
            };
        }
    };
}  // namespace MPL::Config
