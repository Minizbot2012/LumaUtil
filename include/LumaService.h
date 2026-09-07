#pragma once

#include <LumaAPI.h>

namespace MPL::LumaService
{
    struct RegisteredClient
    {
        std::string id;
        void (*OnCellInitialized)(RE::TESObjectCELL*) = nullptr;
        void (*OnReferenceInitialized)(RE::TESObjectREFR*) = nullptr;
        void (*OnCellChanging)(RE::TESObjectCELL*) = nullptr;
        void (*OnCellChanged)(const RE::TESObjectCELL*) = nullptr;
        void (*OnCellPatched)(
            RE::TESObjectCELL*,
            const char*,
            bool) = nullptr;
    };
    class LumaService : public MPL::API::Luma::ILumaPluginService
    {
    private:
        std::mutex callbackLock;
        std::vector<RegisteredClient> callbacks;

    public:
        static LumaService* GetSingleton()
        {
            static LumaService instance;
            return &instance;
        }
        std::uint8_t GetVersion() override { return MPL::API::Luma::kVersion; }
        std::string GetName() override { return "LUMA"; }
        void Initialize() override {}
        rfl::Generic::Object Save() override { return rfl::Generic::Object(); }
        void Load(rfl::Generic::Object) override {}
        std::vector<RegisteredClient> GetCallbacks();
        bool RegisterClient(const MPL::API::Luma::ClientCallbacks*) override;
        bool GetProviderSettings(const char*, bool*, bool*) override;
        bool UpdateProviderSettings(const char*, std::int8_t, std::int8_t) override;

    };
    void NotifyCellInitialized(RE::TESObjectCELL*);
    void NotifyReferenceInitialized(RE::TESObjectREFR*);
    void NotifyCellChanging(RE::TESObjectCELL*);
    void NotifyCellChanged(const RE::TESObjectCELL*);
    void NotifyCellPatched(RE::TESObjectCELL*, const char*, bool);
}  // namespace MPL::LumaService
