#include "../NanoNativeWallet.h"
#include "../Models/Wallet.h"
#include "../UI.h"
#include "../Crypto/crypto_utils.h"

#include "Database.h"

#include <fstream>
#include <filesystem>

#define DB_FILE_NAME "db.json"

#include <nlohmann/json.hpp>
using nlohmann::json;

json getLocalDatabase() {
    if (!std::filesystem::exists(DB_FILE_NAME)) {
        return json::parse("{ \"wallets\": [] }");
    }

    std::ifstream i(DB_FILE_NAME);
    json j;
    i >> j;

    return j;
}

void saveSeed(size_t id, std::vector<byte> encryptedData, std::vector<byte> iv) {
    std::ofstream o(std::to_string(id) + ".key", std::ios::binary);

    std::vector<byte> fileContent = {};
    for (size_t i = 0; i < iv.size(); i++) {
        fileContent.push_back(iv[i]);
    }

    for (size_t i = 0; i < encryptedData.size(); i++) {
        fileContent.push_back(encryptedData.at(i));
    }

    o.write(reinterpret_cast<char const*>(fileContent.data()), fileContent.size());
    o.close();
}

void saveDatabase() {
    std::ofstream o(DB_FILE_NAME);
    json j;

    // Build the json and save
    j["wallets"] = {};

    for (std::vector<Wallet>::size_type i = 0; i != gWallets.size(); i++) {
        j["wallets"].push_back(gWallets[i].name);
        saveSeed(i, gWallets[i].encryptedSeed, gWallets[i].iv);
    }

    o << std::setw(4) << j << std::endl;
}

IVKeyPair loadWalletFromDisk(size_t id) {
    // File Format: 
    // [random_16b_iv][encrypted_seed_64b]

    std::ifstream is(std::to_string(id) + ".key", std::ios::binary);
    char* file_content = new char[IV_SIZE + SEED_SIZE];
    is.read(file_content, IV_SIZE + SEED_SIZE);

    // read first 16 bytes
    std::vector<byte> fileIV = {};
    for (size_t i = 0; i < IV_SIZE; i++) {
        fileIV.push_back(file_content[i]);
    }

    // read the rest 64 bytes
    std::vector<byte> fileEncryptedSeed = {};
    for (size_t i = IV_SIZE; i < (IV_SIZE + SEED_SIZE); i++) {
        fileEncryptedSeed.push_back(file_content[i]);
    }

    return IVKeyPair { fileIV, fileEncryptedSeed };
}