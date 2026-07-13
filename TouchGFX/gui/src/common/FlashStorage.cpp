#include <gui/common/FlashStorage.hpp>
#include <cstring>
#include "stm32f4xx_hal.h"

namespace appstorage {

namespace {
constexpr uint32_t StorageAddress = 0x081E0000UL; // STM32F429ZI sector 23, 128 KiB.
constexpr uint32_t StorageSector = FLASH_SECTOR_23;
}

bool FlashStorage::load(SaveData& data) {
    const SaveData* stored = reinterpret_cast<const SaveData*>(StorageAddress);
    std::memcpy(&data, stored, sizeof(SaveData));

    if (data.magic != SaveMagic || data.version != SaveVersion) {
        return false;
    }
    if (data.snapshot.size < game2048::MinBoard || data.snapshot.size > game2048::MaxBoard) {
        return false;
    }
    return data.checksum == checksum(data);
}

bool FlashStorage::save(const SaveData& input) {
    SaveData data = input;
    makeValid(data);

    // Disable interrupts to prevent HardFault/Crash when erasing/programming Flash
    __disable_irq();

    HAL_FLASH_Unlock();

    /* Clear pending flags (if any) before erasing */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    FLASH_EraseInitTypeDef erase = {};
    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.Banks = FLASH_BANK_2; // Sector 23 is in Bank 2
    erase.Sector = StorageSector;
    erase.NbSectors = 1;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    uint32_t sectorError = 0;
    if (HAL_FLASHEx_Erase(&erase, &sectorError) != HAL_OK) {
        HAL_FLASH_Lock();
        __enable_irq();
        return false;
    }

    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&data);
    const uint32_t length = sizeof(SaveData);
    for (uint32_t offset = 0; offset < length; offset++) {
        uint8_t value = bytes[offset];
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, StorageAddress + offset, value) != HAL_OK) {
            HAL_FLASH_Lock();
            __enable_irq();
            return false;
        }
    }

    HAL_FLASH_Lock();
    __enable_irq();

    /* Flush Flash Instruction and Data caches to ensure CPU reads the new data */
    __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();
    __HAL_FLASH_INSTRUCTION_CACHE_RESET();
    __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();

    __HAL_FLASH_DATA_CACHE_DISABLE();
    __HAL_FLASH_DATA_CACHE_RESET();
    __HAL_FLASH_DATA_CACHE_ENABLE();

    return true;
}

void FlashStorage::makeValid(SaveData& data) {
    data.magic = SaveMagic;
    data.version = SaveVersion;
    if (data.snapshot.size < game2048::MinBoard || data.snapshot.size > game2048::MaxBoard) {
        data.snapshot.size = game2048::DefaultBoard;
    }
    data.checksum = 0;
    data.checksum = checksum(data);
}

uint32_t FlashStorage::checksum(const SaveData& data) {
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&data);
    uint32_t sum = 2166136261UL;
    for (uint32_t i = 0; i < sizeof(SaveData) - sizeof(uint32_t); i++) {
        sum ^= bytes[i];
        sum *= 16777619UL;
    }
    return sum;
}

} // namespace appstorage
