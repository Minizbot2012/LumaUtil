#include "Externals/MMSF_API.h"
#include "SKSE/Interfaces.h"
#include <Config.h>
#include <Hooks.h>
#include <Papyrus.h>
#include <Plugin.h>
#include <REL/Version.h>
#include <SKSE/API.h>
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

void MessageHandler(SKSE::MessagingInterface::Message* msg)
{
    auto sta = MPL::Config::StatData::GetSingleton();
        switch (msg->type)
        {
        case SKSE::MessagingInterface::kPostLoad:
            sta->MMSF = MPL::API::MMSF::RequestMMSFAPI();
        break;
    default:
        break;
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);
    logger::info("Game version : {}", a_skse->RuntimeVersion().string());
    MPL::Hooks::Install();
    SKSE::GetPapyrusInterface()->Register(MPL::Papyrus::Bind);
    SKSE::GetPapyrusInterface()->Register(MPL::Papyrus::BindWeatherPatcher);
    SKSE::GetMessagingInterface()->RegisterListener(MessageHandler);
    SKSE::GetMessagingInterface()->RegisterListener(MPL::WeatherPatcher::OnSKSEMessage);
    auto ser = SKSE::GetSerializationInterface();
    ser->SetUniqueID('LUMA');
    ser->SetSaveCallback(Serialize);
    ser->SetLoadCallback(Deserialize);
    ser->SetRevertCallback(Revert);
    return true;
};
SKSEPluginInfo(
        .Version = REL::Version{ MPL::Plugin::MAJOR, MPL::Plugin::MINOR, MPL::Plugin::PATCH, 0 },
    .Name = "LumaUtil"sv,
    .Author = "mini"sv,
    .SupportEmail = ""sv,
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary)
