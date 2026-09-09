#pragma once
#include <cstdint>
#include <cstring>

struct AppFlags {
    bool use_lcd = true;
    bool test_hc595 = false;
    bool test_lcd = false;
};

inline bool parseAppArg(const char* arg, AppFlags& flags) {
    if (std::strcmp(arg, "--no-lcd") == 0) {
        flags.use_lcd = false;
        return true;
    }
    if (std::strcmp(arg, "--test-hc595") == 0) {
        flags.test_hc595 = true;
        return true;
    }
    if (std::strcmp(arg, "--test-lcd") == 0) {
        flags.test_lcd = true;
        return true;
    }
    return false;
}

inline bool parseAppFlags(int argc, char* const argv[], AppFlags& flags) {
    for (int i = 1; i < argc; ++i) {
        if (!parseAppArg(argv[i], flags)) {
            return false;
        }
    }
    return true;
}

// Full dashboard (no exclusive flags) or exactly one of --no-lcd / --test-hc595 / --test-lcd.
inline bool appFlagsMutuallyExclusive(const AppFlags& flags) {
    const int modes = (flags.test_hc595 ? 1 : 0) + (flags.test_lcd ? 1 : 0) +
                      (!flags.use_lcd ? 1 : 0);
    return modes <= 1;
}

inline bool stalePotShouldWarn(int samples, uint8_t min_v, uint8_t max_v, int threshold,
                               bool already_warned) {
    return !already_warned && samples >= threshold && min_v == max_v;
}
