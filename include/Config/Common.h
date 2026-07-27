#pragma once
#include <Config/Forms.h>
#include <rfl.hpp>

namespace MPL::Config
{
    struct NiColor
    {
        std::optional<uint32_t> red;
        std::optional<uint32_t> green;
        std::optional<uint32_t> blue;
        using Patch = RE::NiColor;
        void Apply(Patch* itm)
        {
            if (this->red) itm->red = ((float) *this->red) / 255.f;
            if (this->green) itm->green = ((float) *this->green) / 255.f;
            if (this->blue) itm->blue = ((float) *this->blue / 255.f);
        }
        static NiColor From(Patch* itm)
        {
            NiColor cpy{
                .red = (uint32_t) (itm->red * 255),
                .green = (uint32_t) (itm->green * 255),
                .blue = (uint32_t) (itm->blue * 255),
            };
            return cpy;
        }
    };

    struct Color
    {
        std::optional<uint8_t> alpha;
        std::optional<uint8_t> red;
        std::optional<uint8_t> green;
        std::optional<uint8_t> blue;
        using Patch = RE::Color;
        void Apply(Patch* itm)
        {
            if (this->alpha) itm->alpha = *this->alpha;
            if (this->red) itm->red = *this->red;
            if (this->green) itm->green = *this->green;
            if (this->blue) itm->blue = *this->blue;
        };
        static Color From(Patch* itm)
        {
            {
                Color col{ .alpha = itm->alpha,
                    .red = itm->red,
                    .green = itm->green,
                    .blue = itm->blue };
                return col;
            }
        }
    };
}  // namespace MPL::Config
