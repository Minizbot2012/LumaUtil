#pragma once
#include <rfl.hpp>

namespace MPL::DynaForm
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
            return {
                .red = (uint32_t) (itm->red * 255),
                .green = (uint32_t) (itm->green * 255),
                .blue = (uint32_t) (itm->blue * 255),
            };
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
                return {
                    .alpha = itm->alpha,
                    .red = itm->red,
                    .green = itm->green,
                    .blue = itm->blue
                };
            }
        }
    };
    struct LiteForm
    {
        RE::FormID formID;
        template <class T>
        T* Get() const
        {
            return formID != 0x0 ? RE::TESForm::LookupByID<T>(formID) : nullptr;
        };
        std::string ToString() const
        {
            if (this->formID == 0x0)
            {
                return "null";
            }
            const auto* form = this->Get<RE::TESForm>();
            if (!form)
            {
                return "null";
            }
            if (form->sourceFiles.array == nullptr)
            {
                return "null";
            }
            if (const auto* edid = form->GetFormEditorID(); edid && *edid)
            {
                return std::string(edid);
            }
            const auto* source = form->GetFile(0);
            return source ? format("{:06X}:{}", form->GetLocalFormID(), source->GetFilename()) : "null";
        }
        static LiteForm FromID(RE::FormID id) { return { .formID = id }; };
        static LiteForm FromForm(RE::TESForm* frm) { return { .formID = frm->formID }; }
        static LiteForm FromString(std::string str)
        {
            if (str == "null")
            {
                return { .formID = 0x0 };
            }
            auto loc = str.find(":");
            if (loc != std::string::npos)
            {
                auto lfid = strtoul(str.substr(0, loc).c_str(), nullptr, 16);
                auto file = str.substr(loc + 1);
                auto* dh = RE::TESDataHandler::GetSingleton();
                return { .formID = dh->LookupFormID(lfid, file) };
            }
            else
            {
                auto* frm = RE::TESForm::LookupByEditorID(str);
                if (frm)
                {
                    return { .formID = frm->formID };
                }
                else
                {
                    return { .formID = 0x0 };
                }
            }
        }
    };
}  // namespace MPL::DynaForm
namespace rfl
{
    template <>
    struct Reflector<MPL::DynaForm::LiteForm>
    {
        using ReflType = std::string;
        static ReflType from(const MPL::DynaForm::LiteForm& v)
        {
            return v.ToString();
        }
        static MPL::DynaForm::LiteForm to(const ReflType& v)
        {
            return MPL::DynaForm::LiteForm::FromString(v);
        }
    };
}  // namespace rfl
