#pragma once

#include <optional>
#include <string>
#include <vector>

namespace MPL::Compatibility
{
    struct Requirement
    {
        std::string id;
        std::string minimumVersion;
        std::optional<std::string> maximumVersion;
        std::string notification;
    };

    struct Manifest
    {
        std::string id;
        bool LumaManifest = false;
        bool detailedLogging = false;
        bool notifications = true;
        std::string version;
        std::vector<Requirement> compatibility;
    };
}  // namespace MPL::Compatibility
