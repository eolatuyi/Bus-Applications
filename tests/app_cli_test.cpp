#include "app_cli.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

namespace {
bool parseArgs(const std::vector<std::string>& args, AppFlags& flags) {
    std::vector<char*> argv;
    argv.push_back(const_cast<char*>("app"));
    for (const auto& a : args) {
        argv.push_back(const_cast<char*>(a.c_str()));
    }
    return parseAppFlags(static_cast<int>(argv.size()), argv.data(), flags);
}
}  // namespace

int main() {
    {
        AppFlags f;
        assert(parseArgs({}, f));
        assert(f.use_lcd && !f.test_hc595 && !f.test_lcd);
        assert(appFlagsMutuallyExclusive(f));
    }
    {
        AppFlags f;
        assert(parseArgs({"--no-lcd"}, f));
        assert(!f.use_lcd);
        assert(appFlagsMutuallyExclusive(f));
    }
    {
        AppFlags f;
        assert(parseArgs({"--test-hc595"}, f));
        assert(f.test_hc595 && f.use_lcd);
        assert(appFlagsMutuallyExclusive(f));
    }
    {
        AppFlags f;
        assert(parseArgs({"--test-lcd"}, f));
        assert(f.test_lcd);
        assert(appFlagsMutuallyExclusive(f));
    }
    {
        AppFlags f;
        assert(parseArgs({"--no-lcd", "--test-hc595"}, f));
        assert(!appFlagsMutuallyExclusive(f));
    }
    {
        AppFlags f;
        assert(parseArgs({"--test-lcd", "--test-hc595"}, f));
        assert(!appFlagsMutuallyExclusive(f));
    }
    {
        AppFlags f;
        assert(parseArgs({"--no-lcd", "--test-lcd"}, f));
        assert(!appFlagsMutuallyExclusive(f));
    }
    {
        AppFlags f;
        assert(!parseArgs({"--bogus"}, f));
    }

    assert(!stalePotShouldWarn(24, 10, 10, 25, false));
    assert(stalePotShouldWarn(25, 10, 10, 25, false));
    assert(!stalePotShouldWarn(25, 10, 11, 25, false));
    assert(!stalePotShouldWarn(25, 10, 10, 25, true));

    std::cout << "app_cli_test: OK\n";
    return 0;
}
