#pragma once

#include <CompatibilityManifest.h>
#include <optional>
#include <string_view>

namespace MPL::CompatibilityChecker
{
    std::optional<Compatibility::Manifest> LoadManifest(std::string_view);
    bool UpdateManifestSettings(
        std::string_view a_id,
        std::optional<bool> a_detailedLogging,
        std::optional<bool> a_notifications);
    void Initialize();
}  // namespace MPL::CompatibilityChecker
