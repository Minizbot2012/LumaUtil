#include <CompatibilityChecker.h>
#include <CompatibilityManifest.h>
#include <JsonFormatting.h>
#include <algorithm>
#include <charconv>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#include <yyjson.h>

namespace MPL::CompatibilityChecker
{
    namespace
    {
        const std::filesystem::path kSettingsRoot{ "./Data/SKSE/Plugins" };
        constexpr std::string_view kSettingsSuffix = "settings.json";

        struct Version
        {
            std::vector<std::uint32_t> parts;
        };

        struct JsonDocumentDeleter
        {
            void operator()(yyjson_doc* a_document) const
            {
                yyjson_doc_free(a_document);
            }
        };

        struct MutableJsonDocumentDeleter
        {
            void operator()(yyjson_mut_doc* a_document) const
            {
                yyjson_mut_doc_free(a_document);
            }
        };

        using JsonDocument =
            std::unique_ptr<yyjson_doc, JsonDocumentDeleter>;
        using MutableJsonDocument =
            std::unique_ptr<
                yyjson_mut_doc,
                MutableJsonDocumentDeleter>;

        struct LoadedManifest
        {
            Compatibility::Manifest settings;
            std::filesystem::path source;
        };

        struct Issue
        {
            std::string detail;
            std::string notification;
        };

        struct State
        {
            std::vector<LoadedManifest> manifests;
            std::vector<Issue> issues;
            bool initialized = false;
            bool mainMenuNotified = false;
        };

        State& GetState()
        {
            static State state;
            return state;
        }

        std::string Lower(std::string_view a_value)
        {
            std::string result(a_value);
            std::ranges::transform(
                result,
                result.begin(),
                [](const unsigned char a_character) {
                    return static_cast<char>(std::tolower(a_character));
                });
            return result;
        }

        bool EndsWithIgnoreCase(
            const std::string_view a_value,
            const std::string_view a_suffix)
        {
            return a_value.size() >= a_suffix.size() &&
                   Lower(a_value.substr(a_value.size() - a_suffix.size())) ==
                       Lower(a_suffix);
        }

        std::optional<Version> ParseVersion(const std::string_view a_value)
        {
            if (a_value.empty())
            {
                return std::nullopt;
            }

            Version version;
            std::size_t begin = 0;
            while (begin < a_value.size())
            {
                const auto end = a_value.find('.', begin);
                const auto part = a_value.substr(
                    begin,
                    end == std::string_view::npos ?
                        a_value.size() - begin :
                        end - begin);
                if (part.empty())
                {
                    return std::nullopt;
                }

                std::uint32_t value = 0;
                const auto parsed =
                    std::from_chars(part.data(), part.data() + part.size(), value);
                if (parsed.ec != std::errc{} ||
                    parsed.ptr != part.data() + part.size())
                {
                    return std::nullopt;
                }
                version.parts.push_back(value);

                if (end == std::string_view::npos)
                {
                    break;
                }
                begin = end + 1;
            }
            return version.parts.empty() ?
                       std::nullopt :
                       std::optional<Version>(std::move(version));
        }

        int Compare(const Version& a_left, const Version& a_right)
        {
            const auto count =
                std::max(a_left.parts.size(), a_right.parts.size());
            for (std::size_t index = 0; index < count; ++index)
            {
                const auto left =
                    index < a_left.parts.size() ? a_left.parts[index] : 0;
                const auto right =
                    index < a_right.parts.size() ? a_right.parts[index] : 0;
                if (left != right)
                {
                    return left < right ? -1 : 1;
                }
            }
            return 0;
        }

        std::string ReadText(const std::filesystem::path& a_path)
        {
            std::ifstream file(a_path, std::ios::binary);
            return file ?
                       std::string(
                           std::istreambuf_iterator<char>(file),
                           std::istreambuf_iterator<char>()) :
                       std::string{};
        }

