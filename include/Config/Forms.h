#pragma once
#include <rfl.hpp>
namespace MPL::Config
{
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
                return MPL::Config::LiteForm{ .formID = 0x0 };
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
}  // namespace MPL::Config
namespace rfl
{
    template <>
    struct Reflector<MPL::Config::LiteForm>
    {
        using ReflType = std::string;
        static ReflType from(const MPL::Config::LiteForm& v)
        {
            return v.ToString();
        }
        static MPL::Config::LiteForm to(const ReflType& v)
        {
            return MPL::Config::LiteForm::FromString(v);
        }
    };
}  // namespace rfl
