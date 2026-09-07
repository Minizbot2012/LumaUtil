#pragma once

#include <Externals/MMSF_API.h>
#include <cstdint>
namespace RE
{
    class TESObjectCELL;
    class TESObjectREFR;
}  // namespace RE

namespace MPL::API::Luma
{
    inline constexpr std::uint8_t kVersion = 4;
    struct ClientCallbacks
    {
        // The ID is copied during registration.
        const char* id = nullptr;
        void (*OnCellInitialized)(RE::TESObjectCELL*) = nullptr;
        void (*OnReferenceInitialized)(RE::TESObjectREFR*) = nullptr;
        void (*OnCellChanging)(RE::TESObjectCELL*) = nullptr;
        void (*OnCellChanged)(const RE::TESObjectCELL*) = nullptr;
        // The provider string is borrowed and valid only during the callback.
        void (*OnCellPatched)(
            RE::TESObjectCELL*,
            const char*,
            bool) = nullptr;
    };

    class ILumaPluginService : public API::MMSF::IPluginService
    {
    public:
        virtual bool RegisterClient(const ClientCallbacks*) = 0;
        virtual bool GetProviderSettings(const char*, bool*, bool*) = 0;
        virtual bool UpdateProviderSettings(const char*, std::int8_t, std::int8_t) = 0;
    };
}  // namespace MPL::API::Luma
