#include "../NanoNativeWallet.h"
#include "../Models/Wallet.h"
#include "../UI.h"
#include "../Crypto/crypto_utils.h"

#include "Database.h"

#include <fstream>
#include <filesystem>

#define DB_FILE_NAME "db.json"

#define FILE_EXTENSION ".key"

#define MAGIC "POGU"
#define MAGIC_SIZE 4

#define FILE_VERSION 1
#define FILE_VERSION_SIZE 1

#include <nlohmann/json.hpp>
using nlohmann::json;

json getLocalDatabase() {
    if (!std::filesystem::exists(DB_FILE_NAME)) {
        return { { "wallets", json::array() } };
    }

    std::ifstream i(DB_FILE_NAME);
    json j;
    i >> j;

    return j;
}

void saveDatabase() {
    std::ofstream o(DB_FILE_NAME);
    json j = {
        { "wallets", json::array() }
    };

    for (std::vector<Wallet>::size_type i = 0; i != gWallets.size(); i++) {
        auto name = gWallets[i].name;
        auto uuid = gWallets[i].uuid;

        j["wallets"].push_back({
            { "name", name },
            { "uuid", uuid }
        });

        saveWalletToDisk(uuid, gWallets[i].encryptedSeed, gWallets[i].iv);
    }

    o << std::setw(4) << j << std::endl;
}

void saveWalletToDisk(std::string uuid, std::vector<byte> encryptedData, std::vector<byte> iv) {
    std::ofstream o(uuid + FILE_EXTENSION, std::ios::binary);
    char file_version[FILE_VERSION_SIZE] = { FILE_VERSION };

    o.write((const char*) MAGIC, MAGIC_SIZE);
    o.write(reinterpret_cast<const char *>(&file_version), FILE_VERSION_SIZE);

    o.write((const char*) &iv[0], IV_SIZE);
    o.write((const char*) &encryptedData[0], SEED_SIZE);

    o.close();
}

void deleteWalletFromDisk (std::string uuid) {
    std::filesystem::remove(std::string(uuid) + FILE_EXTENSION);
}

IVKeyPair loadWalletFromDisk(std::string uuid) {
    // File Format: 
    // [random_16b_iv][encrypted_seed_64b]
    std::ifstream is(uuid + FILE_EXTENSION, std::ios::binary);
    
    if (is.fail()) {
        throw "Failed to open file";
    }

    is.seekg(0, is.end);
    long lSize = is.tellg();
    is.seekg(0, is.beg);

    if (lSize != (MAGIC_SIZE + FILE_VERSION_SIZE + IV_SIZE + SEED_SIZE)) {
        throw "Invalid file provided";
    }

    char* magic = new char[MAGIC_SIZE];
    is.read(magic, MAGIC_SIZE);

    if (strncmp(magic, MAGIC, MAGIC_SIZE) != 0) {
        throw "Invalid file provided";
    }

    char* fileVersion = new char[FILE_VERSION_SIZE];
    is.read(fileVersion, FILE_VERSION_SIZE);

    char* IV = new char[IV_SIZE];
    is.read(IV, IV_SIZE);

    char* seed = new char[SEED_SIZE];
    is.read(seed, SEED_SIZE);

    is.close();

    IVKeyPair pair = { std::vector<byte>(IV, IV + IV_SIZE), std::vector<byte>(seed, seed + SEED_SIZE) };

    delete[] IV;
    delete[] seed;

    return pair;
}