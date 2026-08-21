#include <LPCommand.h>
#include <optional>
#include <string_view>

namespace MPL::LPCommand
{
    namespace
    {
        constexpr std::string_view kCommand = "ToggleBoundVisGeom";

        struct State
        {
            RE::SCRIPT_FUNCTION* slot = nullptr;
            std::optional<RE::SCRIPT_FUNCTION> original;
        };

        State& GetState()
        {
            static State state;
            return state;
        }
    }  // namespace

    void CaptureCommandSlot(const bool a_logFailure)
    {
        auto& state = GetState();
        if (state.original)
        {
            return;
        }

        state.slot = RE::SCRIPT_FUNCTION::LocateConsoleCommand(kCommand);
        if (state.slot)
        {
            state.original = *state.slot;
        }
        else if (a_logFailure)
        {
            logger::warn(
                "[Compatibility] Console command slot unavailable | slot={}",
                kCommand);
        }
    }

    void ReleaseCommandSlot()
    {
        auto& state = GetState();
        if (state.slot && state.original)
        {
            *state.slot = *state.original;
        }
    }
}  // namespace MPL::LPCommand