        std::optional<std::pair<Compatibility::Manifest, std::filesystem::path>>
        FindManifest(const std::string_view a_id)
        {
            if (a_id.empty())
            {
                return std::nullopt;
            }

            std::optional<std::pair<
                Compatibility::Manifest,
                std::filesystem::path>> found;
            std::error_code error;
            for (std::filesystem::directory_iterator iterator(
                     kSettingsRoot,
                     std::filesystem::directory_options::skip_permission_denied,
                     error),
                 end;
                 iterator != end && !error;
                 iterator.increment(error))
            {
                const auto filename = iterator->path().filename().string();
                if (!iterator->is_regular_file(error) ||
                    !EndsWithIgnoreCase(filename, kSettingsSuffix))
                {
                    continue;
                }

                const auto parsed =
                    rfl::json::read<
                        Compatibility::Manifest,
                        rfl::DefaultIfMissing>(ReadText(iterator->path()));
                if (!parsed)
                {
                    logger::warn(
                        "[Compatibility] Could not read settings manifest {}: {}",
                        iterator->path().string(),
                        parsed.error().what());
                    continue;
                }
                if (!parsed.value().LumaManifest ||
                    Lower(parsed.value().id) != Lower(a_id))
                {
                    continue;
                }
                if (found)
                {
                    logger::error(
                        "[Compatibility] Component ID '{}' is duplicated by {} and {}; refusing ambiguous settings access",
                        a_id,
                        found->second.filename().string(),
                        iterator->path().filename().string());
                    return std::nullopt;
                }
                found = std::pair{
                        parsed.value(),
                        iterator->path() };
            }
            return found;
        }

        bool WriteManifestText(
            const std::filesystem::path& a_path,
            const std::string_view a_text)
        {
            auto temporary = a_path;
            temporary += ".tmp";
            {
                std::ofstream file(
                    temporary,
                    std::ios::binary | std::ios::trunc);
                if (!file)
                {
                    logger::error(
                        "[Compatibility] Could not open temporary settings file {}",
                        temporary.string());
                    return false;
                }
                file << a_text << '\n';
                file.flush();
                if (!file)
                {
                    logger::error(
                        "[Compatibility] Could not write temporary settings file {}",
                        temporary.string());
                    return false;
                }
            }

            std::error_code error;
            std::filesystem::copy_file(
                temporary,
                a_path,
                std::filesystem::copy_options::overwrite_existing,
                error);
            std::filesystem::remove(temporary);
            if (error)
            {
                logger::error(
                    "[Compatibility] Could not replace settings file {}: {}",
                    a_path.string(),
                    error.message());
                return false;
            }
            return true;
        }

        bool UpdateManifestBooleans(
            const std::filesystem::path& a_path,
            const std::optional<bool> a_detailedLogging,
            const std::optional<bool> a_notifications)
        {
            const auto source = ReadText(a_path);
            JsonDocument document(
                yyjson_read(source.data(), source.size(), 0));
            auto* sourceRoot = document ?
                                   yyjson_doc_get_root(
                                       document.get()) :
                                   nullptr;
            if (!yyjson_is_obj(sourceRoot))
            {
                logger::error(
                    "[Compatibility] Could not preserve settings in {} because its root is not an object",
                    a_path.string());
                return false;
            }

            MutableJsonDocument result(
                yyjson_mut_doc_new(nullptr));
            auto* root = result ?
                             yyjson_val_mut_copy(
                                 result.get(),
                                 sourceRoot) :
                             nullptr;
            if (!root)
            {
                return false;
            }
            const auto putBoolean =
                [&](const char* a_key, const bool a_value)
            {
                auto* key =
                    yyjson_mut_strcpy(result.get(), a_key);
                auto* value =
                    yyjson_mut_bool(result.get(), a_value);
                return key && value &&
                       yyjson_mut_obj_put(root, key, value);
            };
            if ((a_detailedLogging &&
                    !putBoolean(
                        "detailedLogging",
                        *a_detailedLogging)) ||
                (a_notifications &&
                    !putBoolean(
                        "notifications",
                        *a_notifications)))
            {
                return false;
            }
            yyjson_mut_doc_set_root(result.get(), root);

            std::size_t length = 0;
            auto* data = yyjson_mut_write(
                result.get(),
                0,
                std::addressof(length));
            if (!data)
            {
                return false;
            }
            const std::string compact(data, length);
            std::free(data);
            return WriteManifestText(
                a_path,
                JsonFormatting::Pretty(compact));
        }

