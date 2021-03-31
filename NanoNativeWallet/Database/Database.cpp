#include "../NanoNativeWallet.h"
#include "../Models/Wallet.h"
#include "../UI.h"
#include "../Crypto/crypto_utils.h"

#include "Database.h"

#include <fstream>
#include <filesystem>

#define DB_FILE_NAME "db.json"

#define MAGIC "POGU"
#define MAGIC_SIZE 4

#include <nlohmann/json.hpp>
using nlohmann::json;

json getLocalDatabase() {
    if (!std::filesystem::exists(DB_FILE_NAME)) {
        return { { "wallets", {} } };
    }

    std::ifstream i(DB_FILE_NAME);
    json j;
    i >> j;

    return j;
}

void saveSeed(size_t id, std::vector<byte> encryptedData, std::vector<byte> iv) {
    std::ofstream o(std::to_string(id) + ".key", std::ios::binary);

    o.write((const char*) MAGIC, 4);
    o.write((const char*) &iv[0], IV_SIZE);
    o.write((const char*) &encryptedData[0], SEED_SIZE);

    o.close();
}

void saveDatabase() {
    std::ofstream o(DB_FILE_NAME);
    json j = {
        { "wallets", {} }
    };

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
    
    if (is.fail()) {
        throw "Failed to open file";
    }

    is.seekg(0, is.end);
    long lSize = is.tellg();
    is.seekg(0, is.beg);

    if (lSize != (MAGIC_SIZE + IV_SIZE + SEED_SIZE)) {
        throw "Invalid file provided";
    }

    char* magic = new char[MAGIC_SIZE];
    is.read(magic, MAGIC_SIZE);

    if (strncmp(magic, MAGIC, MAGIC_SIZE) != 0) {
        throw "Invalid file provided";
    }

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