#pragma once
#include <rfl/Generic.hpp>
#include <cstdint>
namespace MPL::API::MMSF
{
    enum struct MMSFAPIFeatures : uint64_t
    {
        kCaching = 1 << 0,
        kAllocator = 1 << 1,
        kCoreService = 1 << 2,
    };
    constexpr MMSFAPIFeatures operator|(MMSFAPIFeatures lhs, MMSFAPIFeatures rhs)
    {
        return static_cast<MMSFAPIFeatures>(static_cast<std::underlying_type_t<MMSFAPIFeatures>>(lhs) | static_cast<std::underlying_type_t<MMSFAPIFeatures>>(rhs));
    }
    constexpr MMSFAPIFeatures operator&(MMSFAPIFeatures lhs, MMSFAPIFeatures rhs)
    {
        return static_cast<MMSFAPIFeatures>(static_cast<std::underlying_type_t<MMSFAPIFeatures>>(lhs) & static_cast<std::underlying_type_t<MMSFAPIFeatures>>(rhs));
    }
    constexpr MMSFAPIFeatures EmbedVersion(uint8_t ver)
    {
        return static_cast<MMSFAPIFeatures>(static_cast<std::underlying_type_t<MMSFAPIFeatures>>(ver) << 56);
    }
    constexpr uint8_t GetVersion(MMSFAPIFeatures features)
    {
        return static_cast<uint8_t>(static_cast<std::underlying_type_t<MMSFAPIFeatures>>(features) >> 56);
    }
    class IPluginService
    {
    public:
        virtual uint8_t GetVersion() = 0;
        virtual std::string GetName() = 0;
        virtual void Initialize() = 0;
        virtual rfl::Generic::Object Save() = 0;
        virtual void Load(rfl::Generic::Object) = 0;
    };
    //Service name "ALLOC"
    class IFormAllocator : public IPluginService
    {
    public:
        virtual RE::TESForm* AllocateForm(std::string, RE::FormType) = 0;
    };
    //Service name "EDID"
    class IEDIDCache : public IPluginService
    {
    public:
        virtual RE::FormID LookupEdid(std::string) = 0;
        virtual std::string LookupFormID(RE::FormID) = 0;
        virtual RE::TESForm* LookupCachedForm(std::string) = 0;
        virtual void CacheForm(std::string, RE::FormID) = 0;
    };

    class Interface
    {
    public:
        virtual MMSFAPIFeatures GetVersion() = 0;
        virtual void RegisterService(IPluginService*) = 0;
        virtual IPluginService* QueryService(std::string) = 0;
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
    [[nodiscard]] inline Interface* RequestMMSFAPI()
    {
        MMSFMessage message{};
        if (auto* messaging = SKSE::GetMessagingInterface())
        {
            messaging->Dispatch(
                MMSFMessage::kMessage_GetInterface,
                &message,
                sizeof(MMSFMessage),
                sender);
        }
        return message.API;
    }
}  // namespace MPL::API::MMSF