        void LoadManifests()
        {
            auto& state = GetState();
            state.manifests.clear();
            state.issues.clear();

            std::error_code error;
            std::vector<std::filesystem::path> files;
            for (std::filesystem::directory_iterator iterator(
                     kSettingsRoot,
                     std::filesystem::directory_options::skip_permission_denied,
                     error),
                 end;
                 iterator != end && !error;
                 iterator.increment(error))
            {
                const auto filename =
                    iterator->path().filename().string();
                if (iterator->is_regular_file(error) &&
                    EndsWithIgnoreCase(filename, kSettingsSuffix))
                {
                    files.push_back(iterator->path());
                }
            }
            std::ranges::sort(files);

            for (const auto& file : files)
            {
                const auto parsed =
                    rfl::json::read<
                        Compatibility::Manifest,
                        rfl::DefaultIfMissing>(ReadText(file));
                if (!parsed)
                {
                    logger::warn(
                        "[Compatibility] Could not read settings manifest {}: {}",
                        file.string(),
                        parsed.error().what());
                    continue;
                }
                auto settings = parsed.value();
                if (!settings.LumaManifest)
                {
                    continue;
                }
                if (settings.id.empty() || settings.version.empty())
                {
                    logger::warn(
                        "[Compatibility] Ignoring settings manifest {} because id or version is empty",
                        file.string());
                    continue;
                }

                logger::info(
                    "[Compatibility] Loaded '{}' version {} from {} with {} compatibility requirement(s)",
                    settings.id,
                    settings.version,
                    file.string(),
                    settings.compatibility.size());
                state.manifests.push_back(
                    { std::move(settings), file });
            }
        }

        std::string RequirementText(
            const Compatibility::Requirement& a_requirement)
        {
            if (a_requirement.maximumVersion)
            {
                return std::format(
                    "{} through {}",
                    a_requirement.minimumVersion,
                    *a_requirement.maximumVersion);
            }
            return std::format(
                "{} or newer",
                a_requirement.minimumVersion);
        }

        void AddIssue(
            std::string a_detail,
            std::string a_notification = {})
        {
            GetState().issues.push_back(
                { std::move(a_detail), std::move(a_notification) });
        }

        void EvaluateCompatibility()
        {
            auto& state = GetState();
            std::map<std::string, const LoadedManifest*> byID;
            for (const auto& manifest : state.manifests)
            {
                const auto key = Lower(manifest.settings.id);
                if (const auto [existing, inserted] =
                        byID.emplace(key, std::addressof(manifest));
                    !inserted)
                {
                    AddIssue(std::format(
                        "Duplicate component ID '{}' was declared by {} and {}.",
                        manifest.settings.id,
                        existing->second->source.filename().string(),
                        manifest.source.filename().string()));
                }
            }

            for (const auto& manifest : state.manifests)
            {
                const auto sourceVersion =
                    ParseVersion(manifest.settings.version);
                if (!sourceVersion)
                {
                    AddIssue(std::format(
                        "{} declares an invalid version '{}'.",
                        manifest.settings.id,
                        manifest.settings.version));
                }

                for (const auto& requirement :
                    manifest.settings.compatibility)
                {
                    if (requirement.id.empty() ||
                        requirement.minimumVersion.empty())
                    {
                        AddIssue(
                            std::format(
                                "{} contains an incomplete compatibility entry.",
                                manifest.settings.id),
                            requirement.notification);
                        continue;
                    }

                    const auto target = byID.find(Lower(requirement.id));
                    if (target == byID.end())
                    {
                        AddIssue(
                            std::format(
                                "{} {} requires {} {}, but its settings file was not found.",
                                manifest.settings.id,
                                manifest.settings.version,
                                requirement.id,
                                RequirementText(requirement)),
                            requirement.notification);
                        continue;
                    }

                    const auto installed =
                        ParseVersion(target->second->settings.version);
                    const auto minimum =
                        ParseVersion(requirement.minimumVersion);
                    const auto maximum =
                        requirement.maximumVersion ?
                            ParseVersion(*requirement.maximumVersion) :
                            std::optional<Version>{};
                    if (!installed || !minimum ||
                        (requirement.maximumVersion && !maximum) ||
                        (maximum && Compare(*minimum, *maximum) > 0))
                    {
                        AddIssue(
                            std::format(
                                "{} contains an invalid compatibility range for {}.",
                                manifest.settings.id,
                                requirement.id),
                            requirement.notification);
                        continue;
                    }

                    const bool below = Compare(*installed, *minimum) < 0;
                    const bool above =
                        maximum && Compare(*installed, *maximum) > 0;
                    if (below || above)
                    {
                        AddIssue(
                            std::format(
                                "{} {} requires {} {}; installed version is {}.",
                                manifest.settings.id,
                                manifest.settings.version,
                                requirement.id,
                                RequirementText(requirement),
                                target->second->settings.version),
                            requirement.notification);
                    }
                    else
                    {
                        logger::info(
                            "[Compatibility] {} {} accepts {} {}",
                            manifest.settings.id,
                            manifest.settings.version,
                            target->second->settings.id,
                            target->second->settings.version);
                    }
                }
            }

            if (state.issues.empty())
            {
                logger::info(
                    "[Compatibility] {} component manifest(s) passed compatibility checks",
                    state.manifests.size());
            }
            else
            {
                logger::warn(
                    "[Compatibility] Found {} compatibility issue(s) across {} component manifest(s)",
                    state.issues.size(),
                    state.manifests.size());
                for (const auto& issue : state.issues)
                {
                    logger::warn("[Compatibility] {}", issue.detail);
                }
            }
        }

