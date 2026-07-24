#pragma once
#include <cstdint>
namespace MPL::API::MMSF
{
    class ServiceMap
    {
    public:
        virtual uint8_t GetMMSFVersion() = 0;
        virtual RE::FormID LookupFormIDForEDID(std::string) = 0;
        virtual std::string LookupEDIDForFormID(RE::FormID) = 0;
        virtual RE::TESForm* LookupCachedForm(std::string) = 0;
        virtual RE::TESForm* AllocateForm(std::string, RE::FormType) = 0;
        virtual bool LoadOrderChanged() = 0;
        virtual uint64_t GetLoadOrderHash() = 0;
    };
    struct MMSFMessage
    {
        enum message_type : uint32_t
        {
            kMessage_GetInterface = 'MMSF'
        };
        ServiceMap* API;
    };
    static const char* sender = "MMSF";
    [[nodiscard]] inline ServiceMap* RequestMMSFAPI()
    {
        MMSFMessage message;
        SKSE::GetMessagingInterface()->Dispatch(MMSFMessage::kMessage_GetInterface, &message, sizeof(MMSFMessage), sender);
        if (message.API != nullptr)
        {
            return message.API;
        }
        else
        {
            return nullptr;
        }
    }
}  // namespace MPL::API
