#include <fstream>
#include <print>

#include "settings.h"

using namespace std;
using nlohmann::json;

void Settings::save() const
{
    json j = *this;

    // Attach the breakpoints

    json json_sets = json::array();
    for (const auto& [game, breakpoints] : breakpoints)
    {
        json json_breakpoints = json::array();
        for (const Breakpoint& breakpoint : breakpoints)
        {
            json_breakpoints.push_back({{"enabled", breakpoint.enabled}, {"address", breakpoint.address}});
        }

        json_sets.push_back({{"game", game}, {"breakpoints", json_breakpoints}});
    }
    j["breakpoint_sets"] = json_sets;

    // Attach the current version

    j["version"] = SETTINGS_FORMAT_VERSION;

    // Save

    println("Saving settings...");

    ofstream file(SETTINGS_FILE);

    if (!file)
    {
        println("Cannot open file \"{}\"", SETTINGS_FILE);
        return;
    }

    file << j.dump(2);
}

Settings Settings::load()
{
    println("Loading settings...");

    ifstream file(SETTINGS_FILE);

    if (!file)
    {
        println("Cannot open file \"{}\"", SETTINGS_FILE);
        return {};
    }

    json j = json::parse(file, nullptr, false);

    if (j.is_discarded())
    {
        println("Cannot parse file \"{}\"", SETTINGS_FILE);
        return {};
    }

    // Check the version number

    int version = j.value("version", 0);

    if (version != SETTINGS_FORMAT_VERSION)
    {
        println("Settings are out of date (loaded version is {}, current version is {})", version, SETTINGS_FORMAT_VERSION);
        return {};
    }

    Settings s = j.get<Settings>();

    // Load the breakpoints

    for (const json& json_set : j.value("breakpoint_sets", json::array()))
    {
        Breakpoints& breakpoints = s.breakpoints[json_set.value("game", "")];
        breakpoints = {};

        const json json_breakpoints = json_set.value("breakpoints", json::array());

        for (size_t i = 0; i < json_breakpoints.size() && i < BREAKPOINTS_COUNT; ++i)
        {
            breakpoints[i].enabled = json_breakpoints[i].value("enabled", false);
            breakpoints[i].address = json_breakpoints[i].value("address", 0u);
        }
    }

    return s;
}

Breakpoint* Settings::breakpoints_for(const string& game)
{
    return breakpoints.try_emplace(game).first->second.data();
}
