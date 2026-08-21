#include "SKSE/Interfaces.h"
#include <Config.h>
#include <Hooks.h>
#include <LPCommand.h>
#include <Papyrus.h>
#include <Plugin.h>
#include <REL/Version.h>
#include <SKSE/API.h>
#include <CompatibilityChecker.h>
namespace
{
    constexpr std::uint32_t kCellChangeSerializationVersion = 1;

    void OnSKSEMessage(SKSE::MessagingInterface::Message* a_message)
    {
        if (!a_message)
        {
            return;
        }
        switch (a_message->type)
        {
        case SKSE::MessagingInterface::kPostLoad:
            MPL::LPCommand::CaptureCommandSlot(true);
            break;
        case SKSE::MessagingInterface::kDataLoaded:
            MPL::LPCommand::ReleaseCommandSlot();
            MPL::CompatibilityChecker::Initialize();
            break;
        default:
            break;
        }
    }
}  // namespace

void Serialize(SKSE::SerializationInterface* ser)
{
    if (!ser->OpenRecord('CLCH', 0x1))
    {
        logger::error("Failed to open record");
        return;
    }
    MPL::Config::StatData::GetSingleton()->cellLoad.Save(ser);
}

void Deserialize(SKSE::SerializationInterface* ser)
{
    uint32_t type;
    uint32_t version;
    uint32_t len;
    while (ser->GetNextRecordInfo(type, version, len))
    {
        switch (type)
        {
        case 'CLCH':
            MPL::Config::StatData::GetSingleton()->cellLoad.Load(ser);
            break;
        default:
            break;
        }
    }
}

void Revert(SKSE::SerializationInterface* ser)
{
    MPL::Config::StatData::GetSingleton()->cellLoad.Revert(ser);
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);
    MPL::LPCommand::CaptureCommandSlot(false);
    logger::info("Game version : {}", a_skse->RuntimeVersion().string());
    MPL::Hooks::Install();
    SKSE::GetPapyrusInterface()->Register(MPL::Papyrus::Bind);
    SKSE::GetMessagingInterface()->RegisterListener(OnSKSEMessage);
    auto ser = SKSE::GetSerializationInterface();
    ser->SetUniqueID('LUMA');
    ser->SetSaveCallback(Serialize);
    ser->SetLoadCallback(Deserialize);
    ser->SetRevertCallback(Revert);
    return true;
};
SKSEPluginInfo(
    .Version = REL::Version{ MPL::Plugin::MAJOR, MPL::Plugin::MINOR, MPL::Plugin::PATCH, 0 },
    .Name = MPL::Plugin::PROJECT,
    .Author = "Mini"sv,
    .SupportEmail = ""sv,
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary
);
