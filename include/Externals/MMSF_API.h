#pragma once
#include <cstdint>
#include <type_traits>
namespace MPL::API::MMSF
{
    enum struct MMSFAPIFeatures : uint64_t
    {
        kCaching = 1 << 0,
        kAllocator = 1 << 1
    };
    constexpr MMSFAPIFeatures operator|(MMSFAPIFeatures lhs, MMSFAPIFeatures rhs)
    {
        return static_cast<MMSFAPIFeatures>(static_cast<std::underlying_type_t<MMSFAPIFeatures>>(lhs) | static_cast<std::underlying_type_t<MMSFAPIFeatures>>(rhs));
    }
    constexpr MMSFAPIFeatures operator&(MMSFAPIFeatures lhs, MMSFAPIFeatures rhs)
    {
        return static_cast<MMSFAPIFeatures>(static_cast<std::underlying_type_t<MMSFAPIFeatures>>(lhs) & static_cast<std::underlying_type_t<MMSFAPIFeatures>>(rhs));
    }
    constexpr MMSFAPIFeatures EmbedVersion(uint8_t ver) {
        return static_cast<MMSFAPIFeatures>(static_cast<std::underlying_type_t<MMSFAPIFeatures>>(ver) << 56);
    }
    constexpr uint8_t GetVersion(MMSFAPIFeatures features) {
        return static_cast<uint8_t>(static_cast<std::underlying_type_t<MMSFAPIFeatures>>(features) >> 56);
    }
    class Interface
    {
    public:
        virtual MMSFAPIFeatures GetVersion() = 0;
        virtual RE::FormID LookupFormIDForEDID(std::string) = 0;
        virtual std::string LookupEDIDForFormID(RE::FormID) = 0;
        virtual RE::TESForm* LookupCachedForm(std::string) = 0;
        virtual RE::TESForm* AllocateForm(std::string, RE::FormType) = 0;
    };

    struct MMSFMessage
    {
        enum message_type : uint32_t
        {
            kMessage_GetInterface = 'MMSF'
        };
        Interface* API;
    };
    static const char* sender = "MMSF";
    inline Interface* RequestMMSFAPI()
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
}  // namespace MPL::API::MMSF
