#pragma once
#include <Config/Common.h>
#include <Config/Templates.h>
#include <optional>
namespace MPL::DynaForm::Cell
{
    struct Flags
    {
        std::optional<bool> IsInteriorCell;
        std::optional<bool> HasWater;
        std::optional<bool> CanTravelFromHere;
        std::optional<bool> NoLODWater;
        std::optional<bool> HasTempData;
        std::optional<bool> PublicArea;
        std::optional<bool> HandChanged;
        std::optional<bool> ShowSky;
        std::optional<bool> UsesSkyLighting;
        std::optional<bool> WarnToLeave;
        std::optional<bool> Unknown10;
        std::optional<bool> Unknown11;
        std::optional<bool> Unknown12;
        std::optional<bool> Unknown13;
        std::optional<bool> Unknown14;
        std::optional<bool> SunlightShadows;
        using Patch = REX::EnumSet<RE::TESObjectCELL::Flag, uint16_t>;
        void Apply(Patch* itm)
        {
            if (IsInteriorCell) itm->set(*this->IsInteriorCell, RE::TESObjectCELL::Flag::kIsInteriorCell);
            if (HasWater) itm->set(*this->HasWater, RE::TESObjectCELL::Flag::kHasWater);
            if (CanTravelFromHere) itm->set(*this->CanTravelFromHere, RE::TESObjectCELL::Flag::kCanTravelFromHere);
            if (NoLODWater) itm->set(*this->NoLODWater, RE::TESObjectCELL::Flag::kNoLODWater);
            if (HasTempData) itm->set(*this->HasTempData, RE::TESObjectCELL::Flag::kHasTempData);
            if (PublicArea) itm->set(*this->PublicArea, RE::TESObjectCELL::Flag::kPublicArea);
            if (HandChanged) itm->set(*this->HandChanged, RE::TESObjectCELL::Flag::kHandChanged);
            if (ShowSky) itm->set(*this->ShowSky, RE::TESObjectCELL::Flag::kShowSky);
            if (UsesSkyLighting) itm->set(*this->UsesSkyLighting, RE::TESObjectCELL::Flag::kUseSkyLighting);
            if (WarnToLeave) itm->set(*this->WarnToLeave, RE::TESObjectCELL::Flag::kWarnToLeave);
            if (Unknown10) itm->set(*this->Unknown10, static_cast<RE::TESObjectCELL::Flag>(1 << 10));
            if (Unknown11) itm->set(*this->Unknown11, static_cast<RE::TESObjectCELL::Flag>(1 << 11));
            if (Unknown12) itm->set(*this->Unknown12, static_cast<RE::TESObjectCELL::Flag>(1 << 12));
            if (Unknown13) itm->set(*this->Unknown13, static_cast<RE::TESObjectCELL::Flag>(1 << 13));
            if (Unknown14) itm->set(*this->Unknown14, static_cast<RE::TESObjectCELL::Flag>(1 << 14));
            if (SunlightShadows) itm->set(*this->SunlightShadows, static_cast<RE::TESObjectCELL::Flag>(1 << 15));
        }
        static Flags From(Patch* itm)
        {
            return{
                .IsInteriorCell = (*itm & RE::TESObjectCELL::Flag::kIsInteriorCell).underlying() != 0,
                .HasWater = (*itm & RE::TESObjectCELL::Flag::kHasWater).underlying() != 0,
                .CanTravelFromHere = (*itm & RE::TESObjectCELL::Flag::kCanTravelFromHere).underlying() != 0,
                .NoLODWater = (*itm & RE::TESObjectCELL::Flag::kNoLODWater).underlying() != 0,
                .HasTempData = (*itm & RE::TESObjectCELL::Flag::kHasTempData).underlying() != 0,
                .PublicArea = (*itm & RE::TESObjectCELL::Flag::kPublicArea).underlying() != 0,
                .HandChanged = (*itm & RE::TESObjectCELL::Flag::kHandChanged).underlying() != 0,
                .ShowSky = (*itm & RE::TESObjectCELL::Flag::kShowSky).underlying() != 0,
                .UsesSkyLighting = (*itm & RE::TESObjectCELL::Flag::kUseSkyLighting).underlying() != 0,
                .WarnToLeave = (*itm & RE::TESObjectCELL::Flag::kWarnToLeave).underlying() != 0,
                .Unknown10 = (itm->underlying() & 1 << 10) != 0,
                .Unknown11 = (itm->underlying() & 1 << 11) != 0,
                .Unknown12 = (itm->underlying() & 1 << 12) != 0,
                .Unknown13 = (itm->underlying() & 1 << 13) != 0,
                .Unknown14 = (itm->underlying() & 1 << 14) != 0,
                .SunlightShadows = (itm->underlying() & 1 << 15) != 0,
            };
        }
    };
    struct TESObjectCELL
    {
        static constexpr std::string_view Name = "Cell";
        std::optional<Flags> flags;
        std::optional<DynaForm::LiteForm> skylight;
        std::optional<DynaForm::Template::INTERIOR_DATA> lighting;
        std::optional<DynaForm::LiteForm> lightTemplate;
        std::optional<DynaForm::LiteForm> imagespace;
        using Patch = RE::TESObjectCELL;
        void Apply(Patch* itm)
        {
            if (this->flags) this->flags->Apply(&itm->cellFlags);
            if (this->skylight)
            {
                auto frm = this->skylight->Get<RE::TESRegion>();
                if (frm != nullptr)
                {
                    RE::ExtraCellSkyRegion* dat = nullptr;
                    if (itm->extraList.HasType<RE::ExtraCellSkyRegion>())
                    {
                        dat = itm->extraList.GetByType<RE::ExtraCellSkyRegion>();
                    }
                    else
                    {
                        dat = RE::BSExtraData::Create<RE::ExtraCellSkyRegion>();
                        itm->extraList.Add(dat);
                    }
                    dat->skyRegion = frm;
                }
            }
            if (this->lighting) this->lighting->Apply(itm->GetLighting());
            if (this->lightTemplate)
            {
                auto frm = this->lightTemplate->Get<RE::BGSLightingTemplate>();
                if (frm != nullptr) itm->GetRuntimeData().lightingTemplate = frm;
            }
            if (this->imagespace)
            {
                auto frm = this->imagespace->Get<RE::TESImageSpace>();
                if (frm != nullptr)
                {
                    RE::ExtraCellImageSpace* dat = nullptr;
                    if (itm->extraList.HasType<RE::ExtraCellImageSpace>())
                    {
                        dat = itm->extraList.GetByType<RE::ExtraCellImageSpace>();
                    }
                    else
                    {
                        dat = RE::BSExtraData::Create<RE::ExtraCellImageSpace>();
                        itm->extraList.Add(dat);
                    }
                    dat->imageSpace = frm;
                }
            }
        }
        static TESObjectCELL From(Patch* itm)
        {
            TESObjectCELL cpy{
                .flags = Flags::From(&itm->cellFlags),
            };
            if (itm->IsInteriorCell())
            {
                cpy.lighting = Template::INTERIOR_DATA::From(itm->GetRuntimeData().cellData.interior);
            }
            if (itm->GetRuntimeData().lightingTemplate != nullptr)
            {
                cpy.lightTemplate = DynaForm::LiteForm::FromID(itm->GetRuntimeData().lightingTemplate->formID);
            }
            if (itm->extraList.HasType<RE::ExtraCellImageSpace>())
            {
                auto* dat = itm->extraList.GetByType<RE::ExtraCellImageSpace>();
                if (dat->imageSpace != nullptr) cpy.imagespace = DynaForm::LiteForm::FromID(dat->imageSpace->formID);
            }
            if (itm->extraList.HasType<RE::ExtraCellSkyRegion>())
            {
                auto* dat = itm->extraList.GetByType<RE::ExtraCellSkyRegion>();
                if (dat->skyRegion != nullptr) cpy.skylight = DynaForm::LiteForm::FromID(dat->skyRegion->formID);
            }
            return cpy;
        }
    };
}  // namespace MPL::Config::Cell
