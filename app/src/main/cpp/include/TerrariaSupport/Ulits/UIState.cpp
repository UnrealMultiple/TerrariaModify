#include "UIState.hpp"

namespace UIState {
    template<class T>
    T& getPanelState() {
        static T state;
        return state;
    }

    template PlayerState& getPanelState<PlayerState>();
    template PanelState& getPanelState<PanelState>();
    template ItemState& getPanelState<ItemState>();
    template FishUIState& getPanelState<FishUIState>();
    template NPCState& getPanelState<NPCState>();
    template WolldState& getPanelState<WolldState>();
}