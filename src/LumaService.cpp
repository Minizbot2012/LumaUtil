#include <CompatibilityChecker.h>
#include <LumaAPI.h>
#include <LumaService.h>
#include <algorithm>
#include <exception>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace MPL::LumaService
{
    namespace
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

        std::mutex callbackLock;
        std::vector<RegisteredClient> callbacks;

        bool SameClient(
            const RegisteredClient& a_left,
            const RegisteredClient& a_right)
        {
            return a_left.OnCellInitialized == a_right.OnCellInitialized &&
                   a_left.OnReferenceInitialized ==
                       a_right.OnReferenceInitialized &&
                   a_left.OnCellChanging == a_right.OnCellChanging &&
                   a_left.OnCellChanged == a_right.OnCellChanged &&
                   a_left.OnCellPatched == a_right.OnCellPatched;
        }

        bool RegisterClient(const LumaAPI::ClientCallbacks* a_callbacks)
        {
            if (!a_callbacks || !a_callbacks->id || !*a_callbacks->id)
            {
                return false;
            }
            try
            {
                const RegisteredClient client{
                    .id = a_callbacks->id,
                    .OnCellInitialized = a_callbacks->OnCellInitialized,
                    .OnReferenceInitialized =
                        a_callbacks->OnReferenceInitialized,
                    .OnCellChanging = a_callbacks->OnCellChanging,
                    .OnCellChanged = a_callbacks->OnCellChanged,
                    .OnCellPatched = a_callbacks->OnCellPatched,
                };
                std::scoped_lock lock(callbackLock);
                if (std::ranges::none_of(
                        callbacks,
                        [&](const auto& a_registered)
                        {
                            return SameClient(a_registered, client);
                        }))
                {
                    callbacks.push_back(client);
                }
                return true;
            }
            catch (const std::exception& error)
            {
                logger::error(
                    "[Luma API] Could not register client '{}': {}",
                    a_callbacks->id,
                    error.what());
            }
            catch (...)
            {
                logger::error(
                    "[Luma API] Could not register client '{}' because it raised an unknown exception",
                    a_callbacks->id);
            }
            return false;
        }

        std::vector<RegisteredClient> GetCallbacks()
        {
            std::scoped_lock lock(callbackLock);
            return callbacks;
        }

        template <class Callback>
        void NotifyClients(
            const std::string_view a_event,
            Callback&& a_callback)
        {
            try
            {
                for (const auto& client : GetCallbacks())
                {
                    try
                    {
                        a_callback(client);
                    }
                    catch (const std::exception& error)
                    {
                        logger::error(
                            "[Luma API] Client '{}' raised an exception during {}: {}",
                            client.id,
                            a_event,
                            error.what());
                    }
                    catch (...)
                    {
                        logger::error(
                            "[Luma API] Client '{}' raised an unknown exception during {}",
                            client.id,
                            a_event);
                    }
                }
            }
            catch (const std::exception& error)
            {
                logger::error(
                    "[Luma API] Could not dispatch {}: {}",
                    a_event,
                    error.what());
            }
            catch (...)
            {
                logger::error(
                    "[Luma API] Could not dispatch {} because an unknown exception occurred",
                    a_event);
            }
        }

        bool GetProviderSettings(
            const char* a_id,
            bool* a_detailedLogging,
            bool* a_notifications)
        {
            try
            {
                if (!a_id || !*a_id)
                {
                    return false;
                }
                const auto settings =
                    CompatibilityChecker::LoadManifest(a_id);
                if (!settings)
                {
                    return false;
                }
                if (a_detailedLogging)
                {
                    *a_detailedLogging = settings->detailedLogging;
                }
                if (a_notifications)
                {
                    *a_notifications = settings->notifications;
                }
                return true;
            }
            catch (const std::exception& error)
            {
                logger::error(
                    "[Luma API] GetProviderSettings failed: {}",
                    error.what());
            }
            catch (...)
            {
                logger::error(
                    "[Luma API] GetProviderSettings failed with an unknown exception");
            }
            return false;
        }

        bool UpdateProviderSettings(
            const char* a_id,
            const std::int8_t a_detailedLogging,
            const std::int8_t a_notifications)
        {
            try
            {
                if (!a_id || !*a_id)
                {
                    return false;
                }
                const auto detailed =
                    a_detailedLogging < 0 ?
                        std::nullopt :
                        std::optional<bool>(a_detailedLogging != 0);
                const auto notifications =
                    a_notifications < 0 ?
                        std::nullopt :
                        std::optional<bool>(a_notifications != 0);
                return CompatibilityChecker::UpdateManifestSettings(
                    a_id,
                    detailed,
                    notifications);
            }
            catch (const std::exception& error)
            {
                logger::error(
                    "[Luma API] UpdateProviderSettings failed: {}",
                    error.what());
            }
            catch (...)
            {
                logger::error(
                    "[Luma API] UpdateProviderSettings failed with an unknown exception");
            }
            return false;
        }

        const LumaAPI::Interface api{
            .version = LumaAPI::kVersion,
            .RegisterClient = RegisterClient,
            .GetProviderSettings = GetProviderSettings,
            .UpdateProviderSettings = UpdateProviderSettings,
        };
    }  // namespace

    void NotifyCellInitialized(RE::TESObjectCELL* a_cell)
    {
        NotifyClients("OnCellInitialized", [&](const auto& client) {
            if (client.OnCellInitialized)
            {
                client.OnCellInitialized(a_cell);
            }
        });
    }

    void NotifyReferenceInitialized(RE::TESObjectREFR* a_reference)
    {
        NotifyClients("OnReferenceInitialized", [&](const auto& client) {
            if (client.OnReferenceInitialized)
            {
                client.OnReferenceInitialized(a_reference);
            }
        });
    }

    void NotifyCellChanging(RE::TESObjectCELL* a_cell)
    {
        NotifyClients("OnCellChanging", [&](const auto& client) {
            if (client.OnCellChanging)
            {
                client.OnCellChanging(a_cell);
            }
        });
    }

    void NotifyCellChanged(const RE::TESObjectCELL* a_cell)
    {
        NotifyClients("OnCellChanged", [&](const auto& client) {
            if (client.OnCellChanged)
            {
                client.OnCellChanged(a_cell);
            }
        });
    }

    void NotifyCellPatched(
        RE::TESObjectCELL* a_cell,
        const char* a_provider,
        const bool a_hasSkylight)
    {
        NotifyClients("OnCellPatched", [&](const auto& client) {
            if (client.OnCellPatched)
            {
                client.OnCellPatched(
                    a_cell,
                    a_provider,
                    a_hasSkylight);
            }
        });
    }

    const LumaAPI::Interface* GetAPI()
    {
        return std::addressof(api);
    }
}  // namespace MPL::LumaService

extern "C" __declspec(dllexport) const MPL::LumaAPI::Interface* LumaUtil_RequestAPI(const std::uint32_t a_version)
{
    return a_version == MPL::LumaAPI::kVersion ?
               MPL::LumaService::GetAPI() :
               nullptr;
}
