#include "../Source/TycoonCampus/CampusToolMode.h"
#include <iostream>
#include <stdexcept>

static void Check(bool Value, const char* Message) { if (!Value) { throw std::runtime_error(Message); } }
int main()
{
    try
    {
        using M = ECampusToolMode;
        using A = ECampusToolAction;
        const M Modes[] = {M::Selection, M::Construction, M::Paths, M::Planning, M::Menu};
        const A Actions[] = {A::Construction, A::Paths, A::Planning, A::Menu, A::Escape, A::CancelTools};
        const M Expected[5][6] = {
            {M::Construction,M::Paths,M::Planning,M::Menu,M::Menu,M::Selection},
            {M::Selection,M::Paths,M::Planning,M::Menu,M::Selection,M::Selection},
            {M::Construction,M::Selection,M::Planning,M::Menu,M::Selection,M::Selection},
            {M::Planning,M::Planning,M::Selection,M::Menu,M::Selection,M::Planning},
            {M::Menu,M::Menu,M::Menu,M::Selection,M::Selection,M::Menu}
        };
        for (int I=0; I<5; ++I)
        {
            for (int J=0; J<6; ++J) { Check(FCampusToolMode::Next(Modes[I],Actions[J])==Expected[I][J],"transition matrix"); }
            Check(!FCampusToolMode::AllowsWorldPress(Modes[I],false,false),"UI press reaches world");
            Check(!FCampusToolMode::AllowsWorldPress(Modes[I],true,true),"rotation press reaches world");
            Check(FCampusToolMode::AllowsWorldPress(Modes[I],true,false)==(I<3),"modal press reaches world");
        }
        M Mode=M::Paths;
        Mode=FCampusToolMode::Next(Mode,A::Escape);
        Check(Mode==M::Selection,"escape must cancel path tool before opening menu");
        Mode=FCampusToolMode::Next(Mode,A::Escape);
        Check(Mode==M::Menu,"second escape opens menu");
        Mode=FCampusToolMode::Next(Mode,A::Escape);
        Check(Mode==M::Selection,"menu closes without reactivating path tool");
        std::cout << "PASS: exclusive tool transitions, escape sequence, modal and UI pointer guards\n";
        return 0;
    }
    catch (const std::exception& Error) { std::cerr << Error.what() << '\n'; return 1; }
}
