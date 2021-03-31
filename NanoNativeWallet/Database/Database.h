#pragma once

#include "../Crypto/crypto_utils.h"
#include <nlohmann/json.hpp>

nlohmann::json getLocalDatabase();
std::string generateUUID();

void saveWalletToDisk(std::string uuid, std::vector<byte> encryptedData, std::vector<byte> iv);
void saveDatabase();
void deleteWalletFromDisk (std::string uuid);

IVKeyPair loadWalletFromDisk(std::string uuid);
