#pragma once

#include "../Crypto/crypto_utils.h"
#include <nlohmann/json.hpp>

nlohmann::json getLocalDatabase();

void saveSeed(size_t id, std::vector<std::byte> encryptedData, std::vector<std::byte> iv);
void saveDatabase();

IVKeyPair loadWalletFromDisk(size_t id);