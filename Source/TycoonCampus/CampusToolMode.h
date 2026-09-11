#pragma once

enum class ECampusToolMode { Selection, Construction, Paths, Planning, Menu };
enum class ECampusToolAction { Construction, Paths, Planning, Menu, Escape, CancelTools };

/** Deterministic navigation policy, independent of Unreal and of simulation rules. */
struct FCampusToolMode
{
    static bool IsModal(ECampusToolMode Mode)
    { return Mode == ECampusToolMode::Planning || Mode == ECampusToolMode::Menu; }

    static ECampusToolMode Next(ECampusToolMode Mode, ECampusToolAction Action)
    {
        using M = ECampusToolMode;
        using A = ECampusToolAction;
        switch (Action)
        {
        case A::Construction: return IsModal(Mode) ? Mode : Mode == M::Construction ? M::Selection : M::Construction;
        case A::Paths: return IsModal(Mode) ? Mode : Mode == M::Paths ? M::Selection : M::Paths;
        case A::Planning: return Mode == M::Menu ? Mode : Mode == M::Planning ? M::Selection : M::Planning;
        case A::Menu: return Mode == M::Menu ? M::Selection : M::Menu;
        case A::Escape: return Mode == M::Selection ? M::Menu : M::Selection;
        case A::CancelTools: return IsModal(Mode) ? Mode : M::Selection;
        }
        return Mode;
    }

    static bool AllowsWorldPress(ECampusToolMode Mode, bool OverTerrain, bool Rotating)
    { return !IsModal(Mode) && OverTerrain && !Rotating; }
};
