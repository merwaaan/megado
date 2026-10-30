#include <cstdlib>
#include <cstring>
#include <format>
#include <fstream>
#include <memory>
#include <print>

#include "core.h"
#include "snapshot.h"

using namespace std;

static string file_name(const string& game, uint8_t slot)
{
    return format("{}.snapshot{}", game, slot);
}

optional<SnapshotMetadata> snapshots::save(Genesis* g, uint8_t slot)
{
    string game = rom_name(g);

    SnapshotMetadata metadata{};
    strncpy(metadata.game, game.c_str(), sizeof(metadata.game));
    metadata.date = time(nullptr);
    metadata.version = SNAPSHOT_VERSION;

    unique_ptr<Snapshot, decltype(&free)> snapshot(snapshot_take(g), &free);

    string name = file_name(game, slot);

    println("Saving snapshot {}...", name);

    ofstream file(name, ios::binary);

    if (!file)
    {
        print("Cannot open file \"{}\"", name);
        return nullopt;
    }

    // Metadata then data

    file.write(reinterpret_cast<const char*>(&metadata), sizeof(SnapshotMetadata));
    file.write(reinterpret_cast<const char*>(snapshot.get()), sizeof(Snapshot));

    return metadata;
}

void snapshots::load(Genesis* g, uint8_t slot)
{
    string name = file_name(rom_name(g), slot);

    println("Loading snapshot {}...", name);

    ifstream file(name, ios::binary);

    if (!file)
    {
        print("Cannot open file \"{}\"", name);
        return;
    }

    auto snapshot = make_unique<Snapshot>();
    file.seekg(sizeof(SnapshotMetadata)); // Skip the metadata header
    file.read(reinterpret_cast<char*>(snapshot.get()), sizeof(Snapshot));

    snapshot_restore(g, snapshot.get());
}

SnapshotSlots snapshots::preload(Genesis* g)
{
    string game = rom_name(g);

    SnapshotSlots slots;

    for (uint8_t slot = 0; slot < SNAPSHOT_SLOTS; ++slot)
    {
        string name = file_name(game, slot);

        ifstream file(name, ios::binary);

        if (!file)
        {
            continue;
        }

        SnapshotMetadata metadata;
        file.read(reinterpret_cast<char*>(&metadata), sizeof(SnapshotMetadata));

        println("Snapshot found: {}", name);

        if (metadata.version != SNAPSHOT_VERSION)
        {
            print("Incompatible snapshot version (loaded version is {}, current version is {})", +metadata.version, SNAPSHOT_VERSION);

            continue;
        }

        slots[slot] = metadata;
    }

    return slots;
}
