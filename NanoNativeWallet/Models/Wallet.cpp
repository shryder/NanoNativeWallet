#include "Wallet.h"
#include "Account.h"

#include <vector>

#include "../Crypto/crypto_utils.h"

Wallet::Wallet(std::string walletName, std::vector<byte> walletEncryptedSeed, std::vector<byte> IV) {
    name = walletName;
    encryptedSeed = walletEncryptedSeed;
    iv = IV;
    isEncrypted = true;
}

Wallet::Wallet(std::string walletName, std::vector<byte> walletEncryptedSeed, std::vector<byte> IV, std::string walletSeed) {
    name = walletName;
    encryptedSeed = walletEncryptedSeed;
    iv = IV;

    unlock(walletSeed);
}

void Wallet::addAccount(size_t i) {
    bool found = false;
    for (Account account : accounts) {
        if (account.index == i) {
            account.show();
            found = true;
        }
    }

    if (!found) {
        auto accountSecret = deriveSecretKey(seed, i);
        auto derivedPublicAddress = derivePublicAddressFromSecret(accountSecret);

        accounts.push_back(Account(i, derivedPublicAddress, 0));
    }
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
    for (size_t i = 0; i < 10; i++) {
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