#include <CompatibilityChecker.h>
#include <Config.h>
#include <Externals/MMSF_API.h>
#include <Hooks.h>
#include <LPCommand.h>
#include <LumaService.h>
#include <Papyrus.h>
#include <Plugin.h>
#include <REL/Version.h>
void OnSKSEMessage(SKSE::MessagingInterface::Message* a_message)
{
    if (!a_message)
    {
        return;
    }
    switch (a_message->type)
    {
    case MPL::API::MMSF::MMSFMessage::kMessage_MMSFServicesReg:
        static_cast<MPL::API::MMSF::MMSFMessage*>(a_message->data)->API->RegisterService(MPL::LumaService::LumaService::GetSingleton());
        break;
    case SKSE::MessagingInterface::kPostLoad:
        SKSE::GetMessagingInterface()->RegisterListener("MMSF", OnSKSEMessage);
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
    SKSE::GetMessagingInterface()->RegisterListener(nullptr, OnSKSEMessage);
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
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary);
