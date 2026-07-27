#pragma once
#include <Config/Common.h>
#include <optional>
namespace MPL::Config
{
    struct ExtraRoomRefData
    {
        using TopLevel = RE::TESObjectREFR;
        std::optional<MPL::Config::LiteForm> imageSpace;
        std::optional<MPL::Config::LiteForm> lightingTemplate;
        std::optional<bool> addIfMissing;
        using Patch = RE::ExtraRoomRefData;
        void Apply(Patch* itm)
        {
            if (!itm || !itm->data) return;
            if (this->imageSpace) itm->data->imageSpace = this->imageSpace->Get<RE::TESImageSpace>();
            if (this->lightingTemplate) itm->data->lightingTemplate = this->lightingTemplate->Get<RE::BGSLightingTemplate>();
        }
        static ExtraRoomRefData From(Patch* itm)
        {
            ExtraRoomRefData cpy;
            if (!itm || !itm->data) return cpy;
            if (itm->data->imageSpace) cpy.imageSpace = MPL::Config::LiteForm::FromID(cpy.imageSpace->formID);
            if (itm->data->lightingTemplate) cpy.lightingTemplate = MPL::Config::LiteForm::FromID(cpy.imageSpace->formID);
            return cpy;
        }
        bool IsValid(TopLevel* itm)
        {
            const auto* base = itm->GetBaseObject();
            return base && base->formID == 0x1F &&
                   (itm->extraList.HasType<RE::ExtraRoomRefData>() || this->addIfMissing.value_or(false));
        }
    };

    // This is initially for setting up RoomBounds / Templates
    struct TESObjectREFR
    {
        static constexpr std::string_view Name = "ObjectReference";
        std::optional<ExtraRoomRefData> roomBound;
        using Patch = RE::TESObjectREFR;
        void Apply(Patch* itm)
        {
            if (this->roomBound)
            {
                if (this->roomBound->IsValid(itm))
                {
                    if (itm->extraList.HasType<RE::ExtraRoomRefData>())
                    {
                        auto rrd = itm->extraList.GetByType<RE::ExtraRoomRefData>();
                        this->roomBound->Apply(rrd);
                    }
                    else if (this->roomBound->addIfMissing.value_or(false))
                    {
                        auto erd = RE::BSExtraData::Create<RE::ExtraRoomRefData>();
                        this->roomBound->Apply(erd);
                        itm->extraList.Add(erd);
                    }
                }
            }
        }
        static TESObjectREFR From(Patch* itm)
        {
            TESObjectREFR cpy;
            if (itm->extraList.HasType<RE::ExtraRoomRefData>())
            {
                cpy.roomBound = ExtraRoomRefData::From(itm->extraList.GetByType<RE::ExtraRoomRefData>());
            }
            return cpy;
        }
    };
}  // namespace MPL::Config
