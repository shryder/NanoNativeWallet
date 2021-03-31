#include "Wallet.h"
#include "Account.h"

#include <vector>

#include "../Crypto/crypto_utils.h"

Wallet::Wallet(std::string walletUuid, std::string walletName, std::vector<byte> walletEncryptedSeed, std::vector<byte> IV) {
    uuid = walletUuid;
    name = walletName;
    encryptedSeed = walletEncryptedSeed;
    iv = IV;

    isEncrypted = true;
}

Wallet::Wallet(std::string walletUuid, std::string walletName, std::vector<byte> walletEncryptedSeed, std::vector<byte> IV, std::string walletSeed) {
    uuid = walletUuid;
    name = walletName;
    encryptedSeed = walletEncryptedSeed;
    seed = walletSeed;
    iv = IV;

    isEncrypted = false;
    loadAccounts();
}

void Wallet::addAccount(size_t i) {
    // Check if account already exists
    for (Account account : accounts) {
        if (account.index == i) return;
    }
       
    auto accountSecret = deriveSecretKey(seed, i);
    auto derivedPublicAddress = derivePublicAddressFromSecret(accountSecret);

    accounts.push_back(Account(i, derivedPublicAddress));
}

void Wallet::addAccount() {
    addAccount(accounts.size());
}

void Wallet::unlock(std::string password) {
    isEncrypted = false;
    seed = decryptAES(encryptedSeed, iv, password);

    loadAccounts();
}

void Wallet::lock() {
    isEncrypted = true;

    accounts.clear();
    seed.clear();
}

void Wallet::loadAccounts() {
    for (size_t i = 0; i < 5; i++) {
        addAccount(i);
    }
}

void Wallet::setName(char* newName) {
    name = std::string(newName);
}

void Wallet::updatePassword(char* newPassword) {
    iv = generateIV();
    auto vEncryptedSeed = encryptAES(seed, std::string(newPassword), iv);

    encryptedSeed = std::vector<byte>(vEncryptedSeed.begin(), vEncryptedSeed.end());

    lock();
}