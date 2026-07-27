#include <Hooking.h>
#include <Config.h>
#include <Hooks.h>
#include <LumaService.h>
#include <DetailedLogging.h>
namespace MPL::Hooks
{
    struct InitCell
    {
        using Target = RE::TESObjectCELL;
        static inline constexpr VariantIndex index = VariantIndex(0x13);
        static inline void thunk(Target* a_ref)
        {
            func(a_ref);
            if (auto* source = a_ref ? a_ref->GetFile(0) : nullptr)
            {
                DetailedLogging::Info(
                    "Loading Cell {:06X}:{}",
                    a_ref->GetLocalFormID(),
                    source->GetFilename());
                MPL::Config::LoadConfigFormID<MPL::Config::Cell::TESObjectCELL>(a_ref);
            }
            LumaService::NotifyCellInitialized(a_ref);
        }
        static void post_hook()
        {
            logger::info("Installed InitCell Hook");
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct InitIS
    {
        using Target = RE::TESImageSpace;
        static inline constexpr VariantIndex index = VariantIndex(0x13);
        static inline void thunk(Target* a_ref)
        {
            func(a_ref);
            if (auto* source = a_ref ? a_ref->GetFile(0) : nullptr)
            {
                DetailedLogging::Info(
                    "Loading IS {:06X}:{}",
                    a_ref->GetLocalFormID(),
                    source->GetFilename());
                MPL::Config::LoadConfigFormID<MPL::Config::ImageSpace::TESImageSpace>(a_ref);
            }
        }
        static void post_hook()
        {
            logger::info("Installed InitImagespace Hook");
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct InitREFR
    {
        using Target = RE::TESObjectREFR;
        static inline constexpr VariantIndex index = VariantIndex(0x13);
        static inline void thunk(Target* a_ref)
        {
            func(a_ref);
            if (auto* source = a_ref ? a_ref->GetFile(0) : nullptr)
            {
                DetailedLogging::Info(
                    "Loading ObjRef {:06X}:{}",
                    a_ref->GetLocalFormID(),
                    source->GetFilename());
                MPL::Config::LoadConfigFormID<MPL::Config::TESObjectREFR>(a_ref);
            }
            LumaService::NotifyReferenceInitialized(a_ref);
        }
        static void post_hook()
        {
            logger::info("Installed InitREFR Hook");
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct InitTMPL
    {
        using Target = RE::BGSLightingTemplate;
        static inline constexpr VariantIndex index = VariantIndex(0x13);
        static inline void thunk(Target* a_ref)
        {
            func(a_ref);
            if (auto* source = a_ref ? a_ref->GetFile(0) : nullptr)
            {
                DetailedLogging::Info(
                    "Loading TMPL {:06X}:{}",
                    a_ref->GetLocalFormID(),
                    source->GetFilename());
                MPL::Config::LoadConfigFormID<MPL::Config::Template::BGSLightingTemplate>(a_ref);
            }
        }
        static void post_hook()
        {
            logger::info("Installed InitTempl Hook");
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct InitLGHT
    {
        using Target = RE::TESObjectLIGH;
        static inline constexpr VariantIndex index = VariantIndex(0x13);
        static inline void thunk(Target* a_ref)
        {
            func(a_ref);
            if (auto* source = a_ref ? a_ref->GetFile(0) : nullptr)
            {
                DetailedLogging::Info(
                    "Loading Light {:06X}:{}",
                    a_ref->GetLocalFormID(),
                    source->GetFilename());
                MPL::Config::LoadConfigFormID<MPL::Config::TESObjectLIGH>(a_ref);
            }
        }
        static void post_hook()
        {
            logger::info("Installed InitLGHT Hook");
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct InitWorldspace
    {
        using Target = RE::TESWorldSpace;
        static inline constexpr VariantIndex index = VariantIndex(0x13);
        static inline void thunk(Target* a_ref)
        {
            func(a_ref);
            if (auto* source = a_ref ? a_ref->GetFile(0) : nullptr)
            {
                DetailedLogging::Info(
                    "Loading WorldSpace {:06X}:{}",
                    a_ref->GetLocalFormID(),
                    source->GetFilename());
                Config::LoadConfigFormID<MPL::Config::Worldspace::TESWorldSpace>(a_ref);
            }
        }
        static void post_hook()
        {
            logger::info("Installed InitWorldspace Hook");
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct CellChange
    {
        using Target = RE::PlayerCharacter;
        static inline constexpr VariantIndex index = VariantIndex(0x98, 0x98, 0x99);
        static inline void thunk(RE::PlayerCharacter* a_ref, const RE::TESObjectCELL* cl)
        {
            auto* destination = const_cast<RE::TESObjectCELL*>(cl);
            LumaService::NotifyCellChanging(destination);
            func(a_ref, cl);
            if (cl != nullptr)
            {
                MPL::Config::StatData::GetSingleton()->cellLoad.QueueEvent(cl);
            }
            LumaService::NotifyCellChanged(cl);
        }
        static void post_hook()
        {
            logger::info("Installed CellChange Hook");
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
    void Install()
    {
        stl::install_hook<InitCell>();
        stl::install_hook<InitIS>();
        stl::install_hook<InitREFR>();
        stl::install_hook<InitTMPL>();
        stl::install_hook<InitLGHT>();
        stl::install_hook<InitWorldspace>();
        stl::install_hook<CellChange>();
    }
}  // namespace MPL::Hooks