        void QueueNotifications()
        {
            const auto& issues = GetState().issues;
            if (issues.empty())
            {
                return;
            }

            for (const auto& issue : issues)
            {
                std::string message =
                    "Luma compatibility warning\n\n";
                message.append(
                    issue.notification.empty() ?
                        issue.detail :
                        issue.notification);
                message.append(
                    "\n\nReview LumaUtil.log for additional information.");

                if (auto* tasks = SKSE::GetTaskInterface())
                {
                    tasks->AddTask([message = std::move(message)]() {
                        RE::DebugMessageBox(message.c_str());
                    });
                }
                else
                {
                    RE::DebugMessageBox(message.c_str());
                }
            }
        }

        class MainMenuSink final :
            public RE::BSTEventSink<RE::MenuOpenCloseEvent>
        {
        public:
            RE::BSEventNotifyControl ProcessEvent(
                const RE::MenuOpenCloseEvent* a_event,
                RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
            {
                if (!a_event ||
                    a_event->menuName != RE::MainMenu::MENU_NAME)
                {
                    return RE::BSEventNotifyControl::kContinue;
                }

                auto& state = GetState();
                if (!a_event->opening)
                {
                    state.mainMenuNotified = false;
                }
                else if (!state.mainMenuNotified)
                {
                    state.mainMenuNotified = true;
                    QueueNotifications();
                }
                return RE::BSEventNotifyControl::kContinue;
            }

            static MainMenuSink& GetSingleton()
            {
                static MainMenuSink singleton;
                return singleton;
            }
        };
    }  // namespace

    std::optional<Compatibility::Manifest> LoadManifest(
        const std::string_view a_id)
    {
        const auto manifest = FindManifest(a_id);
        return manifest ?
                   std::optional<Compatibility::Manifest>(
                       manifest->first) :
                   std::nullopt;
    }

    bool UpdateManifestSettings(
        const std::string_view a_id,
        const std::optional<bool> a_detailedLogging,
        const std::optional<bool> a_notifications)
    {
        auto manifest = FindManifest(a_id);
        if (!manifest)
        {
            logger::error(
                "[Compatibility] Could not update settings for '{}': manifest not found",
                a_id);
            return false;
        }
        if (a_detailedLogging)
        {
            manifest->first.detailedLogging = *a_detailedLogging;
        }
        if (a_notifications)
        {
            manifest->first.notifications = *a_notifications;
        }
        if (!UpdateManifestBooleans(
                manifest->second,
                a_detailedLogging,
                a_notifications))
        {
            return false;
        }
        logger::info(
            "[Compatibility] Updated runtime settings for '{}' in {}",
            manifest->first.id,
            manifest->second.string());
        return true;
    }

    void Initialize()
    {
        auto& state = GetState();
        if (state.initialized)
        {
            return;
        }
        state.initialized = true;

        LoadManifests();
        EvaluateCompatibility();

        if (auto* ui = RE::UI::GetSingleton())
        {
            ui->AddEventSink(std::addressof(
                MainMenuSink::GetSingleton()));
            if (ui->IsMenuOpen(RE::MainMenu::MENU_NAME))
            {
                state.mainMenuNotified = true;
                QueueNotifications();
            }
        }
        else
        {
            logger::warn(
                "[Compatibility] Could not register the main-menu notification because the UI is unavailable");
        }
    }
}  // namespace MPL::CompatibilityChecker
