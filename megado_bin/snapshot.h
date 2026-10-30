#pragma once

#include <array>
#include <cstdint>
#include <ctime>
#include <optional>

struct Genesis;

constexpr uint8_t SNAPSHOT_SLOTS = 3;

struct SnapshotMetadata
{
    char game[48]; // Game name, extracted from the header
    time_t date;
    uint8_t version;
};

using SnapshotSlots = std::array<std::optional<SnapshotMetadata>, SNAPSHOT_SLOTS>;

namespace snapshots
{
// Saves/Loads a snapshot in the given slot for the game currently being executed
std::optional<SnapshotMetadata> save(Genesis*, uint8_t slot);
void load(Genesis*, uint8_t slot);

// Gets the metadata from saved snapshots of the game currently being executed
SnapshotSlots preload(Genesis*);
}
