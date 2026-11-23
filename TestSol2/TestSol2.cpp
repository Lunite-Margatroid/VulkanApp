#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

int main() {

    sol::state lua;
    int x = 0;
    lua.set_function("beep", [&x] { ++x; });
    lua.script("beep()");
    assert(x == 1);

	return 0;
}