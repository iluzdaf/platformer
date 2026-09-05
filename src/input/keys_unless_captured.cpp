#include "input/keys_unless_captured.hpp"
#include "input/keys_down.hpp"

KeysDown keysUnlessCaptured(KeysDown keys, bool captured)
{
    if (captured)
        return [](int) { return false; };

    return keys;
}
