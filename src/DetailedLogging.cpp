#include <DetailedLogging.h>
#include <atomic>

namespace MPL::DetailedLogging
{
    namespace
    {
        std::atomic_bool enabled{ false };
    }  // namespace

    void SetEnabled(const bool a_enabled)
    {
        enabled.store(a_enabled, std::memory_order_relaxed);
    }

    bool IsEnabled()
    {
        return enabled.load(std::memory_order_relaxed);
    }
}  // namespace MPL::DetailedLogging
