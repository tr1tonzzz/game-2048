#pragma once

#include <cstdint>
#include <gui/common/Game2048Engine.hpp>

namespace appstorage {

constexpr uint32_t SaveMagic = 0x20482048UL;
constexpr uint32_t SaveVersion = 1UL;

#pragma pack(push, 1)
struct SaveData {
    uint32_t magic = SaveMagic;
    uint32_t version = SaveVersion;
    game2048::Snapshot snapshot;
    uint32_t highScore[game2048::MaxBoard + 1] = {};
    uint8_t theme = 0;
    uint8_t hasSnapshot = 0;
    uint16_t reserved = 0;
    uint32_t checksum = 0;
};
#pragma pack(pop)

class FlashStorage {
public:
    static bool load(SaveData& data);
    static bool save(const SaveData& data);
    static void makeValid(SaveData& data);

private:
    static uint32_t checksum(const SaveData& data);
};

} // namespace appstorage
